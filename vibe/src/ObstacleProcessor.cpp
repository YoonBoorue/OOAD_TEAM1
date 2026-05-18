#include "../include/rvc/ObstacleProcessor.hpp"

#include "../include/rvc/ObstacleSensorDriver.hpp"

namespace rvc
{

Direction ObstacleProcessor::decideDirection(const ObstacleSensorDriver& obstacleSensorDriver) const
{
    if (!obstacleSensorDriver.front)
    {
        return Direction::Forward;
    }

    if (!obstacleSensorDriver.left)
    {
        return Direction::Left;
    }

    if (!obstacleSensorDriver.right)
    {
        return Direction::Right;
    }

    return Direction::Backward;
}

} // namespace rvc
