#include "sim/Room.hpp"

#include <stdexcept>

namespace sim
{
namespace
{

std::size_t checkedCellCount(int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        throw std::invalid_argument("Room dimensions must be positive");
    }

    return static_cast<std::size_t>(width * height);
}

} // namespace

Room::Room(int width, int height)
    : width_(width),
      height_(height),
      cells_(checkedCellCount(width, height))
{
}

int Room::width() const
{
    return width_;
}

int Room::height() const
{
    return height_;
}

bool Room::inBounds(Position position) const
{
    return position.x >= 0 && position.x < width_ &&
           position.y >= 0 && position.y < height_;
}

bool Room::isBlocked(Position position) const
{
    return !inBounds(position) || cellAt(position).obstacle;
}

bool Room::hasDust(Position position) const
{
    return inBounds(position) && cellAt(position).dust;
}

void Room::setObstacle(Position position, bool blocked)
{
    cellAt(position).obstacle = blocked;
}

void Room::setDust(Position position, bool dust)
{
    cellAt(position).dust = dust;
}

bool Room::cleanDust(Position position)
{
    if (!hasDust(position))
    {
        return false;
    }

    cellAt(position).dust = false;
    return true;
}

int Room::remainingDust() const
{
    int count = 0;
    for (const Cell &cell : cells_)
    {
        if (cell.dust)
        {
            ++count;
        }
    }

    return count;
}

std::vector<Position> Room::dustCells() const
{
    std::vector<Position> positions;
    for (int y = 0; y < height_; ++y)
    {
        for (int x = 0; x < width_; ++x)
        {
            const Position position{x, y};
            if (cellAt(position).dust)
            {
                positions.push_back(position);
            }
        }
    }

    return positions;
}

std::vector<Position> Room::obstacleCells() const
{
    std::vector<Position> positions;
    for (int y = 0; y < height_; ++y)
    {
        for (int x = 0; x < width_; ++x)
        {
            const Position position{x, y};
            if (cellAt(position).obstacle)
            {
                positions.push_back(position);
            }
        }
    }

    return positions;
}

Cell &Room::cellAt(Position position)
{
    if (!inBounds(position))
    {
        throw std::out_of_range("Room position is outside the grid");
    }

    return cells_[static_cast<std::size_t>(index(position))];
}

const Cell &Room::cellAt(Position position) const
{
    if (!inBounds(position))
    {
        throw std::out_of_range("Room position is outside the grid");
    }

    return cells_[static_cast<std::size_t>(index(position))];
}

int Room::index(Position position) const
{
    return position.y * width_ + position.x;
}

} // namespace sim
