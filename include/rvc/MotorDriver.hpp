#pragma once

namespace rvc {

class MotorDriver {
private:
    bool leftWheel;
    bool rightWheel;

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