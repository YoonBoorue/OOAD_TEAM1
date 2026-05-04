#pragma once

namespace rvc {

// 상태 패턴 인터페이스
class OperatingMode {
public:
    virtual ~OperatingMode() = default;

    virtual void checkIsMoving() const = 0;
    virtual OperatingMode& startButtonPressed() = 0;
    virtual OperatingMode& lowBatteryDetected() = 0;
    virtual OperatingMode& lowBatteryCleared() = 0;
    virtual OperatingMode& dustDetected() = 0;
    virtual bool canCharge() const = 0;
    virtual void timerExpired() = 0;
};

} // namespace rvc
