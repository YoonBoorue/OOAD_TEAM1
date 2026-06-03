#ifndef VIBE_RVC_OBSTACLE_PROCESSOR_HPP
#define VIBE_RVC_OBSTACLE_PROCESSOR_HPP

#include "rvc/Direction.hpp"

namespace rvc
{

class ObstacleSensorDriver;

class ObstacleProcessor
{
public:
    ObstacleProcessor() = default;

    // [변경] 1차 판단은 right sensor 없이 front/left input만 사용한다.
    Direction decideDirection(const ObstacleSensorDriver& obstacleSensorDriver) const;
    // [추가] 우회전 후 front sensor recheck 결과로 right 진행 가능 여부를 판단한다.
    Direction decideDirectionAfterRightTurn(const ObstacleSensorDriver& obstacleSensorDriver) const;
    // [추가] 후진 중 재확인 결과에서 left 우선순위로 빈 방향을 선택한다.
    Direction decideDirectionAfterBackwardRecheck(const ObstacleSensorDriver& obstacleSensorDriver) const;
};

} // namespace rvc

#endif // VIBE_RVC_OBSTACLE_PROCESSOR_HPP
