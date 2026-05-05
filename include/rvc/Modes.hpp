#pragma once
#include "rvc/OperatingMode.hpp"
#include "rvc/Direction.hpp"
#include "rvc/MotorDriver.hpp"
#include "rvc/CleanerDriver.hpp"

namespace rvc
{

    // todo 클래스 다이어그램의 각 4가지 모드에서 Void 타입에 대한 논의 필요
    class StandbyMode : public OperatingMode
    {
    public:
        void checkIsMoving(Direction direction, MotorDriver &motor) const override;
        OperatingMode &startButtonPressed(MotorDriver &motor, CleanerDriver &cleaner) override;
        OperatingMode &lowBatteryDetected(MotorDriver &motor, CleanerDriver &cleaner) override;
        OperatingMode &lowBatteryCleared() override;
        OperatingMode &dustDetected(CleanerDriver &cleaner) override;
        bool canCharge() const override;
        OperatingMode &timerExpired(CleanerDriver &cleaner) override;
    };

    class NormalMode : public OperatingMode
    {
    public:
        void checkIsMoving(Direction direction, MotorDriver &motor) const override;
        OperatingMode &startButtonPressed(MotorDriver &motor, CleanerDriver &cleaner) override;
        OperatingMode &lowBatteryDetected(MotorDriver &motor, CleanerDriver &cleaner) override;
        OperatingMode &lowBatteryCleared() override;
        OperatingMode &dustDetected(CleanerDriver &cleaner) override;
        bool canCharge() const override;
        OperatingMode &timerExpired(CleanerDriver &cleaner) override;
    };

    class BoostMode : public OperatingMode
    {
    public:
        void checkIsMoving(Direction direction, MotorDriver &motor) const override;
        OperatingMode &startButtonPressed(MotorDriver &motor, CleanerDriver &cleaner) override;
        OperatingMode &lowBatteryDetected(MotorDriver &motor, CleanerDriver &cleaner) override;
        OperatingMode &lowBatteryCleared() override;
        OperatingMode &dustDetected(CleanerDriver &cleaner) override;
        bool canCharge() const override;
        OperatingMode &timerExpired(CleanerDriver &cleaner) override;
    };

    class LowBatteryMode : public OperatingMode
    {
    public:
        void checkIsMoving(Direction direction, MotorDriver &motor) const override;
        OperatingMode &startButtonPressed(MotorDriver &motor, CleanerDriver &cleaner) override;
        OperatingMode &lowBatteryDetected(MotorDriver &motor, CleanerDriver &cleaner) override;
        OperatingMode &lowBatteryCleared() override;
        OperatingMode &dustDetected(CleanerDriver &cleaner) override;
        bool canCharge() const override;
        OperatingMode &timerExpired(CleanerDriver &cleaner) override;
    };

} // namespace rvc
