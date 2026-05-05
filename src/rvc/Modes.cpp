#include "rvc/Modes.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/MotorDriver.hpp"

namespace rvc
{

<<<<<<< feat/power-usecases(1-10-11-16)
    // --- Mode Singleton Accessors ---
    OperatingMode &standbyMode()
    {
        static StandbyMode mode;
        return mode;
    }

    OperatingMode &normalMode()
    {
        static NormalMode mode;
        return mode;
    }

    OperatingMode &boostMode()
    {
        static BoostMode mode;
        return mode;
    }

    OperatingMode &lowBatteryMode()
    {
        static LowBatteryMode mode;
        return mode;
    }

    // --- StandbyMode ---
    bool StandbyMode::checkIsMoving() const { return false; }
    OperatingMode &StandbyMode::startButtonPressed() { return normalMode(); }
    OperatingMode &StandbyMode::lowBatteryDetected() { return lowBatteryMode(); }
    OperatingMode &StandbyMode::lowBatteryCleared() { return standbyMode(); }
    OperatingMode &StandbyMode::dustDetected() { return standbyMode(); }
    bool StandbyMode::canCharge() const { return true; }
    OperatingMode &StandbyMode::timerExpired() { return standbyMode(); }

    void StandbyMode::apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) const
    {
        motorDriver.stopMoving();
        cleanerDriver.stopCleaning();
    }

    ModeKind StandbyMode::kind() const { return ModeKind::Standby; }
    const char *StandbyMode::name() const { return "StandbyMode"; }

    // --- NormalMode ---
    bool NormalMode::checkIsMoving() const { return true; }
    OperatingMode &NormalMode::startButtonPressed() { return standbyMode(); }
    OperatingMode &NormalMode::lowBatteryDetected() { return lowBatteryMode(); }
    OperatingMode &NormalMode::lowBatteryCleared() { return normalMode(); }
    OperatingMode &NormalMode::dustDetected() { return boostMode(); }
    bool NormalMode::canCharge() const { return false; }
    OperatingMode &NormalMode::timerExpired() { return normalMode(); }

    void NormalMode::apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) const
    {
        // UC2/UC3/UC4 behavior is delegated to the mode instead of Controller.
        motorDriver.initialize();
        motorDriver.moveForward();
        cleanerDriver.startCleaning();
        cleanerDriver.decideSetting(false);
    }

    ModeKind NormalMode::kind() const { return ModeKind::Normal; }
    const char *NormalMode::name() const { return "NormalMode"; }

    // --- BoostMode ---
    bool BoostMode::checkIsMoving() const { return true; }
    OperatingMode &BoostMode::startButtonPressed() { return standbyMode(); }
    OperatingMode &BoostMode::lowBatteryDetected() { return lowBatteryMode(); }
    OperatingMode &BoostMode::lowBatteryCleared() { return boostMode(); }
    OperatingMode &BoostMode::dustDetected() { return boostMode(); }
    bool BoostMode::canCharge() const { return false; }
    OperatingMode &BoostMode::timerExpired() { return normalMode(); }

    void BoostMode::apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) const
    {
        // UC6 itself is handled in another branch, but the mode owns its Cleaner/Motor effect.
        motorDriver.moveForward();
        cleanerDriver.startCleaning();
        cleanerDriver.decideSetting(true);
    }

    ModeKind BoostMode::kind() const { return ModeKind::Boost; }
    const char *BoostMode::name() const { return "BoostMode"; }

    // --- LowBatteryMode ---
    bool LowBatteryMode::checkIsMoving() const { return false; }
    OperatingMode &LowBatteryMode::startButtonPressed() { return lowBatteryMode(); }
    OperatingMode &LowBatteryMode::lowBatteryDetected() { return lowBatteryMode(); }
    OperatingMode &LowBatteryMode::lowBatteryCleared() { return standbyMode(); }
    OperatingMode &LowBatteryMode::dustDetected() { return lowBatteryMode(); }
    bool LowBatteryMode::canCharge() const { return true; }
    OperatingMode &LowBatteryMode::timerExpired() { return lowBatteryMode(); }

    void LowBatteryMode::apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) const
    {
        // SD-15 references UC8 Stop Moving and UC9 Stop Cleaning.
        motorDriver.stopMoving();
        cleanerDriver.stopCleaning();
    }

    ModeKind LowBatteryMode::kind() const { return ModeKind::LowBattery; }
    const char *LowBatteryMode::name() const { return "LowBatteryMode"; }

} // namespace rvc
=======
    // --- StandbyMode ---
    void StandbyMode::checkIsMoving(Direction direction, MotorDriver &motor) const {}
    OperatingMode &StandbyMode::startButtonPressed(MotorDriver &motor, CleanerDriver &cleaner) { return *this; }
    OperatingMode &StandbyMode::lowBatteryDetected(MotorDriver &motor, CleanerDriver &cleaner) { return *this; }
    OperatingMode &StandbyMode::lowBatteryCleared() { return *this; }
    OperatingMode &StandbyMode::dustDetected(CleanerDriver &cleaner) { return *this; }
    bool StandbyMode::canCharge() const { return true; }
    OperatingMode &StandbyMode::timerExpired(CleanerDriver &cleaner) {}

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
>>>>>>> main
