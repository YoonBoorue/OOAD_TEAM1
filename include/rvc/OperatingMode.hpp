#pragma once

namespace rvc
{

    class CleanerDriver;
    class MotorDriver;

    enum class ModeKind
    {
        Standby,
        Normal,
        Boost,
        LowBattery
    };

    // State-pattern interface for RVC operating modes.
    class OperatingMode
    {
    public:
        virtual ~OperatingMode() = default;

        virtual bool checkIsMoving() const = 0;
        virtual OperatingMode &startButtonPressed() = 0;
        virtual OperatingMode &lowBatteryDetected() = 0;
        virtual OperatingMode &lowBatteryCleared() = 0;
        virtual OperatingMode &dustDetected() = 0;
        virtual bool canCharge() const = 0;
        virtual OperatingMode &timerExpired() = 0;

        // Cleaner/Motor commands are intentionally delegated to Mode.
        // Controller directly touches Cleaner/Motor only in power on/off sequences.
        virtual void apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) = 0;

        virtual ModeKind kind() const = 0;
        virtual const char *name() const = 0;
    };

} // namespace rvc
