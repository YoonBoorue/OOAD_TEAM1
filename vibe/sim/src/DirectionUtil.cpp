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
    case rvc::Direction::Forward:
        return "Forward";
    case rvc::Direction::Left:
        return "Left";
    case rvc::Direction::Right:
        return "Right";
    case rvc::Direction::Backward:
        return "Backward";
    }

    return "UNKNOWN";
}

Position unitVector(rvc::Direction direction)
{
    switch (direction)
    {
    case rvc::Direction::Forward:
        return Position{0, -1};
    case rvc::Direction::Left:
        return Position{-1, 0};
    case rvc::Direction::Right:
        return Position{1, 0};
    case rvc::Direction::Backward:
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
    case rvc::Direction::Forward:
        return rvc::Direction::Left;
    case rvc::Direction::Left:
        return rvc::Direction::Backward;
    case rvc::Direction::Right:
        return rvc::Direction::Forward;
    case rvc::Direction::Backward:
        return rvc::Direction::Right;
    }

    return rvc::Direction::Forward;
}

rvc::Direction rightOf(rvc::Direction direction)
{
    switch (direction)
    {
    case rvc::Direction::Forward:
        return rvc::Direction::Right;
    case rvc::Direction::Left:
        return rvc::Direction::Forward;
    case rvc::Direction::Right:
        return rvc::Direction::Backward;
    case rvc::Direction::Backward:
        return rvc::Direction::Left;
    }

    return rvc::Direction::Forward;
}

} // namespace sim
