#pragma once

namespace rvc {

class OperatingMode;
class BatteryDriver;
class CleanerDriver;
class DustSensorDriver;
class DustProcessor;

class Controller {
private:
    bool power;
    bool isNowCharging;
    OperatingMode* currentMode;
    BatteryDriver* batteryDriver;
    CleanerDriver* cleanerDriver;
    DustSensorDriver* dustSensorDriver;
    DustProcessor* dustProcessor;

public:
    Controller();
    ~Controller();
    Controller(const Controller&) = delete;
    Controller& operator=(const Controller&) = delete;

    bool startTimer();
    void powerButtonPressed();
    void startButtonPressed();
    void chargeBattery();
    void lowBatteryDetected();
    void lowBatteryCleared();
    void stopCharging();
    void dustDetected();
    void obstacleDetected(const bool direction[3]);
};

}