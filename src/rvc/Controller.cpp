#include <thread>
#include <chrono>

#include "rvc/Controller.hpp"

#include "rvc/BatteryDriver.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/DustSensorDriver.hpp"
#include "rvc/Modes.hpp"
#include "rvc/MotorDriver.hpp"
#include "rvc/ObstacleSensorDriver.hpp"
#include "rvc/ObstacleProcessor.hpp"
#include "rvc/DustProcessor.hpp"

namespace rvc
{

    Controller::Controller()
        : power(false),
          isNowCharging(false),
          currentMode(nullptr),
          batteryDriver(new BatteryDriver()),
          cleanerDriver(new CleanerDriver()),
          dustSensorDriver(new DustSensorDriver()),
          motorDriver(new MotorDriver()),
          obstacleSensorDriver(new ObstacleSensorDriver()),
          obstacleProcessor(new ObstacleProcessor()),
          dustProcessor(new DustProcessor()) {}

    Controller::~Controller()
    {
        delete dustProcessor;
        delete obstacleProcessor;
        delete obstacleSensorDriver;
        delete motorDriver;
        delete dustSensorDriver;
        delete cleanerDriver;
        delete batteryDriver;

        // currentMode is not deleted because it points to a static mode singleton.
    }

    bool Controller::canStartCharging() const
    {

        if (power)
            return currentMode->canCharge();
        else
            return true;
    }

    bool Controller::startTimer()
    {
        std::thread([this]()
                    {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            if (currentMode != nullptr) {
                currentMode = &currentMode->timerExpired(*cleanerDriver); 
            } })
            .detach();
        return true;
    }

    void Controller::powerButtonPressed()
    {
        if (!power)
        {
            // SD-01 TURN ON SYSTEM
            power = true;

            batteryDriver->initialize();
            currentMode = &standbyMode();

            // Power on is one of the two allowed direct Controller -> Cleaner/Motor paths.
            dustSensorDriver->initialize();
            cleanerDriver->initialize();
            motorDriver->initialize();
            obstacleSensorDriver->initialize();
            return;
        }

        // SD-11 TURN OFF SYSTEM
        power = false;
        isNowCharging = false;

        batteryDriver->turnOffBattery();
        obstacleSensorDriver->deactivateObstacleSensor();
        dustSensorDriver->deactivateDustSensor();

        // Power off is the other allowed direct Controller -> Cleaner/Motor path.
        motorDriver->stopMoving();
        cleanerDriver->stopCleaning();

        currentMode = nullptr; // mode delete
    }

    void Controller::startButtonPressed()
    {
        if (currentMode == nullptr || isNowCharging)
        {
            return;
        }

        currentMode = &currentMode->startButtonPressed(*cleanerDriver, *motorDriver);
    }

    void Controller::chargeBattery()
    {
        // SD-10 CHARGE BATTERY
        if (!canStartCharging())
        {
            return;
        }

        isNowCharging = batteryDriver->startCharging();

        if (!isNowCharging)
        {
            return;
        }

        // One charging loop iteration.
        chargingTick();
    }

    void Controller::chargingTick()
    {
        if (!isNowCharging)
        {
            return;
        }

        if (!batteryDriver->inclineLV())
        {
            isNowCharging = false;
            return;
        }

        if (batteryDriver->isFull())
        {
            isNowCharging = false;
        }

        if (power &&
            currentMode != nullptr &&
            currentMode->kind() == ModeKind::LowBattery &&
            batteryDriver->level() > BatteryDriver::LowBatteryThreshold)
        {
            lowBatteryCleared();
        }
    }

    void Controller::lowBatteryDetected()
    {
        // SD-15 Enter Low Battery Mode
        if (currentMode == nullptr)
        {
            return;
        }

        // SD-15: currentMode = lowBatteryDetected(motor, cleaner)
        currentMode = &currentMode->lowBatteryDetected(*cleanerDriver, *motorDriver);
    }

    void Controller::lowBatteryCleared()
    {
        if (currentMode == nullptr)
        {
            return;
        }

        // SD-10: currentMode = lowBatteryCleared()
        currentMode = &currentMode->lowBatteryCleared();
    }

    void Controller::stopCharging()
    {
        // SD-16 Stop Charging
        batteryDriver->stopCharging();
        isNowCharging = false;
    }

    void Controller::dustDetected()
    {
        if (currentMode == nullptr || isNowCharging)
        {
            return;
        }

        currentMode = &dustProcessor->decideIsDusted(*cleanerDriver, *currentMode);

        if (currentMode->kind() == ModeKind::Boost)
        {
            startTimer();
        }
    }

    void Controller::obstacleDetected(const bool direction[3])
    {
        if (currentMode == nullptr || direction == nullptr)
        {
            return;
        }

        std::array<bool, 3> dir = {
            direction[0],
            direction[1],
            direction[2]};

        obstacleProcessor->decideDirection(dir, *currentMode, *motorDriver);
    }

    //////////////////////////////////////////////////////////////
    // tests
    //////////////////////////////////////////////////////////////

    bool Controller::isPowerOn() const
    {
        return power;
    }

    bool Controller::isCharging() const
    {
        return isNowCharging;
    }

    int Controller::batteryLevel() const
    {
        return batteryDriver->level();
    }

    bool Controller::hasCurrentMode() const
    {
        return currentMode != nullptr;
    }

    ModeKind Controller::currentModeKind() const
    {
        return currentMode == nullptr ? ModeKind::Standby : currentMode->kind();
    }

    std::string Controller::currentModeName() const
    {
        return currentMode == nullptr ? std::string("Off") : std::string(currentMode->name());
    }

    void Controller::setBatteryLevel(int level)
    {
        batteryDriver->setLevel(level);
    }

    bool Controller::isDustSensorActive() const
    {
        return dustSensorDriver->isActive();
    }

    bool Controller::isObstacleSensorActive() const
    {
        return obstacleSensorDriver->isActive();
    }

    bool Controller::isCleanerCleaning() const
    {
        return cleanerDriver->isCleaning();
    }

    std::string Controller::cleanerMode() const
    {
        return cleanerDriver->currentMode();
    }

    bool Controller::isMotorMoving() const
    {
        return motorDriver->isMoving();
    }

    bool Controller::isMotorForward() const
    {
        return motorDriver->checkIsForward();
    }

    Direction Controller::motorDirection() const
    {
        return motorDriver->currentDirection();
    }

    void Controller::timerExpiredNow()
    {
        if (power && currentMode != nullptr)
        {
            currentMode = &currentMode->timerExpired(*cleanerDriver);
        }
    }

} // namespace rvc