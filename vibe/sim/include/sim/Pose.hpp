#pragma once

namespace sim
{

struct Position
{
    int x = 0;
    int y = 0;
};

struct Pose
{
    Position position;
};

bool operator==(const Position &lhs, const Position &rhs);
bool operator!=(const Position &lhs, const Position &rhs);

Position operator+(const Position &lhs, const Position &rhs);
Position operator-(const Position &lhs, const Position &rhs);

} // namespace sim
