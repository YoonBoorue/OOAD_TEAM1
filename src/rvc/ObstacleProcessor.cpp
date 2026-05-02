#include "rvc/ObstacleProcessor.hpp"

namespace rvc {

Direction ObstacleProcessor::decideDirection(const std::array<bool, 3>& /*dir*/, 
                                            const OperatingMode& /*currentMode*/, 
                                            bool /*fw*/) {
    return Direction::FRONT;
}

Direction ObstacleProcessor::decideExit(const std::array<bool, 2>& /*dir*/) {
    return Direction::FRONT;
}

} // namespace rvc
