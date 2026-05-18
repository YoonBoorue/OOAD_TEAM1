#ifndef VIBE_RVC_DUST_SENSOR_DRIVER_HPP
#define VIBE_RVC_DUST_SENSOR_DRIVER_HPP

namespace rvc
{

class DustSensorDriver
{
public:
    bool dustDetected;

    DustSensorDriver() = default;

    void initialize();
    void deactivateDustSensor();
    void clear();
};

} // namespace rvc

#endif // VIBE_RVC_DUST_SENSOR_DRIVER_HPP
