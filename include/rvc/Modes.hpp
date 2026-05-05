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
        bool checkIsMoving() const override;
        OperatingMode &startButtonPressed() override;
        OperatingMode &lowBatteryDetected() override;
        OperatingMode &lowBatteryCleared() override;
        OperatingMode &dustDetected() override;
        bool canCharge() const override;
        OperatingMode &timerExpired() override;
        void apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) const override;
        ModeKind kind() const override;
        const char *name() const override;
    };

    class NormalMode : public OperatingMode
    {
    public:
        bool checkIsMoving() const override;
        OperatingMode &startButtonPressed() override;
        OperatingMode &lowBatteryDetected() override;
        OperatingMode &lowBatteryCleared() override;
        OperatingMode &dustDetected() override;
        bool canCharge() const override;
        OperatingMode &timerExpired() override;
        void apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) const override;
        ModeKind kind() const override;
        const char *name() const override;
    };

    class BoostMode : public OperatingMode
    {
    public:
        bool checkIsMoving() const override;
        OperatingMode &startButtonPressed() override;
        OperatingMode &lowBatteryDetected() override;
        OperatingMode &lowBatteryCleared() override;
        OperatingMode &dustDetected() override;
        bool canCharge() const override;
        OperatingMode &timerExpired() override;
        void apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) const override;
        ModeKind kind() const override;
        const char *name() const override;
    };

    class LowBatteryMode : public OperatingMode
    {
    public:
        bool checkIsMoving() const override;
        OperatingMode &startButtonPressed() override;
        OperatingMode &lowBatteryDetected() override;
        OperatingMode &lowBatteryCleared() override;
        OperatingMode &dustDetected() override;
        bool canCharge() const override;
        OperatingMode &timerExpired() override;
        void apply(CleanerDriver &cleanerDriver, MotorDriver &motorDriver) const override;
        ModeKind kind() const override;
        const char *name() const override;
    };

} // namespace rvc
