#pragma once

#include "rvc/Direction.hpp"

#include "sim/Pose.hpp"

namespace sim
{

const char *directionName(rvc::Direction direction);
Position unitVector(rvc::Direction direction);
Position opposite(const Position &delta);
rvc::Direction leftOf(rvc::Direction direction);
rvc::Direction rightOf(rvc::Direction direction);

} // namespace sim
