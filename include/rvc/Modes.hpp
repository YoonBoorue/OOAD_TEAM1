#pragma once
#include "rvc/OperatingMode.hpp"

namespace rvc {

// todo 클래스 다이어그램의 각 4가지 모드에서 Void 타입에 대한 논의 필요
class StandbyMode : public OperatingMode {
public:
    bool checkIsMoving() const override;
    OperatingMode& startButtonPressed() override;
    OperatingMode& lowBatteryDetected() override;
    OperatingMode& lowBatteryCleared() override;
    OperatingMode& dustDetected() override;
    bool canCharge() const override;
    void timerExpired() override;
};

class NormalMode : public OperatingMode {
public:
    bool checkIsMoving() const override;
    OperatingMode& startButtonPressed() override;
    OperatingMode& lowBatteryDetected() override;
    OperatingMode& lowBatteryCleared() override;
    OperatingMode& dustDetected() override;
    bool canCharge() const override;
    void timerExpired() override;
};

class BoostMode : public OperatingMode {
public:
    bool checkIsMoving() const override;
    OperatingMode& startButtonPressed() override;
    OperatingMode& lowBatteryDetected() override;
    OperatingMode& lowBatteryCleared() override;
    OperatingMode& dustDetected() override;
    bool canCharge() const override;
    void timerExpired() override;
};

class LowBatteryMode : public OperatingMode {
public:
    bool checkIsMoving() const override;
    OperatingMode& startButtonPressed() override;
    OperatingMode& lowBatteryDetected() override;
    OperatingMode& lowBatteryCleared() override;
    OperatingMode& dustDetected() override;
    bool canCharge() const override;
    void timerExpired() override;
};

} // namespace rvc
