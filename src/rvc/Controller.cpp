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
        // 충전 중에는 StandbyMode에서 청소를 시작하는 SD-02 동작을 막는다.
        // SD-07의 Normal/BoostMode는 충전 중 도달할 수 없는 상태이므로,
        // 유효한 SD-07 시나리오에서는 이 조건이 항상 통과된다.
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
