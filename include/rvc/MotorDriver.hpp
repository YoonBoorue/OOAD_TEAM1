#pragma once
#include "rvc/Direction.hpp"

namespace rvc
{

    class MotorDriver
    {
    private:
        bool status;
        Direction moveDirection;
        bool forward;

    public:
        MotorDriver();
        void initialize();
        void moveForward();
        void stopMoving();
        void turnLeft();
        void turnRight();
        void moveBackward();

        bool checkIsForward();
        bool isMoving() const;
        Direction currentDirection() const;
    };

}