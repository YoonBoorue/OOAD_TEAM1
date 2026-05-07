#pragma once

#include "rvc/Direction.hpp"

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

    class OperatingMode
    {
    public:
        virtual ~OperatingMode() = default;

        virtual void checkIsMoving(Direction direction, MotorDriver &motorDriver) const = 0;

        virtual OperatingMode &startButtonPressed(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) = 0;

        // SD-15: Mode receives Motor/Cleaner and performs stop actions.
        virtual OperatingMode &lowBatteryDetected(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) = 0;

        virtual OperatingMode &lowBatteryCleared() = 0;
        virtual OperatingMode &dustDetected(CleanerDriver &cleanerDriver) = 0;
        virtual bool canCharge() const = 0;
        virtual OperatingMode &timerExpired(CleanerDriver &cleanerDriver) = 0;

        virtual ModeKind kind() const = 0;
        virtual const char *name() const = 0;
    };

} // namespace rvc