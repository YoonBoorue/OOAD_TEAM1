#include "rvc/BatteryDriver.hpp"

#include <algorithm>

namespace rvc
{

    BatteryDriver::BatteryDriver()
        : LV(MaxLevel), status(true), charging(false), powered(false) {}

    void BatteryDriver::initialize()
    {
        powered = true;
        status = (LV >= MaxLevel);
    }

    void BatteryDriver::turnOffBattery()
    {
        powered = false;
        charging = false;
    }

    void BatteryDriver::declineLV()
    {
        if (LV > MinLevel)
        {
            LV = std::max(MinLevel, LV - DischargeStep);
        }
        status = (LV >= MaxLevel);
    }

    void BatteryDriver::inclineLV()
    {
        if (!charging)
        {
            return;
        }

        if (LV < MaxLevel)
        {
            LV = std::min(MaxLevel, LV + ChargeStep);
        }

        status = (LV >= MaxLevel);
        if (status)
        {
            charging = false;
        }
    }

    void BatteryDriver::startCharging()
    {
        charging = (LV < MaxLevel);
        status = (LV >= MaxLevel);
    }

    void BatteryDriver::stopCharging()
    {
        charging = false;
        status = (LV >= MaxLevel);
    }

    int BatteryDriver::level() const
    {
        return LV;
    }

    bool BatteryDriver::isFull() const
    {
        return status;
    }

    bool BatteryDriver::isLow() const
    {
        return LV <= LowBatteryThreshold;
    }

    bool BatteryDriver::isCharging() const
    {
        return charging;
    }

    bool BatteryDriver::isPowered() const
    {
        return powered;
    }

    void BatteryDriver::setLevel(int level)
    {
        LV = std::clamp(level, MinLevel, MaxLevel);
        status = (LV >= MaxLevel);
        if (status)
        {
            charging = false;
        }
    }

}