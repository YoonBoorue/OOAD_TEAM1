#pragma once
#include <array>

namespace rvc
{

    class OperatingMode;
    class BatteryDriver;
    class CleanerDriver;
    class MotorDriver;
    class DustSensorDriver;
    class DustProcessor;
    class ObstacleProcessor;
    class ObstacleSensorDriver;

    class Controller
    {
    private:
        bool power;
        bool isNowCharging;
        OperatingMode *currentMode;
        BatteryDriver *batteryDriver;
        CleanerDriver *cleanerDriver;
        MotorDriver *motorDriver;
        DustSensorDriver *dustSensorDriver;
        DustProcessor *dustProcessor;
        ObstacleProcessor *obstacleProcessor;
        ObstacleSensorDriver *obstacleSensorDriver;

    public:
        Controller();
        ~Controller();
        Controller(const Controller &) = delete;
        Controller &operator=(const Controller &) = delete;

        bool startTimer();
        void powerButtonPressed();
        void startButtonPressed();
        void chargeBattery();
        void lowBatteryDetected();
        void lowBatteryCleared();
        void stopCharging();
        void dustDetected();
        void obstacleDetected(const std::array<bool, 3> &dir);
    };

}