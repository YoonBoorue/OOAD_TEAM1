#include "rvc/DustSensorDriver.hpp"

namespace rvc
{
    DustSensorDriver::DustSensorDriver()
        : dust(false), active(false) {}

    void DustSensorDriver::initialize()
    {
        active = true;
        dust = false;
    }

    void DustSensorDriver::deactivateDustSensor()
    {
        active = false;
        dust = false;
    }

    bool DustSensorDriver::isActive() const { return active; }

    bool DustSensorDriver::isDustDetected() const { return dust; }

}