#include "rvc/BatteryDriver.hpp"

#include <algorithm>

namespace rvc
{

    BatteryDriver::BatteryDriver()
        : LV(MaxLevel), status(true), charging(false), powered(false) {}

    void BatteryDriver::updateStatus()
    {
        status = (LV >= MaxLevel);
        if (status)
            charging = false;
    }

    void BatteryDriver::initialize()
    {
        powered = (LV > MinLevel);
        updateStatus();
    }

    void BatteryDriver::turnOffBattery() { powered = false; }

    void BatteryDriver::declineLV()
    {
        if (!powered || charging)
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
        charging = (LV < MaxLevel);
        updateStatus();
    }

    void BatteryDriver::stopCharging()
    {
        charging = false;
        updateStatus();
    }

    int BatteryDriver::level() const { return LV; }

    bool BatteryDriver::isFull() const { return status; }

    bool BatteryDriver::isLow() const { return LV <= LowBatteryThreshold; }

    bool BatteryDriver::isCharging() const { return charging; }

    bool BatteryDriver::isPowered() const { return powered; }

    void BatteryDriver::setLevel(int level)
    {
        LV = std::clamp(level, MinLevel, MaxLevel);
        updateStatus();
    }

} // namespace rvc
