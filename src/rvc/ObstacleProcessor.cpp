#include "rvc/ObstacleProcessor.hpp"

namespace rvc
{
    ObstacleProcessor::ObstacleProcessor()
        : direction_{false, false, false}, checkR(false) {}

    void ObstacleProcessor::decideDirection(const std::array<bool, 2> &dir, const OperatingMode &currentMode, MotorDriver &motor)
    {
        Direction direction;
        bool fw = motor.checkIsForward();

        if (checkR)
        { // 우회전 후 전방 센서 확인 상황
            if (!dir[0])
                direction = Direction::FRONT;
            else
            {
                direction = Direction::BACK;
            }
            checkR = false;
        }
        else if (fw)
        { // 전진상황
            if (!dir[0])
                direction = Direction::FRONT;
            else if (!dir[1])
                direction = Direction::LEFT;
            else
            {
                direction = Direction::RIGHT;
                checkR = true;
            }
        }
        else
        { // 후진(장애물 탈출)
            if (!dir[1])
                direction = Direction::LEFT;
            else
            {
                direction = Direction::RIGHT;
                checkR = true;
            }
        }
        currentMode.checkIsMoving(direction, motor);
    }

} // namespace rvc
