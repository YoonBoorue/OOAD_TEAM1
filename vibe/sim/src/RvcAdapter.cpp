#include "sim/RvcAdapter.hpp"

#include "rvc/BatteryDriver.hpp"

namespace sim
{

namespace
{

constexpr int BoostDurationTicks = 5;

template <typename Mode>
bool isCurrentMode(const rvc::Controller &controller)
{
    return dynamic_cast<Mode *>(controller.currentMode) != nullptr;
}

std::string currentModeName(const rvc::Controller &controller)
{
    if (controller.currentMode == nullptr)
    {
        return "Off";
    }

    if (isCurrentMode<rvc::StandbyMode>(controller))
    {
        return "StandbyMode";
    }

    if (isCurrentMode<rvc::NormalMode>(controller))
    {
        return "NormalMode";
    }

    if (isCurrentMode<rvc::BoostMode>(controller))
    {
        return "BoostMode";
    }

    if (isCurrentMode<rvc::LowBatteryMode>(controller))
    {
        return "LowBatteryMode";
    }

    return "UnknownMode";
}

} // namespace

RvcAdapter::RvcAdapter()
    : controller_(),
      boostTicks_(0)
{
}

void RvcAdapter::powerOnAndStart()
{
    if (controller_.currentMode == nullptr)
    {
        controller_.powerButtonPressed();
    }

    if (isCurrentMode<rvc::StandbyMode>(controller_))
    {
        controller_.startButtonPressed();
    }
}

void RvcAdapter::feedSensors(const SensorSnapshot &sensors)
{
    controller_.batteryDriver.level = sensors.batteryLevel;

    if (sensors.batteryLevel <= rvc::BatteryDriver::LowBatteryThreshold)
    {
        controller_.lowBatteryDetected();
        return;
    }

    if (isCurrentMode<rvc::LowBatteryMode>(controller_))
    {
        controller_.lowBatteryCleared();
    }

    controller_.dustSensorDriver.dustDetected = sensors.dustDetected;
    if (sensors.dustDetected)
    {
        controller_.dustDetected();
    }

    const bool blocked[3] = {
        sensors.obstacleBlocked[0],
        sensors.obstacleBlocked[1],
        sensors.obstacleBlocked[2],
    };
    controller_.obstacleDetected(blocked);

    if (isCurrentMode<rvc::BoostMode>(controller_))
    {
        ++boostTicks_;
        if (boostTicks_ >= BoostDurationTicks)
        {
            controller_.timerExpiredNow();
            boostTicks_ = 0;
        }
    }
    else
    {
        boostTicks_ = 0;
    }
}

void RvcAdapter::powerButtonPressed()
{
    controller_.powerButtonPressed();
}

void RvcAdapter::startButtonPressed()
{
    controller_.startButtonPressed();
}

void RvcAdapter::chargeBattery()
{
    controller_.chargeBattery();
}

void RvcAdapter::chargingTick()
{
    controller_.chargingTick();
}

void RvcAdapter::stopCharging()
{
    controller_.stopCharging();
}

ActuatorSnapshot RvcAdapter::actuators() const
{
    ActuatorSnapshot snapshot;
    snapshot.motorMoving = controller_.motorDriver.isRunning;
    snapshot.motorForward = true;
    snapshot.motorDirection = controller_.motorDriver.direction;
    snapshot.cleanerRunning = controller_.cleanerDriver.isRunning;
    snapshot.cleanerBoost = controller_.cleanerDriver.isBoosting;
    return snapshot;
}

rvc::Direction RvcAdapter::heading() const
{
    return controller_.motorDriver.direction;
}

std::string RvcAdapter::modeName() const
{
    return currentModeName(controller_);
}

int RvcAdapter::batteryLevel() const
{
    return controller_.batteryDriver.level;
}

bool RvcAdapter::isPowerOn() const
{
    return controller_.currentMode != nullptr;
}

bool RvcAdapter::isCharging() const
{
    return controller_.batteryDriver.isCharging;
}

bool RvcAdapter::isLowBatteryMode() const
{
    return isCurrentMode<rvc::LowBatteryMode>(controller_);
}

} // namespace sim
