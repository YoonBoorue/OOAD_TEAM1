#include "rvc/Controller.hpp"

#include "rvc/BatteryDriver.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/DustSensorDriver.hpp"
#include "rvc/Modes.hpp"
#include "rvc/MotorDriver.hpp"
#include "rvc/ObstacleSensorDriver.hpp"
#include "rvc/ObstacleProcessor.hpp"

namespace rvc
{

    Controller::Controller()
        : power(false),
          lowBatteryClearedEventSent(true),
          currentMode(nullptr),
          batteryDriver(new BatteryDriver()),
          cleanerDriver(new CleanerDriver()),
          dustSensorDriver(new DustSensorDriver()),
          motorDriver(new MotorDriver()),
          obstacleSensorDriver(new ObstacleSensorDriver()),
          obstacleProcessor(new ObstacleProcessor()) {}

    Controller::~Controller()
    {
        delete batteryDriver;
        delete cleanerDriver;
        delete dustSensorDriver;
        delete motorDriver;
        delete obstacleSensorDriver;
        delete obstacleProcessor;

        // currentMode is not deleted because it points to a static mode singleton.
    }

    void Controller::enterMode(OperatingMode &nextMode)
    {
        currentMode = &nextMode;

        // Cleaner/Motor commands are delegated to Mode.
        // Controller touches Cleaner/Motor directly only in power on/off sequences.
        if (power)
            currentMode->apply(*cleanerDriver, *motorDriver);
    }

    bool Controller::canStartCharging() const
    {
        if (!power)
        {
            return true;
        }

        return currentMode != nullptr && currentMode->canCharge();
    }

    void Controller::performChargingStep()
    {
        batteryDriver->inclineLV();

        if (power && currentMode != nullptr &&
            batteryDriver->level() > BatteryDriver::LowBatteryThreshold &&
            !lowBatteryClearedEventSent)
        {
            lowBatteryCleared();
        }
    }

    bool Controller::startTimer()
    {
        // UC6 Adjust Boost Mode is intentionally left for the separate boost branch.
        return false;
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

            lowBatteryClearedEventSent = !batteryDriver->isLow();
            return;
        }

        // SD-11 TURN OFF SYSTEM
        power = false;

        batteryDriver->turnOffBattery();
        obstacleSensorDriver->deactivateObstacleSensor();
        dustSensorDriver->deactivateDustSensor();

        // Power off is the other allowed direct Controller -> Cleaner/Motor path.
        motorDriver->stopMoving();
        cleanerDriver->stopCleaning();

        currentMode = nullptr;
        lowBatteryClearedEventSent = true;
    }

    void Controller::startButtonPressed()
    {
        if (!power || currentMode == nullptr || batteryDriver->isCharging())
        {
            return;
        }

        enterMode(currentMode->startButtonPressed());
    }

    void Controller::chargeBattery()
    {
        // SD-10 CHARGE BATTERY
        if (!canStartCharging())
        {
            return;
        }

        if (!batteryDriver->startCharging())
        {
            return;
        }

        // One charging loop iteration. Repeated chargingTick() calls represent SD-10 loop.
        chargingTick();
    }

    void Controller::chargingTick()
    {
        if (!batteryDriver->isCharging())
        {
            return;
        }

        performChargingStep();
    }

    void Controller::lowBatteryDetected()
    {
        // SD-15 Enter Low Battery Mode
        if (!power || currentMode == nullptr)
        {
            return;
        }

        lowBatteryClearedEventSent = false;

        // SD-15: currentMode = lowBatteryDetected(motor, cleaner)
        currentMode = &currentMode->lowBatteryDetected(*cleanerDriver, *motorDriver);
    }

    void Controller::lowBatteryCleared()
    {
        if (!power || currentMode == nullptr)
        {
            lowBatteryClearedEventSent = true;
            return;
        }

        lowBatteryClearedEventSent = true;

        // SD-10: currentMode = lowBatteryCleared()
        currentMode = &currentMode->lowBatteryCleared();
    }

    void Controller::stopCharging()
    {
        // SD-16 Stop Charging
        batteryDriver->stopCharging();
    }

    void Controller::dustDetected()
    {
        // UC6 Adjust Boost Mode is handled in another branch.
    }

    void Controller::obstacleDetected(const bool direction[3])
    {
        if (!power || currentMode == nullptr || direction == nullptr)
        {
            return;
        }

        std::array<bool, 3> dir = {
            direction[0],
            direction[1],
            direction[2]};

        obstacleProcessor->decideDirection(dir, *currentMode, *motorDriver);
    }

    bool Controller::isPowerOn() const
    {
        return power;
    }

    bool Controller::isCharging() const
    {
        return batteryDriver->isCharging();
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
        lowBatteryClearedEventSent = !batteryDriver->isLow();
    }

    // tests

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

} // namespace rvc