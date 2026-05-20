#pragma once

#include "sim/Scenario.hpp"

#include <iosfwd>
#include <stdexcept>
#include <string>

namespace sim
{

class MapParseError final : public std::runtime_error
{
public:
    MapParseError(int line, const std::string &message);

    int line() const;

private:
    int line_;
};

Scenario loadScenarioFromMap(std::istream &input, const std::string &name);
Scenario loadScenarioFromMapFile(const std::string &path);

} // namespace sim
