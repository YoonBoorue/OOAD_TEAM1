#include "sim/RvcAdapter.hpp"

#include "rvc/BatteryDriver.hpp"

namespace sim
{

namespace
{

constexpr int BoostDurationTicks = 5;

} // namespace

RvcAdapter::RvcAdapter()
    : controller_(),
      boostTicks_(0)
{
}

void RvcAdapter::powerOnAndStart()
{
    if (!controller_.isPowerOn())
    {
        controller_.powerButtonPressed();
    }

    if (controller_.currentModeKind() == rvc::ModeKind::Standby)
    {
        controller_.startButtonPressed();
    }
}

void RvcAdapter::feedSensors(const SensorSnapshot &sensors)
{
    controller_.setBatteryLevel(sensors.batteryLevel);

    if (sensors.batteryLevel <= rvc::BatteryDriver::LowBatteryThreshold)
    {
        controller_.lowBatteryDetected();
        return;
    }

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

    if (controller_.currentModeKind() == rvc::ModeKind::Boost)
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
    snapshot.motorMoving = controller_.isMotorMoving();
    snapshot.motorForward = controller_.isMotorForward();
    snapshot.motorDirection = controller_.motorDirection();
    snapshot.cleanerRunning = controller_.isCleanerCleaning();
    snapshot.cleanerBoost = controller_.cleanerMode() == "boost";
    return snapshot;
}

rvc::Direction RvcAdapter::heading() const
{
    return controller_.motorDirection();
}

std::string RvcAdapter::modeName() const
{
    return controller_.currentModeName();
}

int RvcAdapter::batteryLevel() const
{
    return controller_.batteryLevel();
}

bool RvcAdapter::isPowerOn() const
{
    return controller_.isPowerOn();
}

bool RvcAdapter::isCharging() const
{
    return controller_.isCharging();
}

bool RvcAdapter::isLowBatteryMode() const
{
    return controller_.hasCurrentMode() &&
           controller_.currentModeKind() == rvc::ModeKind::LowBattery;
}

} // namespace sim
