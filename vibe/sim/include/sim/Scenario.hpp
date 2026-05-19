#pragma once

#include "sim/Environment.hpp"

#include <string>

namespace sim
{

struct Scenario
{
    std::string name;
    Environment environment;
};

} // namespace sim
