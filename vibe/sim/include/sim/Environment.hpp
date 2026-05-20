#pragma once

#include "rvc/Direction.hpp"

#include "sim/Pose.hpp"
#include "sim/Room.hpp"

#include <array>

namespace sim
{

struct SensorSnapshot
{
    bool dustDetected = false;
    std::array<bool, 3> obstacleBlocked = {false, false, false};
    int batteryLevel = 100;
};

struct ActuatorSnapshot
{
    bool motorMoving = false;
    bool motorForward = true;
    rvc::Direction motorDirection = rvc::Direction::Forward;
    bool cleanerRunning = false;
    bool cleanerBoost = false;
};

class Environment
{
public:
    Environment(Room room, Pose initialPose, int initialBatteryLevel);

    const Room &room() const;
    const Pose &pose() const;
    int batteryLevel() const;
    int cleanedCells() const;

    SensorSnapshot sense(rvc::Direction heading) const;
    void applyActuators(const ActuatorSnapshot &actuators);

private:
    bool blockedInDirection(rvc::Direction direction) const;
    void drainBattery(const ActuatorSnapshot &actuators);

    Room room_;
    Pose pose_;
    int batteryLevel_;
    int cleanedCells_;
};

} // namespace sim
