#pragma once
#include "rvc/Direction.hpp"

namespace rvc {

enum class MotionCommand {
    MoveForward,
    TurnLeft,
    TurnRight,
    MoveBackward
};

struct SensorSnapshot {
    bool frontObstacle = false;
    bool leftObstacle = false;
    bool rightObstacle = false;
    bool dustDetected = false;
};

struct ControlCommand {
    MotionCommand motion = MotionCommand::MoveForward;
    bool cleaningPowerUp = false;
};

class RobotController {
public:
    explicit RobotController(Direction preferredTurn = Direction::LEFT);

    ControlCommand decide(const SensorSnapshot& sensors) const;

private:
    Direction preferredTurn_;
};

} // namespace rvc