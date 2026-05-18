#ifndef VIBE_RVC_OBSTACLE_PROCESSOR_HPP
#define VIBE_RVC_OBSTACLE_PROCESSOR_HPP

#include "rvc/Direction.hpp"

namespace rvc
{

class ObstacleSensorDriver;

class ObstacleProcessor
{
public:
    ObstacleProcessor() = default;

    Direction decideDirection(const ObstacleSensorDriver& obstacleSensorDriver) const;
};

} // namespace rvc

#endif // VIBE_RVC_OBSTACLE_PROCESSOR_HPP
