#include "rvc/ObstacleSensorDriver.hpp"

namespace rvc
{

    ObstacleSensorDriver::ObstacleSensorDriver()
        : direction{false, false}, active(false) {}

    void ObstacleSensorDriver::initialize()
    {
        active = true;
        direction[0] = false;
        direction[1] = false;
    }

    void ObstacleSensorDriver::deactivateObstacleSensor()
    {
        active = false;
        direction[0] = false;
        direction[1] = false;
    }

    bool ObstacleSensorDriver::isActive() const { return active; }

}