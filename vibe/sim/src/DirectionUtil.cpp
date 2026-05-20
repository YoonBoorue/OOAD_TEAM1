#include "sim/DirectionUtil.hpp"

namespace sim
{

bool operator==(const Position &lhs, const Position &rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!=(const Position &lhs, const Position &rhs)
{
    return !(lhs == rhs);
}

Position operator+(const Position &lhs, const Position &rhs)
{
    return Position{lhs.x + rhs.x, lhs.y + rhs.y};
}

Position operator-(const Position &lhs, const Position &rhs)
{
    return Position{lhs.x - rhs.x, lhs.y - rhs.y};
}

const char *directionName(rvc::Direction direction)
{
    switch (direction)
    {
    case rvc::Direction::FRONT:
        return "FRONT";
    case rvc::Direction::LEFT:
        return "LEFT";
    case rvc::Direction::RIGHT:
        return "RIGHT";
    case rvc::Direction::BACK:
        return "BACK";
    }

    return "UNKNOWN";
}

Position unitVector(rvc::Direction direction)
{
    switch (direction)
    {
    case rvc::Direction::FRONT:
        return Position{0, -1};
    case rvc::Direction::LEFT:
        return Position{-1, 0};
    case rvc::Direction::RIGHT:
        return Position{1, 0};
    case rvc::Direction::BACK:
        return Position{0, 1};
    }

    return Position{0, 0};
}

Position opposite(const Position &delta)
{
    return Position{-delta.x, -delta.y};
}

rvc::Direction leftOf(rvc::Direction direction)
{
    switch (direction)
    {
    case rvc::Direction::FRONT:
        return rvc::Direction::LEFT;
    case rvc::Direction::LEFT:
        return rvc::Direction::BACK;
    case rvc::Direction::RIGHT:
        return rvc::Direction::FRONT;
    case rvc::Direction::BACK:
        return rvc::Direction::RIGHT;
    }

    return rvc::Direction::FRONT;
}

rvc::Direction rightOf(rvc::Direction direction)
{
    switch (direction)
    {
    case rvc::Direction::FRONT:
        return rvc::Direction::RIGHT;
    case rvc::Direction::LEFT:
        return rvc::Direction::FRONT;
    case rvc::Direction::RIGHT:
        return rvc::Direction::BACK;
    case rvc::Direction::BACK:
        return rvc::Direction::LEFT;
    }

    return rvc::Direction::FRONT;
}

} // namespace sim
