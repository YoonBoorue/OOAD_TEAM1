#include "../include/rvc/Controller.hpp"

namespace rvc
{
namespace
{
void moveByDirection(Direction direction, MotorDriver& motorDriver)
{
    switch (direction)
    {
    case Direction::Forward:
        motorDriver.moveForward();
        break;
    case Direction::Left:
        motorDriver.turnLeft();
        motorDriver.moveForward();
        break;
    case Direction::Right:
        motorDriver.turnRight();
        motorDriver.moveForward();
        break;
    case Direction::Backward:
        motorDriver.moveBackward();
        break;
    }
}
} // namespace

OperatingMode* StandbyMode::startButtonPressed(Controller& controller)
{
    controller.motorDriver.moveForward();
    controller.cleanerDriver.startCleaning();
    return new NormalMode();
}

OperatingMode* StandbyMode::powerButtonPressed(Controller&)
{
    return this;
}

OperatingMode* StandbyMode::dustDetected(Controller&)
{
    return this;
}

OperatingMode* StandbyMode::lowBatteryDetected(Controller& controller)
{
    controller.motorDriver.stopMoving();
    controller.cleanerDriver.stopCleaning();
    return new LowBatteryMode();
}

OperatingMode* StandbyMode::lowBatteryCleared(Controller&)
{
    return this;
}

OperatingMode* StandbyMode::timerExpired(Controller&)
{
    return this;
}

OperatingMode* StandbyMode::obstacleDetected(Controller&)
{
    return this;
}

void StandbyMode::checkIsMoving(Direction, MotorDriver& motorDriver)
{
    motorDriver.stopMoving();
}

bool StandbyMode::canCharge() const
{
    return true;
}

OperatingMode* NormalMode::startButtonPressed(Controller& controller)
{
    controller.motorDriver.stopMoving();
    controller.cleanerDriver.stopCleaning();
    return new StandbyMode();
}

OperatingMode* NormalMode::powerButtonPressed(Controller&)
{
    return this;
}

OperatingMode* NormalMode::dustDetected(Controller& controller)
{
    controller.cleanerDriver.decideSetting(true);
    return new BoostMode();
}

OperatingMode* NormalMode::lowBatteryDetected(Controller& controller)
{
    controller.motorDriver.stopMoving();
    controller.cleanerDriver.stopCleaning();
    return new LowBatteryMode();
}

OperatingMode* NormalMode::lowBatteryCleared(Controller&)
{
    return this;
}

OperatingMode* NormalMode::timerExpired(Controller&)
{
    return this;
}

OperatingMode* NormalMode::obstacleDetected(Controller&)
{
    return this;
}

void NormalMode::checkIsMoving(Direction direction, MotorDriver& motorDriver)
{
    moveByDirection(direction, motorDriver);
}

bool NormalMode::canCharge() const
{
    return false;
}

OperatingMode* BoostMode::startButtonPressed(Controller& controller)
{
    controller.motorDriver.stopMoving();
    controller.cleanerDriver.stopCleaning();
    return new StandbyMode();
}

OperatingMode* BoostMode::powerButtonPressed(Controller&)
{
    return this;
}

OperatingMode* BoostMode::dustDetected(Controller&)
{
    return this;
}

OperatingMode* BoostMode::lowBatteryDetected(Controller& controller)
{
    controller.motorDriver.stopMoving();
    controller.cleanerDriver.stopCleaning();
    return new LowBatteryMode();
}

OperatingMode* BoostMode::lowBatteryCleared(Controller&)
{
    return this;
}

OperatingMode* BoostMode::timerExpired(Controller& controller)
{
    controller.cleanerDriver.decideSetting(false);
    return new NormalMode();
}

OperatingMode* BoostMode::obstacleDetected(Controller&)
{
    return this;
}

void BoostMode::checkIsMoving(Direction direction, MotorDriver& motorDriver)
{
    moveByDirection(direction, motorDriver);
}

bool BoostMode::canCharge() const
{
    return false;
}

OperatingMode* LowBatteryMode::startButtonPressed(Controller& controller)
{
    controller.motorDriver.stopMoving();
    controller.cleanerDriver.stopCleaning();
    return this;
}

OperatingMode* LowBatteryMode::powerButtonPressed(Controller&)
{
    return this;
}

OperatingMode* LowBatteryMode::dustDetected(Controller&)
{
    return this;
}

OperatingMode* LowBatteryMode::lowBatteryDetected(Controller& controller)
{
    controller.motorDriver.stopMoving();
    controller.cleanerDriver.stopCleaning();
    return this;
}

OperatingMode* LowBatteryMode::lowBatteryCleared(Controller&)
{
    return new StandbyMode();
}

OperatingMode* LowBatteryMode::timerExpired(Controller&)
{
    return this;
}

OperatingMode* LowBatteryMode::obstacleDetected(Controller&)
{
    return this;
}

void LowBatteryMode::checkIsMoving(Direction, MotorDriver& motorDriver)
{
    motorDriver.stopMoving();
}

bool LowBatteryMode::canCharge() const
{
    return true;
}

} // namespace rvc
