#include "sim/RvcAdapter.hpp"

#include "rvc/BatteryDriver.hpp"

#include "sim/DirectionUtil.hpp"

namespace sim
{

namespace
{

constexpr int BoostDurationTicks = 5;

// [추가] 후진 이동을 현재 heading의 반대 좌표 방향으로 변환한다.
rvc::Direction backOf(rvc::Direction direction)
{
    switch (direction)
    {
    case rvc::Direction::Forward:
        return rvc::Direction::Backward;
    case rvc::Direction::Left:
        return rvc::Direction::Right;
    case rvc::Direction::Right:
        return rvc::Direction::Left;
    case rvc::Direction::Backward:
        return rvc::Direction::Forward;
    }

    return rvc::Direction::Backward;
}

template <typename Mode>
bool isCurrentMode(const rvc::Controller &controller)
{
    return dynamic_cast<Mode *>(controller.currentMode) != nullptr;
}

std::string currentModeName(const rvc::Controller &controller)
{
    if (controller.currentMode == nullptr)
    {
        return "Off";
    }

    if (isCurrentMode<rvc::StandbyMode>(controller))
    {
        return "StandbyMode";
    }

    if (isCurrentMode<rvc::NormalMode>(controller))
    {
        return "NormalMode";
    }

    if (isCurrentMode<rvc::BoostMode>(controller))
    {
        return "BoostMode";
    }

    if (isCurrentMode<rvc::LowBatteryMode>(controller))
    {
        return "LowBatteryMode";
    }

    return "UnknownMode";
}

} // namespace

RvcAdapter::RvcAdapter()
    : controller_(),
      boostTicks_(0),
      heading_(rvc::Direction::Forward),
      movementDirection_(rvc::Direction::Forward)
{
    controller_.clockTick();
}

void RvcAdapter::powerOnAndStart()
{
    if (controller_.currentMode == nullptr)
    {
        controller_.powerButtonPressed();
    }

    if (isCurrentMode<rvc::StandbyMode>(controller_))
    {
        controller_.startButtonPressed();
    }
}

void RvcAdapter::feedSensors(const SensorSnapshot &sensors)
{
    const rvc::Direction headingBeforeSensors = heading_;
    controller_.batteryDriver.level = sensors.batteryLevel;

    if (sensors.batteryLevel <= rvc::BatteryDriver::LowBatteryThreshold)
    {
        controller_.lowBatteryDetected();
        return;
    }

    if (isCurrentMode<rvc::LowBatteryMode>(controller_))
    {
        controller_.lowBatteryCleared();
    }

    controller_.dustSensorDriver.dustDetected = sensors.dustDetected;
    if (sensors.dustDetected)
    {
        controller_.dustDetected();
    }

    // [변경] immediate front/left obstacle이 없으면 recheck state만으로 obstacle flow를 호출하지 않는다.
    const bool hasObstacleEvent =
        sensors.obstacleBlocked[0] ||
        sensors.obstacleBlocked[1];
    if (!hasObstacleEvent)
    {
        movementDirection_ = heading_;
        return;
    }

    // [변경] right sensor input 제거로 front/left만 immediate obstacle input으로 전달한다.
    controller_.obstacleSensorDriver.setObstacleInput(sensors.obstacleBlocked[0],
                                                      sensors.obstacleBlocked[1]);
    // [추가] right path는 turnRight 후 front sensor recheck 결과로 전달한다.
    controller_.obstacleSensorDriver.setFrontAfterRightTurn(
        sensors.frontAfterRightTurnBlocked);
    // [추가] 후진 중 left 우선/오른쪽 확인도 recheck state로 전달한다.
    controller_.obstacleSensorDriver.setBackwardRecheck(
        sensors.leftAfterBackwardBlocked,
        sensors.frontAfterBackwardRightCheckBlocked);
    controller_.obstacleDetected();

    // [추가] Controller의 relative turn+forward 결과를 map 좌표상의 실제 이동 방향으로 변환한다.
    if (sensors.obstacleBlocked[0])
    {
        if (!sensors.obstacleBlocked[1])
        {
            heading_ = leftOf(headingBeforeSensors);
            movementDirection_ = heading_;
        }
        else if (!sensors.frontAfterRightTurnBlocked)
        {
            heading_ = rightOf(headingBeforeSensors);
            movementDirection_ = heading_;
        }
        else if (!sensors.leftAfterBackwardBlocked)
        {
            heading_ = leftOf(headingBeforeSensors);
            movementDirection_ = heading_;
        }
        else if (!sensors.frontAfterBackwardRightCheckBlocked)
        {
            heading_ = rightOf(headingBeforeSensors);
            movementDirection_ = heading_;
        }
        else
        {
            movementDirection_ = backOf(headingBeforeSensors);
        }
    }
    else
    {
        movementDirection_ = heading_;
    }

    if (isCurrentMode<rvc::BoostMode>(controller_))
    {
        ++boostTicks_;
        if (boostTicks_ >= BoostDurationTicks)
        {
            controller_.timerExpiredNow();
            boostTicks_ = 0;
        }
    }
    else
    {
        boostTicks_ = 0;
    }
}

void RvcAdapter::powerButtonPressed()
{
    controller_.powerButtonPressed();
}

void RvcAdapter::startButtonPressed()
{
    controller_.startButtonPressed();
}

void RvcAdapter::chargeBattery()
{
    controller_.chargeBattery();
}

void RvcAdapter::chargingTick()
{
    controller_.chargingTick();
}

void RvcAdapter::stopCharging()
{
    controller_.stopCharging();
}

void RvcAdapter::setBatteryLevel(int batteryLevel)
{
    controller_.batteryDriver.level = batteryLevel;
}

ActuatorSnapshot RvcAdapter::actuators() const
{
    ActuatorSnapshot snapshot;
    snapshot.motorMoving = controller_.motorDriver.isRunning;
    snapshot.motorDirection = movementDirection_;
    snapshot.motorForward = controller_.motorDriver.direction != rvc::Direction::Backward;
    snapshot.cleanerRunning = controller_.cleanerDriver.isRunning;
    snapshot.cleanerBoost = controller_.cleanerDriver.isBoosting;
    return snapshot;
}

rvc::Direction RvcAdapter::heading() const
{
    return heading_;
}

std::string RvcAdapter::modeName() const
{
    return currentModeName(controller_);
}

int RvcAdapter::batteryLevel() const
{
    return controller_.batteryDriver.level;
}

bool RvcAdapter::isPowerOn() const
{
    return controller_.currentMode != nullptr;
}

bool RvcAdapter::isCharging() const
{
    return controller_.batteryDriver.isCharging;
}

bool RvcAdapter::isLowBatteryMode() const
{
    return isCurrentMode<rvc::LowBatteryMode>(controller_);
}

} // namespace sim
