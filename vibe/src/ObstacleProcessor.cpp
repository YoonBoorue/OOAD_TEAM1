#include "../include/rvc/ObstacleProcessor.hpp"

#include "../include/rvc/ObstacleSensorDriver.hpp"

namespace rvc
{

// [변경] right sensor 입력을 읽지 않고 front/left만으로 1차 회피 방향을 결정한다.
Direction ObstacleProcessor::decideDirection(const ObstacleSensorDriver& obstacleSensorDriver) const
{
    if (!obstacleSensorDriver.front)
    {
        return Direction::Forward;
    }

    if (!obstacleSensorDriver.left)
    {
        return Direction::Left;
    }

    return Direction::Right;
}

// [추가] 우회전 후 front sensor 재확인 결과로 right 진행 가능 여부를 결정한다.
Direction ObstacleProcessor::decideDirectionAfterRightTurn(
    const ObstacleSensorDriver& obstacleSensorDriver) const
{
    if (obstacleSensorDriver.isFrontClearAfterRightTurn())
    {
        return Direction::Forward;
    }

    return Direction::Backward;
}

// [추가] 후진 중 재확인된 빈 방향을 선택하되 left를 우선한다.
Direction ObstacleProcessor::decideDirectionAfterBackwardRecheck(
    const ObstacleSensorDriver& obstacleSensorDriver) const
{
    if (obstacleSensorDriver.isLeftClearAfterBackward())
    {
        return Direction::Left;
    }

    if (obstacleSensorDriver.isRightClearAfterBackward())
    {
        return Direction::Right;
    }

    return Direction::Backward;
}

} // namespace rvc
