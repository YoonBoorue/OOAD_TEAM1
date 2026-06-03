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
    // [변경] obstacle input은 front/left 두 센서만 포함한다.
    std::array<bool, 2> obstacleBlocked = {false, false};
    // [추가] right availability는 turnRight 후 front sensor recheck로 제공한다.
    bool frontAfterRightTurnBlocked = false;
    bool leftAfterBackwardBlocked = false;
    bool frontAfterBackwardRightCheckBlocked = false;
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
    void setBatteryLevel(int batteryLevel);
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
