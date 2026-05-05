#include "rvc/Controller.hpp"

#include "rvc/BatteryDriver.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/DustSensorDriver.hpp"
<<<<<<< feat/power-usecases(1-10-11-16)
#include "rvc/Modes.hpp"
#include "rvc/MotorDriver.hpp"
#include "rvc/ObstacleSensorDriver.hpp"
=======
#include "rvc/DustProcessor.hpp"
#include "rvc/MotorDriver.hpp"
#include "rvc/ObstacleProcessor.hpp"
>>>>>>> main

namespace rvc
{

    Controller::Controller()
<<<<<<< feat/power-usecases(1-10-11-16)
        : power(false), isNowCharging(false), lowBatteryClearedEventSent(true), currentMode(nullptr), batteryDriver(new BatteryDriver()), cleanerDriver(new CleanerDriver()), dustSensorDriver(new DustSensorDriver()), motorDriver(new MotorDriver()), obstacleSensorDriver(new ObstacleSensorDriver()) {}
=======
        : power(false), isNowCharging(false), currentMode(nullptr), batteryDriver(new BatteryDriver()), cleanerDriver(new CleanerDriver()), motorDriver(new MotorDriver()), dustSensorDriver(new DustSensorDriver()), dustProcessor(new DustProcessor()), obstacleProcessor(new ObstacleProcessor())
    {
    }
>>>>>>> main

    Controller::~Controller()
    {
        delete batteryDriver;
        delete cleanerDriver;
        delete dustSensorDriver;
<<<<<<< feat/power-usecases(1-10-11-16)
        delete motorDriver;
        delete obstacleSensorDriver;

        // currentMode is not deleted because it points to a static mode singleton.
    }

    void Controller::enterMode(OperatingMode &nextMode)
    {
        currentMode = &nextMode;

        // Cleaner/Motor commands are delegated to Mode.
        // Controller touches Cleaner/Motor directly only in power on/off sequences.
        if (power && currentMode != nullptr)
        {
            currentMode->apply(*cleanerDriver, *motorDriver);
        }
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
        isNowCharging = batteryDriver->isCharging();

        if (power && currentMode != nullptr &&
            batteryDriver->level() > BatteryDriver::LowBatteryThreshold &&
            !lowBatteryClearedEventSent)
        {
            lowBatteryCleared();
        }
=======
        delete dustProcessor;
        delete currentMode;
>>>>>>> main
    }

    bool Controller::startTimer()
    {
<<<<<<< feat/power-usecases(1-10-11-16)
        // UC6 Adjust Boost Mode is intentionally left for the separate boost branch.
=======
>>>>>>> main
        return false;
    }

    void Controller::powerButtonPressed()
    {
<<<<<<< feat/power-usecases(1-10-11-16)
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

            isNowCharging = batteryDriver->isCharging();
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
        isNowCharging = batteryDriver->isCharging();
=======
        // TODO
>>>>>>> main
    }

    void Controller::startButtonPressed()
    {
<<<<<<< feat/power-usecases(1-10-11-16)
        if (!power || currentMode == nullptr || isNowCharging)
        {
            return;
        }

        enterMode(currentMode->startButtonPressed());
=======
        // TODO
>>>>>>> main
    }

    void Controller::chargeBattery()
    {
<<<<<<< feat/power-usecases(1-10-11-16)
        // SD-10 CHARGE BATTERY
        if (!canStartCharging())
        {
            return;
        }

        batteryDriver->startCharging();
        isNowCharging = batteryDriver->isCharging();

        // One charging loop iteration. Repeated chargingTick() calls represent SD-10 loop.
        chargingTick();
    }

    void Controller::chargingTick()
    {
        if (!batteryDriver->isCharging())
        {
            isNowCharging = false;
            return;
        }

        performChargingStep();
=======
        // TODO
>>>>>>> main
    }

    void Controller::lowBatteryDetected()
    {
<<<<<<< feat/power-usecases(1-10-11-16)
        // SD-15 ENTER LOW BATTERY MODE
        if (!power || currentMode == nullptr)
        {
            return;
        }

        lowBatteryClearedEventSent = false;
        enterMode(currentMode->lowBatteryDetected());
=======
        // TODO
>>>>>>> main
    }

    void Controller::lowBatteryCleared()
    {
<<<<<<< feat/power-usecases(1-10-11-16)
        if (!power || currentMode == nullptr)
        {
            lowBatteryClearedEventSent = true;
            return;
        }

        lowBatteryClearedEventSent = true;
        enterMode(currentMode->lowBatteryCleared());
=======
        // TODO
>>>>>>> main
    }

    void Controller::stopCharging()
    {
<<<<<<< feat/power-usecases(1-10-11-16)
        // SD-16 STOP CHARGING
        batteryDriver->stopCharging();
        isNowCharging = false;
=======
        // TODO
>>>>>>> main
    }

    void Controller::dustDetected()
    {
<<<<<<< feat/power-usecases(1-10-11-16)
        // UC6 Adjust Boost Mode is intentionally left for the separate boost branch.
    }

    void Controller::obstacleDetected(const bool direction[3])
    {
        // Obstacle-related UCs are intentionally left outside this branch.
        (void)direction;
    }

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

    bool Controller::isBatteryPowered() const
    {
        return batteryDriver->isPowered();
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

} // namespace rvc
=======
        // TODO
    }

    void Controller::obstacleDetected(const std::array<bool, 3> &dir)
    {
        bool fw = this->motorDriver->checkIsForward();
        obstacleProcessor->decideDirection(dir, *currentMode, *motorDriver);
    }

}
>>>>>>> main
