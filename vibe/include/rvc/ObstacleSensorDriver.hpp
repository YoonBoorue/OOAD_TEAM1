#ifndef VIBE_RVC_OBSTACLE_SENSOR_DRIVER_HPP
#define VIBE_RVC_OBSTACLE_SENSOR_DRIVER_HPP

namespace rvc
{

class ObstacleSensorDriver
{
public:
    bool front;
    bool left;
    bool right;

    ObstacleSensorDriver() = default;

    void initialize();
    void deactivateObstacleSensor();
    void clear();
    bool hasObstacle() const;
};

} // namespace rvc

#endif // VIBE_RVC_OBSTACLE_SENSOR_DRIVER_HPP
