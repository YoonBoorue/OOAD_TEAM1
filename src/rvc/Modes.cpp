#include "rvc/Modes.hpp"

namespace rvc
{

    // --- StandbyMode ---
    void StandbyMode::checkIsMoving(Direction direction, MotorDriver &motor) const {}
    OperatingMode &StandbyMode::startButtonPressed(MotorDriver &motor, CleanerDriver &cleaner) { return *this; }
    OperatingMode &StandbyMode::lowBatteryDetected(MotorDriver &motor, CleanerDriver &cleaner) { return *this; }
    OperatingMode &StandbyMode::lowBatteryCleared() { return *this; }

    OperatingMode &NormalMode::dustDetected(CleanerDriver &cleaner) 
    {
        static BoostMode boost;
        return boost;
    }
    bool StandbyMode::canCharge() const { return true; }
    
    OperatingMode &StandbyMode::timerExpired(CleanerDriver &cleaner) 
    {
        cleaner.decideSetting(false);
        static NormalMode normal;
        return normal;
    }

    // --- NormalMode ---
    void NormalMode::checkIsMoving(Direction direction, MotorDriver &motor) const
    {
        if (direction == Direction::LEFT)
        {
            motor.turnLeft();
            motor.moveForward();
        }
        else if (direction == Direction::RIGHT)
        {
            motor.turnRight();
            motor.moveForward();
        }
        else if (direction == Direction::BACK)
        {
            motor.moveBackward();
        }
    }
    OperatingMode &NormalMode::startButtonPressed(MotorDriver &motor, CleanerDriver &cleaner) { return *this; }
    OperatingMode &NormalMode::lowBatteryDetected(MotorDriver &motor, CleanerDriver &cleaner) { return *this; }
    OperatingMode &NormalMode::lowBatteryCleared() { return *this; }
    OperatingMode &NormalMode::dustDetected(CleanerDriver &cleaner) { return *this; }
    bool NormalMode::canCharge() const { return false; }
    OperatingMode &NormalMode::timerExpired(CleanerDriver &cleaner) {}

    // --- BoostMode ---
    void BoostMode::checkIsMoving(Direction direction, MotorDriver &motor) const
    {
        if (direction == Direction::LEFT)
        {
            motor.turnLeft();
            motor.moveForward();
        }
        else if (direction == Direction::RIGHT)
        {
            motor.turnRight();
            motor.moveForward();
        }
        else if (direction == Direction::BACK)
        {
            motor.moveBackward();
        }
    }
    OperatingMode &BoostMode::startButtonPressed(MotorDriver &motor, CleanerDriver &cleaner) { return *this; }
    OperatingMode &BoostMode::lowBatteryDetected(MotorDriver &motor, CleanerDriver &cleaner) { return *this; }
    OperatingMode &BoostMode::lowBatteryCleared() { return *this; }
    OperatingMode &BoostMode::dustDetected(CleanerDriver &cleaner) { return *this; }
    bool BoostMode::canCharge() const { return false; }
    OperatingMode &BoostMode::timerExpired(CleanerDriver &cleaner) {}

    // --- LowBatteryMode ---
    void LowBatteryMode::checkIsMoving(Direction direction, MotorDriver &motor) const {}
    OperatingMode &LowBatteryMode::startButtonPressed(MotorDriver &motor, CleanerDriver &cleaner) { return *this; }
    OperatingMode &LowBatteryMode::lowBatteryDetected(MotorDriver &motor, CleanerDriver &cleaner) { return *this; }
    OperatingMode &LowBatteryMode::lowBatteryCleared() { return *this; }
    OperatingMode &LowBatteryMode::dustDetected(CleanerDriver &cleaner) { return *this; }
    bool LowBatteryMode::canCharge() const { return false; }
    OperatingMode &LowBatteryMode::timerExpired(CleanerDriver &cleaner) {}

} // namespace rvc
