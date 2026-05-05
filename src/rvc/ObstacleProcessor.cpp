#include "rvc/ObstacleProcessor.hpp"

namespace rvc
{

    void ObstacleProcessor::decideDirection(const std::array<bool, 3> &dir, const OperatingMode &currentMode, MotorDriver &motor)
    {
        Direction direction;
        bool fw = motor.checkIsForward();
        if (fw)
        { // 전진상황
            if (!dir[0])
                direction = Direction::FRONT;
            else if (!dir[1])
                direction = Direction::LEFT;
            else if (!dir[2])
                direction = Direction::RIGHT;
            else
                direction = Direction::BACK;
        }
        else
        { // 후진(장애물 탈출)
            if (!dir[1])
                direction = Direction::LEFT;
            else if (!dir[2])
                direction = Direction::RIGHT;
            else
                direction = Direction::BACK;
        }
        currentMode.checkIsMoving(direction, motor);
    }

} // namespace rvc
