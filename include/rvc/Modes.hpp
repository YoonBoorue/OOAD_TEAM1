#pragma once
#include "rvc/OperatingMode.hpp"

namespace rvc
{

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
        ModeKind kind() const override;
        const char *name() const override;
    };

    OperatingMode &standbyMode();
    OperatingMode &normalMode();
    OperatingMode &boostMode();
    OperatingMode &lowBatteryMode();

} // namespace rvc
