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
        controller.batteryDriver.isCharging = false;
        controller.batteryDriver.isLowBattery = false;
        controller.cleanerDriver.isRunning = false;
        controller.cleanerDriver.isBoosting = false;
        controller.motorDriver.isRunning = false;
        controller.motorDriver.direction = Direction::Forward;
        controller.obstacleSensorDriver.clear();
        controller.dustSensorDriver.clear();
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

void applyModeOutputs(Controller& controller)
{
    if (dynamic_cast<NormalMode*>(controller.currentMode) != nullptr)
    {
        controller.motorDriver.moveForward();
        controller.cleanerDriver.start();
        return;
    }

    if (dynamic_cast<BoostMode*>(controller.currentMode) != nullptr)
    {
        controller.motorDriver.moveForward();
        controller.cleanerDriver.boost();
        return;
    }

    controller.motorDriver.stop();
    controller.cleanerDriver.stop();
}

void commitModeTransition(Controller& controller, OperatingMode* previousMode, OperatingMode* nextMode)
{
    if (nextMode != previousMode)
    {
        deleteMode(previousMode);
        controller.currentMode = nextMode;
        applyModeOutputs(controller);
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

void moveMotor(MotorDriver& motorDriver, Direction direction)
{
    switch (direction)
    {
    case Direction::Forward:
        motorDriver.moveForward();
        break;
    case Direction::Left:
        motorDriver.turnLeft();
        break;
    case Direction::Right:
        motorDriver.turnRight();
        break;
    case Direction::Backward:
        motorDriver.moveBackward();
        break;
    }
}
} // namespace

void Controller::powerButtonPressed()
{
    ControllerState& state = stateFor(*this);

    if (!state.power)
    {
        batteryDriver.isCharging = false;
        obstacleSensorDriver.clear();
        dustSensorDriver.clear();
        currentMode = newMode<StandbyMode>();
        applyModeOutputs(*this);
        state.power = true;
        return;
    }

    if (currentMode != nullptr)
    {
        currentMode->powerButtonPressed(*this);
    }

    deleteMode(currentMode);
    currentMode = nullptr;
    applyModeOutputs(*this);
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

    if (dynamic_cast<NormalMode*>(currentMode) != nullptr ||
        dynamic_cast<BoostMode*>(currentMode) != nullptr)
    {
        return;
    }

    batteryDriver.isCharging = true;
}

void Controller::stopCharging()
{
    stateFor(*this);
    batteryDriver.isCharging = false;
}

void Controller::lowBatteryDetected()
{
    stateFor(*this);

    deleteMode(currentMode);
    currentMode = newMode<LowBatteryMode>();
    batteryDriver.isLowBattery = true;
    applyModeOutputs(*this);
}

void Controller::dustDetected()
{
    stateFor(*this);

    if (currentMode == nullptr)
    {
        return;
    }

    OperatingMode* previousMode = currentMode;
    OperatingMode* nextMode = currentMode->dustDetected(*this);
    commitModeTransition(*this, previousMode, nextMode);
}

void Controller::obstacleDetected()
{
    stateFor(*this);

    if (currentMode == nullptr)
    {
        return;
    }

    const bool shouldResumeCleaning = cleanerDriver.isRunning && isActiveCleaningMode(currentMode);
    const bool shouldResumeBoosting = cleanerDriver.isBoosting && isBoostMode(currentMode);
    const Direction direction = obstacleProcessor.decideDirection(obstacleSensorDriver);

    cleanerDriver.stop();
    moveMotor(motorDriver, direction);

    if (shouldResumeCleaning)
    {
        if (shouldResumeBoosting)
        {
            cleanerDriver.boost();
        }
        else
        {
            cleanerDriver.start();
        }
    }
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

void BatteryDriver::charge()
{
    isCharging = true;
}

void BatteryDriver::stopCharging()
{
    isCharging = false;
}

void BatteryDriver::setLowBattery(bool lowBattery)
{
    isLowBattery = lowBattery;
}

void CleanerDriver::start()
{
    isRunning = true;
    isBoosting = false;
}

void CleanerDriver::stop()
{
    isRunning = false;
    isBoosting = false;
}

void CleanerDriver::boost()
{
    isRunning = true;
    isBoosting = true;
}

void CleanerDriver::normal()
{
    start();
}

void MotorDriver::start(Direction nextDirection)
{
    isRunning = true;
    direction = nextDirection;
}

void MotorDriver::stop()
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

void DustSensorDriver::clear()
{
    dustDetected = false;
}

} // namespace rvc
