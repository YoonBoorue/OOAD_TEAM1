#pragma once

#include "rvc/Controller.hpp"

#include "sim/Environment.hpp"

#include <string>

namespace sim
{

class RvcAdapter
{
public:
    RvcAdapter();

    void powerOnAndStart();
    void feedSensors(const SensorSnapshot &sensors);
    void powerButtonPressed();
    void startButtonPressed();
    void chargeBattery();
    void chargingTick();
    void stopCharging();
    void setBatteryLevel(int batteryLevel);

    ActuatorSnapshot actuators() const;
    rvc::Direction heading() const;

    std::string modeName() const;
    int batteryLevel() const;
    bool isPowerOn() const;
    bool isCharging() const;
    bool isLowBatteryMode() const;

private:
    rvc::Controller controller_;
    int boostTicks_;
    // [추가] map simulator는 robot heading과 실제 좌표 이동 방향을 adapter에서 유지한다.
    rvc::Direction heading_;
    rvc::Direction movementDirection_;
};

} // namespace sim
