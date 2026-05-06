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
    class ObstacleProcessor;

    enum class MotionState;

    class Controller
    {
    private:
        bool power;
        bool lowBatteryClearedEventSent;

        OperatingMode *currentMode; // non-owning pointer to mode singleton

        BatteryDriver *batteryDriver;
        CleanerDriver *cleanerDriver;
        DustSensorDriver *dustSensorDriver;
        MotorDriver *motorDriver;
        ObstacleSensorDriver *obstacleSensorDriver;
        ObstacleProcessor *obstacleProcessor;

        void enterMode(OperatingMode &nextMode);
        bool canStartCharging() const;
        void performChargingStep();

    public:
        Controller();
        ~Controller();

        Controller(const Controller &) = delete;
        Controller &operator=(const Controller &) = delete;

        bool startTimer();

        // UC1 / UC11
        void powerButtonPressed();
        void startButtonPressed();

        // UC10 / UC16
        void chargeBattery();
        void chargingTick();
        void stopCharging();

        // UC15 and low-battery recovery
        void lowBatteryDetected();
        void lowBatteryCleared();

        void dustDetected();
        void obstacleDetected(const bool direction[3]);

        // Test / simulator accessors
        bool isPowerOn() const;
        bool isCharging() const;
        int batteryLevel() const;
        bool hasCurrentMode() const;
        ModeKind currentModeKind() const;
        std::string currentModeName() const;

        // Simulator/test support
        void setBatteryLevel(int level);
    };

}