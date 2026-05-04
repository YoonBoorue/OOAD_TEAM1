#include "rvc/MotorDriver.hpp"

namespace rvc {

MotorDriver::MotorDriver() : status(false), moveDirection(Direction::FRONT), forward(false) {}

void MotorDriver::initialize() {
    // TODO
}

void MotorDriver::moveForward() {
    // TODO
}

void MotorDriver::stopMoving() {
    // TODO
}

void MotorDriver::turnLeft() {
    // TODO
}

void MotorDriver::turnRight() {
    // TODO
}

void MotorDriver::moveBackward() {
    // TODO
}

bool MotorDriver::checkIsForward() {
    return false;
}

}