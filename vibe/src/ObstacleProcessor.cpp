#include "../include/rvc/ObstacleProcessor.hpp"

#include "../include/rvc/ObstacleSensorDriver.hpp"

namespace rvc
{

// [변경] right sensor 입력 제거, front/left 상태와 우회전 후 front 재확인 상태 저장
Direction ObstacleProcessor::decideDirection(const ObstacleSensorDriver& obstacleSensorDriver)
{
    frontBlocked = obstacleSensorDriver.front;
    leftBlocked = obstacleSensorDriver.left;
    frontBlockedAfterRightTurn = obstacleSensorDriver.front && obstacleSensorDriver.left;

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

// [추가] 우회전 후 front sensor 재확인 결과를 processor 내부 상태로 제공
bool ObstacleProcessor::isFrontClearAfterRightTurn() const
{
    return !frontBlockedAfterRightTurn;
}

// [추가] all-blocked 후진 회피 후 비어있는 방향을 왼쪽 우선으로 선택
Direction ObstacleProcessor::decideDirectionAfterBackwardRecheck() const
{
    if (!leftBlocked)
    {
        return Direction::Left;
    }

    if (!frontBlockedAfterRightTurn)
    {
        return Direction::Right;
    }

    return Direction::Backward;
}

} // namespace rvc
