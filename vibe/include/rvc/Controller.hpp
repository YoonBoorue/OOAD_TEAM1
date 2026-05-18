#ifndef VIBE_RVC_CONTROLLER_HPP
#define VIBE_RVC_CONTROLLER_HPP

#include "rvc/BatteryDriver.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/Direction.hpp"
#include "rvc/DustProcessor.hpp"
#include "rvc/DustSensorDriver.hpp"
#include "rvc/MotorDriver.hpp"
#include "rvc/ObstacleProcessor.hpp"
#include "rvc/ObstacleSensorDriver.hpp"
#include "rvc/OperatingMode.hpp"

namespace rvc
{

class Controller
{
public:
    BatteryDriver batteryDriver;
    CleanerDriver cleanerDriver;
    MotorDriver motorDriver;
    ObstacleSensorDriver obstacleSensorDriver;
    DustSensorDriver dustSensorDriver;
    ObstacleProcessor obstacleProcessor;
    DustProcessor dustProcessor;

    StandbyMode standbyMode;
    NormalMode normalMode;
    BoostMode boostMode;
    LowBatteryMode lowBatteryMode;
    OperatingMode* currentMode;

    Controller() = default;

    void powerButtonPressed();
    void startButtonPressed();
    void chargeBattery();
    void stopCharging();
    void lowBatteryDetected();
    void dustDetected();
    void obstacleDetected();
    void timerExpired();
};

} // namespace rvc

#endif // VIBE_RVC_CONTROLLER_HPP
