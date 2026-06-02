#ifndef VIBE_RVC_OBSTACLE_SENSOR_DRIVER_HPP
#define VIBE_RVC_OBSTACLE_SENSOR_DRIVER_HPP

namespace rvc
{

class ObstacleSensorDriver
{
public:
    // [변경] front/left는 현재 obstacle input의 blocked 상태만 나타낸다.
    bool front;
    bool left;
    // [삭제] f8be8cc 기준 right sensor state는 제거하고 front sensor recheck state로 대체한다.
    // [추가] 우회전/후진 이후 sensor 재확인 결과를 blocked 상태로 저장한다.
    bool frontAfterRightTurn;
    bool leftAfterBackward;
    bool frontAfterBackwardRightCheck;

    ObstacleSensorDriver() = default;

    void initialize();
    void deactivateObstacleSensor();
    void clear();
    void setObstacleInput(bool frontBlocked, bool leftBlocked);
    void setFrontAfterRightTurn(bool frontBlocked);
    void setBackwardRecheck(bool leftBlocked, bool frontBlockedForRightCheck);
    bool isFrontClearAfterRightTurn() const;
    bool isLeftClearAfterBackward() const;
    bool isRightClearAfterBackward() const;
    bool hasObstacle() const;
};

} // namespace rvc

#endif // VIBE_RVC_OBSTACLE_SENSOR_DRIVER_HPP
