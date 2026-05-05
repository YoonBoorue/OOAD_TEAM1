#include "rvc/Controller.hpp"
#include "rvc/OperatingMode.hpp"
#include "rvc/Modes.hpp"
#include "rvc/BatteryDriver.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/DustSensorDriver.hpp"
#include "rvc/DustProcessor.hpp"
#include "rvc/MotorDriver.hpp"
#include "rvc/ObstacleProcessor.hpp"
#include "rvc/ObstacleSensorDriver.hpp"

namespace rvc
{

    Controller::Controller()
        : power(false), isNowCharging(false), currentMode(nullptr),
          batteryDriver(new BatteryDriver()), cleanerDriver(new CleanerDriver()), motorDriver(new MotorDriver()),
          dustSensorDriver(new DustSensorDriver()), dustProcessor(new DustProcessor()), obstacleProcessor(new ObstacleProcessor()),
          obstacleSensorDriver(new ObstacleSensorDriver())
    {
    }

    Controller::~Controller()
    {
        delete batteryDriver;
        delete cleanerDriver;
        delete motorDriver;
        delete dustSensorDriver;
        delete dustProcessor;
        delete obstacleProcessor;
        delete obstacleSensorDriver;
        delete currentMode;
    }

    bool Controller::startTimer()
    {
        return false;
    }

    void Controller::powerButtonPressed()
    {
        // TODO
    }

    void Controller::startButtonPressed()
    {
        if (!this->isNowCharging)
            currentMode = &currentMode->startButtonPressed(*motorDriver, *cleanerDriver);
    }

    void Controller::chargeBattery()
    {
        // TODO
    }

    void Controller::lowBatteryDetected()
    {
        // TODO
    }

    void Controller::lowBatteryCleared()
    {
        // TODO
    }

    void Controller::stopCharging()
    {
        // TODO
    }

    void Controller::dustDetected()
    {
        // TODO
    }

    void Controller::obstacleDetected(const std::array<bool, 3> &dir)
    {
        obstacleProcessor->decideDirection(dir, *currentMode, *motorDriver);
    }

}