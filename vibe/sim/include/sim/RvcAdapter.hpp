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
};

} // namespace sim
