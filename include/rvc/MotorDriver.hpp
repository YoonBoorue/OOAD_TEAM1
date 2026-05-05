#pragma once
#include "rvc/Direction.hpp"

namespace rvc {

class MotorDriver {
private:
    // status: MotorDriver가 moving 상태인지 stop moving 상태인지 
    // forward: 바퀴가 전진 방향인지 후진 방향인지 
    // moveDirection: 바퀴 방향이 아니라 RVC 몸체가 바라보는 방향
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
};

}