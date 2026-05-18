#include "../include/rvc/Controller.hpp"

#include <unordered_map>

namespace rvc
{
namespace
{
struct ControllerState
{
    bool power = false;
};

std::unordered_map<Controller*, ControllerState> controllerStates;
ControllerState& stateFor(Controller& controller)
{
    const auto inserted = controllerStates.emplace(&controller, ControllerState{});
    if (inserted.second)
    {
        controller.currentMode = nullptr;
        controller.batteryDriver.initialize();
        controller.cleanerDriver.initialize();
        controller.motorDriver.initialize();
        controller.obstacleSensorDriver.initialize();
        controller.dustSensorDriver.initialize();
    }

    return inserted.first->second;
}

template <typename Mode>
OperatingMode* newMode()
{
    return new Mode();
}

void deleteMode(OperatingMode* mode)
{
    delete mode;
}

void commitModeTransition(Controller& controller, OperatingMode* previousMode, OperatingMode* nextMode)
{
    if (nextMode != previousMode)
    {
        deleteMode(previousMode);
        controller.currentMode = nextMode;
    }
}

bool isActiveCleaningMode(const OperatingMode* mode)
{
    return dynamic_cast<const NormalMode*>(mode) != nullptr ||
           dynamic_cast<const BoostMode*>(mode) != nullptr;
}

bool isBoostMode(const OperatingMode* mode)
{
    return dynamic_cast<const BoostMode*>(mode) != nullptr;
}

bool canStartCharging(const Controller& controller)
{
    return controller.currentMode == nullptr || controller.currentMode->canCharge();
}
} // namespace

void Controller::powerButtonPressed()
{
    ControllerState& state = stateFor(*this);

    if (!state.power)
    {
        batteryDriver.initialize();
        dustSensorDriver.initialize();
        cleanerDriver.initialize();
        motorDriver.initialize();
        obstacleSensorDriver.initialize();
        currentMode = newMode<StandbyMode>();
        state.power = true;
        return;
    }

    if (currentMode != nullptr)
    {
        currentMode->powerButtonPressed(*this);
    }

    deleteMode(currentMode);
    currentMode = nullptr;
    batteryDriver.turnOffBattery();
    obstacleSensorDriver.deactivateObstacleSensor();
    dustSensorDriver.deactivateDustSensor();
    motorDriver.stopMoving();
    cleanerDriver.stopCleaning();
    state.power = false;
}

void Controller::startButtonPressed()
{
    stateFor(*this);

    if (currentMode == nullptr)
    {
        return;
    }

    OperatingMode* previousMode = currentMode;
    OperatingMode* nextMode = currentMode->startButtonPressed(*this);
    commitModeTransition(*this, previousMode, nextMode);
}

void Controller::chargeBattery()
{
    stateFor(*this);

    if (!canStartCharging(*this))
    {
        return;
    }

    batteryDriver.startCharging();
    chargingTick();
}

void Controller::stopCharging()
{
    stateFor(*this);
    batteryDriver.stopCharging();

    if (currentMode == nullptr)
    {
        currentMode = newMode<StandbyMode>();
    }
}

void Controller::lowBatteryDetected()
{
    stateFor(*this);

    if (currentMode == nullptr)
    {
        return;
    }

    OperatingMode* previousMode = currentMode;
    OperatingMode* nextMode = currentMode->lowBatteryDetected(*this);
    commitModeTransition(*this, previousMode, nextMode);
    batteryDriver.isLowBattery = true;
    batteryDriver.level = BatteryDriver::LowBatteryThreshold;
}

void Controller::lowBatteryCleared()
{
    stateFor(*this);

    if (currentMode == nullptr)
    {
        return;
    }

    OperatingMode* previousMode = currentMode;
    OperatingMode* nextMode = currentMode->lowBatteryCleared(*this);
    commitModeTransition(*this, previousMode, nextMode);
    batteryDriver.isLowBattery = false;
}

void Controller::dustDetected()
{
    stateFor(*this);

    if (currentMode == nullptr || batteryDriver.isCharging)
    {
        return;
    }

    if (!dustSensorDriver.dustDetected || !dustProcessor.shouldBoost(cleanerDriver))
    {
        return;
    }

    OperatingMode* previousMode = currentMode;
    OperatingMode* nextMode = currentMode->dustDetected(*this);
    commitModeTransition(*this, previousMode, nextMode);
}

void Controller::obstacleDetected(const bool direction[3])
{
    stateFor(*this);

    if (currentMode == nullptr || direction == nullptr)
    {
        return;
    }

    const bool shouldResumeCleaning = cleanerDriver.isRunning && isActiveCleaningMode(currentMode);
    const bool shouldResumeBoosting = cleanerDriver.isBoosting && isBoostMode(currentMode);

    obstacleSensorDriver.front = direction[0];
    obstacleSensorDriver.left = direction[1];
    obstacleSensorDriver.right = direction[2];

    const Direction selectedDirection = obstacleProcessor.decideDirection(obstacleSensorDriver);
    cleanerDriver.stopCleaning();
    currentMode->checkIsMoving(selectedDirection, motorDriver);

    if (shouldResumeCleaning)
    {
        if (shouldResumeBoosting)
        {
            cleanerDriver.decideSetting(true);
        }
        else
        {
            cleanerDriver.startCleaning();
        }
    }
}

void Controller::obstacleDetected()
{
    const bool direction[3] = {
        obstacleSensorDriver.front,
        obstacleSensorDriver.left,
        obstacleSensorDriver.right,
    };

    obstacleDetected(direction);
}

void Controller::timerExpired()
{
    stateFor(*this);

    if (currentMode == nullptr)
    {
        return;
    }

    OperatingMode* previousMode = currentMode;
    OperatingMode* nextMode = currentMode->timerExpired(*this);
    commitModeTransition(*this, previousMode, nextMode);
}

void Controller::timerExpiredNow()
{
    timerExpired();
}

void Controller::chargingTick()
{
    stateFor(*this);

    if (!batteryDriver.isCharging)
    {
        return;
    }

    const bool increased = batteryDriver.inclineLV();
    if (!increased || batteryDriver.isFull())
    {
        batteryDriver.stopCharging();
    }

    if (dynamic_cast<LowBatteryMode*>(currentMode) != nullptr &&
        batteryDriver.level > BatteryDriver::LowBatteryThreshold)
    {
        lowBatteryCleared();
    }
}

void Controller::clockTick()
{
    stateFor(*this);

    chargingTick();

    if (currentMode == nullptr)
    {
        return;
    }

    if (batteryDriver.isLowBattery &&
        dynamic_cast<LowBatteryMode*>(currentMode) == nullptr)
    {
        lowBatteryDetected();
        return;
    }

    if (dustSensorDriver.dustDetected)
    {
        dustDetected();
    }

    if (obstacleSensorDriver.hasObstacle())
    {
        obstacleDetected();
    }
}

void BatteryDriver::initialize()
{
    isCharging = false;
    isLowBattery = false;
    level = FullBatteryLevel;
}

void BatteryDriver::charge()
{
    startCharging();
}

void BatteryDriver::startCharging()
{
    isCharging = !isFull();
}

void BatteryDriver::stopCharging()
{
    isCharging = false;
}

void BatteryDriver::turnOffBattery()
{
    stopCharging();
}

void BatteryDriver::setLowBattery(bool lowBattery)
{
    isLowBattery = lowBattery;
    if (lowBattery)
    {
        level = LowBatteryThreshold;
    }
}

bool BatteryDriver::inclineLV()
{
    return inclineLevel();
}

bool BatteryDriver::inclineLevel()
{
    if (isFull())
    {
        return false;
    }

    level += 10;
    if (level > FullBatteryLevel)
    {
        level = FullBatteryLevel;
    }

    return true;
}

bool BatteryDriver::isFull() const
{
    return level >= FullBatteryLevel;
}

void CleanerDriver::initialize()
{
    stopCleaning();
}

void CleanerDriver::start()
{
    startCleaning();
}

void CleanerDriver::startCleaning()
{
    isRunning = true;
    isBoosting = false;
}

void CleanerDriver::stop()
{
    stopCleaning();
}

void CleanerDriver::stopCleaning()
{
    isRunning = false;
    isBoosting = false;
}

void CleanerDriver::boost()
{
    decideSetting(true);
}

void CleanerDriver::normal()
{
    decideSetting(false);
}

void CleanerDriver::decideSetting(bool boostEnabled)
{
    isRunning = true;
    isBoosting = boostEnabled;
}

void MotorDriver::initialize()
{
    isRunning = false;
    direction = Direction::Forward;
}

void MotorDriver::start(Direction nextDirection)
{
    isRunning = true;
    direction = nextDirection;
}

void MotorDriver::stop()
{
    stopMoving();
}

void MotorDriver::stopMoving()
{
    isRunning = false;
}

void MotorDriver::moveForward()
{
    start(Direction::Forward);
}

void MotorDriver::turnLeft()
{
    start(Direction::Left);
}

void MotorDriver::turnRight()
{
    start(Direction::Right);
}

void MotorDriver::moveBackward()
{
    start(Direction::Backward);
}

void ObstacleSensorDriver::initialize()
{
    clear();
}

void ObstacleSensorDriver::deactivateObstacleSensor()
{
    clear();
}

void ObstacleSensorDriver::clear()
{
    front = false;
    left = false;
    right = false;
}

bool ObstacleSensorDriver::hasObstacle() const
{
    return front || left || right;
}

void DustSensorDriver::initialize()
{
    clear();
}

void DustSensorDriver::deactivateDustSensor()
{
    clear();
}

void DustSensorDriver::clear()
{
    dustDetected = false;
}

} // namespace rvc
