#include "rvc/Modes.hpp"

#include "rvc/CleanerDriver.hpp"
#include "rvc/MotorDriver.hpp"

namespace rvc
{

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

    namespace
    {
        void moveByDirection(Direction direction, MotorDriver &motorDriver)
        {
            switch (direction)
            {
            case Direction::FRONT:
                motorDriver.moveForward();
                break;

            case Direction::LEFT:
                motorDriver.turnLeft();
                motorDriver.moveForward();
                break;

            case Direction::RIGHT:
                motorDriver.turnRight();
                motorDriver.moveForward();
                break;

            case Direction::BACK:
                motorDriver.moveBackward();
                break;
            }
        }

        OperatingMode &enterLowBatteryMode(CleanerDriver &cleanerDriver, MotorDriver &motorDriver)
        {
            OperatingMode &nextMode = lowBatteryMode();
            nextMode.apply(cleanerDriver, motorDriver);
            return nextMode;
        }
    }

    // --- StandbyMode ---

    void StandbyMode::checkIsMoving(Direction, MotorDriver &) const {}

    OperatingMode &StandbyMode::startButtonPressed() { return normalMode(); }
    OperatingMode &StandbyMode::lowBatteryDetected(CleanerDriver &cleanerDriver, MotorDriver &motorDriver)
    {
        return enterLowBatteryMode(cleanerDriver, motorDriver);
    }
    OperatingMode &StandbyMode::lowBatteryCleared() { return standbyMode(); }
    OperatingMode &StandbyMode::dustDetected() { return standbyMode(); }
    bool StandbyMode::canCharge() const { return true; }
    OperatingMode &StandbyMode::timerExpired() { return standbyMode(); }

    void StandbyMode::apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver)
    {
        motorDriver.stopMoving();
        cleanerDriver.stopCleaning();
    }

    ModeKind StandbyMode::kind() const { return ModeKind::Standby; }

    const char *StandbyMode::name() const { return "StandbyMode"; }

    // --- NormalMode ---

    void NormalMode::checkIsMoving(Direction direction, MotorDriver &motorDriver) const { moveByDirection(direction, motorDriver); }
    OperatingMode &NormalMode::startButtonPressed() { return standbyMode(); }
    OperatingMode &NormalMode::lowBatteryDetected(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) { return enterLowBatteryMode(cleanerDriver, motorDriver); }
    OperatingMode &NormalMode::lowBatteryCleared() { return normalMode(); }
    OperatingMode &NormalMode::dustDetected() { return boostMode(); }
    bool NormalMode::canCharge() const { return false; }
    OperatingMode &NormalMode::timerExpired() { return normalMode(); }

    void NormalMode::apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver)
    {
        motorDriver.moveForward();
        cleanerDriver.startCleaning();
        cleanerDriver.decideSetting(false);
    }

    ModeKind NormalMode::kind() const { return ModeKind::Normal; }
    const char *NormalMode::name() const { return "NormalMode"; }

    // --- BoostMode ---

    void BoostMode::checkIsMoving(Direction direction, MotorDriver &motorDriver) const { moveByDirection(direction, motorDriver); }
    OperatingMode &BoostMode::startButtonPressed() { return standbyMode(); }
    OperatingMode &BoostMode::lowBatteryDetected(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) { return enterLowBatteryMode(cleanerDriver, motorDriver); }
    OperatingMode &BoostMode::lowBatteryCleared() { return boostMode(); }
    OperatingMode &BoostMode::dustDetected() { return boostMode(); }
    bool BoostMode::canCharge() const { return false; }
    OperatingMode &BoostMode::timerExpired() { return normalMode(); }

    void BoostMode::apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver)
    {
        motorDriver.moveForward();
        cleanerDriver.startCleaning();
        cleanerDriver.decideSetting(true);
    }

    ModeKind BoostMode::kind() const { return ModeKind::Boost; }
    const char *BoostMode::name() const { return "BoostMode"; }

    // --- LowBatteryMode ---

    void LowBatteryMode::checkIsMoving(Direction, MotorDriver &) const {}

    OperatingMode &LowBatteryMode::startButtonPressed() { return lowBatteryMode(); }
    OperatingMode &LowBatteryMode::lowBatteryDetected(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) { return enterLowBatteryMode(cleanerDriver, motorDriver); }
    OperatingMode &LowBatteryMode::lowBatteryCleared() { return standbyMode(); }
    OperatingMode &LowBatteryMode::dustDetected() { return lowBatteryMode(); }
    bool LowBatteryMode::canCharge() const { return true; }
    OperatingMode &LowBatteryMode::timerExpired() { return lowBatteryMode(); }

    void LowBatteryMode::apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver)
    {
        motorDriver.stopMoving();
        cleanerDriver.stopCleaning();
    }

    ModeKind LowBatteryMode::kind() const { return ModeKind::LowBattery; }
    const char *LowBatteryMode::name() const { return "LowBatteryMode"; }

} // namespace rvc