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

// [추가] processor의 후진 후 재확인 결과를 motor command로 변환
void moveAfterBackwardRecheck(Direction direction, MotorDriver& motorDriver)
{
    if (direction == Direction::Left)
    {
        motorDriver.turnLeft();
        motorDriver.moveForward();
        return;
    }

    if (direction == Direction::Right)
    {
        motorDriver.turnRight();
        motorDriver.moveForward();
        return;
    }

    motorDriver.moveBackward();
}

// [추가] front/left 입력과 저장된 recheck state를 사용해 obstacle avoidance를 수행한다.
void handleObstacleAvoidance(Controller& controller)
{
    const bool shouldResumeCleaning =
        controller.cleanerDriver.isRunning && isActiveCleaningMode(controller.currentMode);
    const bool shouldResumeBoosting =
        controller.cleanerDriver.isBoosting && isBoostMode(controller.currentMode);

    const Direction selectedDirection =
        controller.obstacleProcessor.decideDirection(controller.obstacleSensorDriver);
    controller.cleanerDriver.stopCleaning();
    // [변경] right-recheck avoidance flow는 실제 청소 중인 mode에서만 수행하고, safe mode는 mode 정책에 맡긴다.
    if (!isActiveCleaningMode(controller.currentMode))
    {
        controller.currentMode->checkIsMoving(selectedDirection, controller.motorDriver);
        return;
    }

    if (selectedDirection == Direction::Right)
    {
        controller.motorDriver.turnRight();
        const Direction directionAfterRightTurn =
            controller.obstacleProcessor.decideDirectionAfterRightTurn(
                controller.obstacleSensorDriver);
        if (directionAfterRightTurn == Direction::Forward)
        {
            controller.motorDriver.moveForward();
        }
        else
        {
            // [추가] right가 막히면 left turn으로 원래 front를 복구한 뒤 backward recheck flow를 수행한다.
            controller.motorDriver.turnLeft();
            controller.motorDriver.moveBackward();
            moveAfterBackwardRecheck(
                controller.obstacleProcessor.decideDirectionAfterBackwardRecheck(
                    controller.obstacleSensorDriver),
                controller.motorDriver);
        }
    }
    else
    {
        controller.currentMode->checkIsMoving(selectedDirection, controller.motorDriver);
    }

    if (shouldResumeCleaning)
    {
        if (shouldResumeBoosting)
        {
            controller.cleanerDriver.decideSetting(true);
        }
        else
        {
            controller.cleanerDriver.startCleaning();
        }
    }
}
} // namespace

// [추가] Controller 객체 주소가 재사용될 때 이전 stateFor() map entry가 새 인스턴스에 적용되지 않게 한다.
Controller::Controller()
    : currentMode(nullptr)
{
    controllerStates.erase(this);
    batteryDriver.initialize();
    cleanerDriver.initialize();
    motorDriver.initialize();
    obstacleSensorDriver.initialize();
    dustSensorDriver.initialize();
}

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

// [변경] 외부 obstacle 입력을 front/left 두 칸으로 축소
void Controller::obstacleDetected(const bool direction[2])
{
    stateFor(*this);

    if (currentMode == nullptr || direction == nullptr)
    {
        return;
    }

    obstacleSensorDriver.setObstacleInput(direction[0], direction[1]);
    handleObstacleAvoidance(*this);
}

// [변경] 저장된 obstacle sensor state에서 right sensor 입력 제외
void Controller::obstacleDetected()
{
    stateFor(*this);

    if (currentMode == nullptr)
    {
        return;
    }

    handleObstacleAvoidance(*this);
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

// [변경] right sensor state 제거, front/left와 recheck state를 초기화
void ObstacleSensorDriver::clear()
{
    front = false;
    left = false;
    frontAfterRightTurn = true;
    leftAfterBackward = true;
    frontAfterBackwardRightCheck = true;
}

// [변경] direction[2] 입력은 front/left만 갱신하고 right sensor 입력은 받지 않는다.
void ObstacleSensorDriver::setObstacleInput(bool frontBlocked, bool leftBlocked)
{
    front = frontBlocked;
    left = leftBlocked;
    frontAfterRightTurn = true;
    leftAfterBackward = true;
    frontAfterBackwardRightCheck = true;
}

// [추가] right 방향은 우회전 후 front sensor 재확인값으로 저장한다.
void ObstacleSensorDriver::setFrontAfterRightTurn(bool frontBlocked)
{
    frontAfterRightTurn = frontBlocked;
}

// [추가] 후진 중 left sensor와 front sensor 기반 right 확인값을 저장한다.
void ObstacleSensorDriver::setBackwardRecheck(bool leftBlocked, bool frontBlockedForRightCheck)
{
    leftAfterBackward = leftBlocked;
    frontAfterBackwardRightCheck = frontBlockedForRightCheck;
}

// [추가] 우회전 후 front가 비어 있으면 기존 right 방향으로 전진 가능하다.
bool ObstacleSensorDriver::isFrontClearAfterRightTurn() const
{
    return !frontAfterRightTurn;
}

// [추가] 후진 후 비어있는 방향 선택에서 left가 우선순위이다.
bool ObstacleSensorDriver::isLeftClearAfterBackward() const
{
    return !leftAfterBackward;
}

// [추가] right는 별도 sensor가 아니라 front 기반 재확인값으로 판단한다.
bool ObstacleSensorDriver::isRightClearAfterBackward() const
{
    return !frontAfterBackwardRightCheck;
}

// [변경] right sensor 입력 제거로 front/left만 obstacle 여부에 사용
bool ObstacleSensorDriver::hasObstacle() const
{
    return front || left;
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
