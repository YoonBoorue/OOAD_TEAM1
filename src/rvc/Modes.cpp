#include "rvc/Modes.hpp"

#include "rvc/CleanerDriver.hpp"
#include "rvc/MotorDriver.hpp"

namespace rvc
{

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

    void StandbyMode::apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver)
    {
        // SD-07 references SD-08 and SD-09. Mode performs Cleaner/Motor control.
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

    void NormalMode::apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver)
    {
        // SD-02 delegates SD-03 and SD-04 behavior to current mode.
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

    void BoostMode::apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver)
    {
        // UC6 is handled by another branch, but BoostMode behavior is ready for integration.
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

    void LowBatteryMode::apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver)
    {
        // SD-15 references SD-08 and SD-09. Mode performs Cleaner/Motor control.
        motorDriver.stopMoving();
        cleanerDriver.stopCleaning();
    }

    ModeKind LowBatteryMode::kind() const { return ModeKind::LowBattery; }
    const char *LowBatteryMode::name() const { return "LowBatteryMode"; }

} // namespace rvc
