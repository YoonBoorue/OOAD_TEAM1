#include "rvc/Controller.hpp"
#include "rvc/OperatingMode.hpp"
#include "rvc/Modes.hpp"
#include "rvc/BatteryDriver.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/DustSensorDriver.hpp"
#include "rvc/DustProcessor.hpp"
#include "rvc/MotorDriver.hpp"
#include "rvc/ObstacleProcessor.hpp"

namespace rvc
{

    Controller::Controller()
        : power(false), isNowCharging(false), currentMode(nullptr), batteryDriver(new BatteryDriver()), cleanerDriver(new CleanerDriver()), motorDriver(new MotorDriver()), dustSensorDriver(new DustSensorDriver()), dustProcessor(new DustProcessor()), obstacleProcessor(new ObstacleProcessor())
    {
    }

    Controller::~Controller()
    {
        delete batteryDriver;
        delete cleanerDriver;
        delete dustSensorDriver;
        delete dustProcessor;
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
        // TODO
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
        bool fw = this->motorDriver->checkIsForward();
        obstacleProcessor->decideDirection(dir, *currentMode, *motorDriver);
    }

}