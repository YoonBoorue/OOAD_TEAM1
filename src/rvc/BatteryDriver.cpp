#include "rvc/BatteryDriver.hpp"

#include <algorithm>

namespace rvc
{

    BatteryDriver::BatteryDriver()
        : LV(MaxLevel), status(false), charging(false) {}

    void BatteryDriver::updateStatus()
    {
        status = (LV < MaxLevel);

        if (!status)
        {
            charging = false;
        }
    }

    void BatteryDriver::initialize() { updateStatus(); }

    void BatteryDriver::turnOffBattery()
    {
        charging = false;
        updateStatus();
    }

    void BatteryDriver::declineLV()
    {
        // If charging is in progress, do not discharge in this simple simulation.
        if (charging)
        {
            return;
        }

        LV = std::max(MinLevel, LV - DischargeStep);
        updateStatus();
    }

    void BatteryDriver::inclineLV()
    {
        if (!charging)
        {
            return;
        }

        LV = std::min(MaxLevel, LV + ChargeStep);
        updateStatus();
    }

    void BatteryDriver::startCharging()
    {
        updateStatus();
        if (!status)
        {
            charging = false;
            return;
        }
        charging = true;
    }

    void BatteryDriver::stopCharging()
    {
        charging = false;
        updateStatus();
    }

    bool BatteryDriver::canCharge() const { return status; }

    bool BatteryDriver::isLow() const { return LV <= LowBatteryThreshold; }

    bool BatteryDriver::isFull() const { return LV >= MaxLevel; }

    bool BatteryDriver::isCharging() const { return charging; }

    int BatteryDriver::level() const { return LV; }

    void BatteryDriver::setLevel(int level)
    {
        LV = std::clamp(level, MinLevel, MaxLevel);
        updateStatus();
    }

} // namespace rvc