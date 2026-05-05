#include "rvc/RobotController.hpp"

namespace rvc {

RobotController::RobotController(Direction preferredTurn)
    : preferredTurn_(preferredTurn) {}

ControlCommand RobotController::decide(const SensorSnapshot& sensors) const {
    ControlCommand command;
    command.cleaningPowerUp = sensors.dustDetected;

    if (!sensors.frontObstacle) {
        command.motion = MotionCommand::MoveForward;
        return command;
    }

    if (preferredTurn_ == Direction::LEFT) {
        if (!sensors.leftObstacle) {
            command.motion = MotionCommand::TurnLeft;
        } else if (!sensors.rightObstacle) {
            command.motion = MotionCommand::TurnRight;
        } else {
            command.motion = MotionCommand::MoveBackward;
        }
    } else {
        if (!sensors.rightObstacle) {
            command.motion = MotionCommand::TurnRight;
        } else if (!sensors.leftObstacle) {
            command.motion = MotionCommand::TurnLeft;
        } else {
            command.motion = MotionCommand::MoveBackward;
        }
    }

    return command;
}

} // namespace rvc