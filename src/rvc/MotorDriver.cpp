#include "rvc/MotorDriver.hpp"
#include "rvc/Direction.hpp"

namespace rvc
{

    MotorDriver::MotorDriver() : status(false), moveDirection(Direction::FRONT), forward(false) {}

    void MotorDriver::initialize()
    {
    }

    void MotorDriver::moveForward()
    {
        this->forward = true;
        this->status = true;
    }

    void MotorDriver::stopMoving()
    {
        this->status = false;
    }

    void MotorDriver::turnLeft()
    {
        switch (this->moveDirection)
        {
        case Direction::FRONT:
            this->moveDirection = Direction::LEFT;
            break;
        case Direction::LEFT:
            this->moveDirection = Direction::BACK;
            break;
        case Direction::RIGHT:
            this->moveDirection = Direction::FRONT;
            break;
        case Direction::BACK:
            this->moveDirection = Direction::RIGHT;
            break;
        }
    }

    void MotorDriver::turnRight()
    {
        switch (this->moveDirection)
        {
        case Direction::FRONT:
            this->moveDirection = Direction::RIGHT;
            break;
        case Direction::LEFT:
            this->moveDirection = Direction::FRONT;
            break;
        case Direction::RIGHT:
            this->moveDirection = Direction::BACK;
            break;
        case Direction::BACK:
            this->moveDirection = Direction::LEFT;
            break;
        }
    }

    void MotorDriver::moveBackward()
    {
        this->forward = false;
    }

    bool MotorDriver::checkIsForward()
    {
        return false;
    }

}