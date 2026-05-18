#ifndef VIBE_RVC_MOTOR_DRIVER_HPP
#define VIBE_RVC_MOTOR_DRIVER_HPP

#include "rvc/Direction.hpp"

namespace rvc
{

class MotorDriver
{
public:
    bool isRunning;
    Direction direction;

    MotorDriver() = default;

    void start(Direction nextDirection);
    void stop();
    void moveForward();
    void turnLeft();
    void turnRight();
    void moveBackward();
};

} // namespace rvc

#endif // VIBE_RVC_MOTOR_DRIVER_HPP
