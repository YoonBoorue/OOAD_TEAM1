#pragma once
#include <array>

<<<<<<< feat/power-usecases(1-10-11-16)
#include <string>

#include "rvc/OperatingMode.hpp"

namespace rvc
{

    class BatteryDriver;
    class CleanerDriver;
    class DustSensorDriver;
    class MotorDriver;
    class ObstacleSensorDriver;

    enum class MotionState;
=======
namespace rvc
{

    class OperatingMode;
    class BatteryDriver;
    class CleanerDriver;
    class MotorDriver;
    class DustSensorDriver;
    class DustProcessor;
    class ObstacleProcessor;
>>>>>>> main

    class Controller
    {
    private:
        bool power;
        bool isNowCharging;
<<<<<<< feat/power-usecases(1-10-11-16)
        bool lowBatteryClearedEventSent;

        OperatingMode *currentMode; // non-owning pointer to mode singleton

        BatteryDriver *batteryDriver;
        CleanerDriver *cleanerDriver;
        DustSensorDriver *dustSensorDriver;
        MotorDriver *motorDriver;
        ObstacleSensorDriver *obstacleSensorDriver;

        void enterMode(OperatingMode &nextMode);
        bool canStartCharging() const;
        void performChargingStep();
=======
        OperatingMode *currentMode;
        BatteryDriver *batteryDriver;
        CleanerDriver *cleanerDriver;
        MotorDriver *motorDriver;
        DustSensorDriver *dustSensorDriver;
        DustProcessor *dustProcessor;
        ObstacleProcessor *obstacleProcessor;
>>>>>>> main

    public:
        Controller();
        ~Controller();
<<<<<<< feat/power-usecases(1-10-11-16)

=======
>>>>>>> main
        Controller(const Controller &) = delete;
        Controller &operator=(const Controller &) = delete;

        bool startTimer();
        void powerButtonPressed();
        void startButtonPressed();
        void chargeBattery();
<<<<<<< feat/power-usecases(1-10-11-16)
        void chargingTick();
=======
>>>>>>> main
        void lowBatteryDetected();
        void lowBatteryCleared();
        void stopCharging();
        void dustDetected();
<<<<<<< feat/power-usecases(1-10-11-16)
        void obstacleDetected(const bool direction[3]);

        // Test / simulator accessors
        bool isPowerOn() const;
        bool isCharging() const;
        int batteryLevel() const;
        bool isBatteryPowered() const;
        bool hasCurrentMode() const;
        ModeKind currentModeKind() const;
        std::string currentModeName() const;
        bool isDustSensorActive() const;
        bool isObstacleSensorActive() const;
        bool isCleanerCleaning() const;
        std::string cleanerMode() const;
        MotionState motorMotion() const;

        // Simulator/test support
        void setBatteryLevel(int level);
    };

} // namespace rvc
=======
        void obstacleDetected(const std::array<bool, 3> &dir);
    };

}
>>>>>>> main
