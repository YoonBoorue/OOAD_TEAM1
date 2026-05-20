#include "sim/Environment.hpp"

#include "rvc/BatteryDriver.hpp"

#include "sim/DirectionUtil.hpp"

#include <algorithm>
#include <utility>

namespace sim
{
namespace
{

constexpr int MotorDrain = 1;
constexpr int CleanerDrain = 1;
constexpr int BoostDrain = 1;
constexpr int MinBatteryLevel = 0;

} // namespace

Environment::Environment(Room room, Pose initialPose, int initialBatteryLevel)
    : room_(std::move(room)),
      pose_(initialPose),
      batteryLevel_(std::clamp(initialBatteryLevel,
                               MinBatteryLevel,
                               rvc::BatteryDriver::FullBatteryLevel)),
      cleanedCells_(0)
{
}

const Room &Environment::room() const
{
    return room_;
}

const Pose &Environment::pose() const
{
    return pose_;
}

int Environment::batteryLevel() const
{
    return batteryLevel_;
}

void Environment::setBatteryLevel(int batteryLevel)
{
    batteryLevel_ = std::clamp(batteryLevel,
                               MinBatteryLevel,
                               rvc::BatteryDriver::FullBatteryLevel);
}

int Environment::cleanedCells() const
{
    return cleanedCells_;
}

SensorSnapshot Environment::sense(rvc::Direction heading) const
{
    SensorSnapshot snapshot;
    snapshot.dustDetected = room_.hasDust(pose_.position);
    snapshot.obstacleBlocked = {
        blockedInDirection(heading),
        blockedInDirection(leftOf(heading)),
        blockedInDirection(rightOf(heading)),
    };
    snapshot.batteryLevel = batteryLevel_;
    return snapshot;
}

void Environment::applyActuators(const ActuatorSnapshot &actuators)
{
    if (actuators.cleanerRunning && room_.cleanDust(pose_.position))
    {
        ++cleanedCells_;
    }

    if (actuators.motorMoving)
    {
        Position delta = unitVector(actuators.motorDirection);
        const Position next = pose_.position + delta;
        if (!room_.isBlocked(next))
        {
            pose_.position = next;
        }
    }

    drainBattery(actuators);
}

bool Environment::blockedInDirection(rvc::Direction direction) const
{
    return room_.isBlocked(pose_.position + unitVector(direction));
}

void Environment::drainBattery(const ActuatorSnapshot &actuators)
{
    int drain = 0;
    if (actuators.motorMoving)
    {
        drain += MotorDrain;
    }
    if (actuators.cleanerRunning)
    {
        drain += CleanerDrain;
    }
    if (actuators.cleanerBoost)
    {
        drain += BoostDrain;
    }

    batteryLevel_ = std::max(MinBatteryLevel, batteryLevel_ - drain);
}

} // namespace sim
