#pragma once

namespace rvc {

enum class TurnDirection {
    Left,
    Right
};

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
    explicit RobotController(TurnDirection preferredTurn = TurnDirection::Left);

    ControlCommand decide(const SensorSnapshot& sensors) const;

private:
    TurnDirection preferredTurn_;
};

} // namespace rvc