#pragma once

#include "sim/Cell.hpp"
#include "sim/Pose.hpp"

#include <vector>

namespace sim
{

class Room
{
public:
    Room(int width, int height);

    int width() const;
    int height() const;

    bool inBounds(Position position) const;
    bool isBlocked(Position position) const;
    bool hasDust(Position position) const;

    void setObstacle(Position position, bool blocked = true);
    void setDust(Position position, bool dust = true);
    bool cleanDust(Position position);

    int remainingDust() const;
    std::vector<Position> dustCells() const;
    std::vector<Position> obstacleCells() const;

private:
    Cell &cellAt(Position position);
    const Cell &cellAt(Position position) const;
    int index(Position position) const;

    int width_;
    int height_;
    std::vector<Cell> cells_;
};

} // namespace sim
