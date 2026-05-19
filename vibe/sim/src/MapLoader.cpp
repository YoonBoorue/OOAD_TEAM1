#include "sim/MapLoader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim
{
namespace
{

bool isAllowedGlyph(char glyph)
{
    return glyph == '.' || glyph == '*' || glyph == '#' || glyph == 'R';
}

std::string formatError(int line, const std::string &message)
{
    std::ostringstream output;
    output << "line " << line << ": " << message;
    return output.str();
}

} // namespace

MapParseError::MapParseError(int line, const std::string &message)
    : std::runtime_error(formatError(line, message)),
      line_(line)
{
}

int MapParseError::line() const
{
    return line_;
}

Scenario loadScenarioFromMap(std::istream &input, const std::string &name)
{
    int width = 0;
    int height = 0;
    if (!(input >> width >> height))
    {
        throw MapParseError(1, "expected map size header: W H");
    }
    if (width <= 0 || height <= 0)
    {
        throw MapParseError(1, "map width and height must be positive");
    }

    std::string row;
    std::getline(input, row);

    Room room(width, height);
    Pose pose;
    bool foundRobot = false;

    for (int y = 0; y < height; ++y)
    {
        const int line = y + 2;
        if (!std::getline(input, row))
        {
            throw MapParseError(line, "missing map row");
        }
        if (static_cast<int>(row.size()) != width)
        {
            throw MapParseError(line, "row width does not match header");
        }

        for (int x = 0; x < width; ++x)
        {
            const char glyph = row[static_cast<std::size_t>(x)];
            if (!isAllowedGlyph(glyph))
            {
                throw MapParseError(line, std::string("invalid glyph: ") + glyph);
            }

            const Position position{x, y};
            if (glyph == '#')
            {
                room.setObstacle(position);
            }
            else if (glyph == '*')
            {
                room.setDust(position);
            }
            else if (glyph == 'R')
            {
                if (foundRobot)
                {
                    throw MapParseError(line, "multiple robot start positions");
                }
                pose.position = position;
                foundRobot = true;
            }
        }
    }

    if (!foundRobot)
    {
        throw MapParseError(height + 1, "missing robot start position");
    }

    Scenario scenario{name, Environment(std::move(room), pose, 100)};
    return scenario;
}

Scenario loadScenarioFromMapFile(const std::string &path)
{
    std::ifstream input(path);
    if (!input)
    {
        throw std::runtime_error("could not open map file: " + path);
    }

    return loadScenarioFromMap(input, path);
}

} // namespace sim
