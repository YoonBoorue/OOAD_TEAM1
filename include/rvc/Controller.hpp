#pragma once

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

    class Controller
    {
    private:
        bool power;
        bool isNowCharging;
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

    public:
        Controller();
        ~Controller();

        Controller(const Controller &) = delete;
        Controller &operator=(const Controller &) = delete;

        bool startTimer();
        void powerButtonPressed();
        void startButtonPressed();
        void chargeBattery();
        void chargingTick();
        void lowBatteryDetected();
        void lowBatteryCleared();
        void stopCharging();
        void dustDetected();
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
