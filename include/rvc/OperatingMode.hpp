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

    // 상태 패턴 인터페이스
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

        // Cleaner/Motor side effects are owned by each mode.
        // Controller calls this method, but does not decide which driver method to call.
        virtual void apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) const = 0;

        virtual ModeKind kind() const = 0;
        virtual const char *name() const = 0;
    };

} // namespace rvc
