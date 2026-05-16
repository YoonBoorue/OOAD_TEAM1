#pragma once

#include "rvc/OperatingMode.hpp"

namespace rvc
{

    OperatingMode &standbyMode();
    OperatingMode &normalMode();
    OperatingMode &boostMode();
    OperatingMode &lowBatteryMode();

    class StandbyMode : public OperatingMode
    {
    public:
        void checkIsMoving(Direction direction, MotorDriver &motorDriver) const override;

        OperatingMode &startButtonPressed(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) override;
        OperatingMode &lowBatteryDetected(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) override;
        OperatingMode &lowBatteryCleared() override;
        OperatingMode &dustDetected(CleanerDriver &cleanerDriver) override;
        OperatingMode &timerExpired(CleanerDriver &cleanerDriver) override;
        bool canCharge() const override;
        ModeKind kind() const override;
        const char *name() const override;
    };

    class NormalMode : public OperatingMode
    {
    public:
        void checkIsMoving(Direction direction, MotorDriver &motorDriver) const override;

        OperatingMode &startButtonPressed(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) override;
        OperatingMode &lowBatteryDetected(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) override;
        OperatingMode &lowBatteryCleared() override;
        OperatingMode &dustDetected(CleanerDriver &cleanerDriver) override;
        OperatingMode &timerExpired(CleanerDriver &cleanerDriver) override;
        bool canCharge() const override;
        ModeKind kind() const override;
        const char *name() const override;
    };

    class BoostMode : public OperatingMode
    {
    public:
        void checkIsMoving(Direction direction, MotorDriver &motorDriver) const override;

        OperatingMode &startButtonPressed(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) override;
        OperatingMode &lowBatteryDetected(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) override;
        OperatingMode &lowBatteryCleared() override;
        OperatingMode &dustDetected(CleanerDriver &cleanerDriver) override;
        OperatingMode &timerExpired(CleanerDriver &cleanerDriver) override;
        bool canCharge() const override;
        ModeKind kind() const override;
        const char *name() const override;
    };

    class LowBatteryMode : public OperatingMode
    {
    public:
        void checkIsMoving(Direction direction, MotorDriver &motorDriver) const override;

        OperatingMode &startButtonPressed(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) override;
        OperatingMode &lowBatteryDetected(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) override;
        OperatingMode &lowBatteryCleared() override;
        OperatingMode &dustDetected(CleanerDriver &cleanerDriver) override;
        OperatingMode &timerExpired(CleanerDriver &cleanerDriver) override;
        bool canCharge() const override;
        ModeKind kind() const override;
        const char *name() const override;
    };

} // namespace rvc