#pragma once
#include "rvc/MotorDriver.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/Direction.hpp"

namespace rvc
{

    // 상태 패턴 인터페이스
    class OperatingMode
    {
    public:
        virtual ~OperatingMode() = default;

        virtual void checkIsMoving(Direction direction, MotorDriver &motor) const = 0;
        virtual OperatingMode &startButtonPressed(MotorDriver &motor, CleanerDriver &cleaner) = 0;
        virtual OperatingMode &lowBatteryDetected(MotorDriver &motor, CleanerDriver &cleaner) = 0;
        virtual OperatingMode &lowBatteryCleared() = 0;
        virtual OperatingMode &dustDetected(CleanerDriver &cleaner) = 0;
        virtual bool canCharge() const = 0;
        virtual OperatingMode &timerExpired(CleanerDriver &cleaner) = 0;
    };

} // namespace rvc
