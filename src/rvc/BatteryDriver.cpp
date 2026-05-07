#include "rvc/BatteryDriver.hpp"

#include <algorithm>

namespace rvc
{

    BatteryDriver::BatteryDriver()
        : LV(MaxLevel), status(true)
    {
    }

    void BatteryDriver::updateStatus()
    {
        // status == true means full / not chargeable.
        status = (LV >= MaxLevel);
    }

    void BatteryDriver::initialize()
    {
        updateStatus();
    }

    void BatteryDriver::turnOffBattery()
    {
        updateStatus();
    }

    void BatteryDriver::declineLV()
    {
        LV = std::max(MinLevel, LV - DischargeStep);
        updateStatus();
    }

    bool BatteryDriver::inclineLV()
    {
        if (status)
        {
            return false;
        }

        const int previousLevel = LV;

        LV = std::min(MaxLevel, LV + ChargeStep);
        updateStatus();

        return LV > previousLevel;
    }

    void BatteryDriver::startCharging()
    {
        updateStatus();
    }

    void BatteryDriver::stopCharging()
    {
        updateStatus();
    }

    bool BatteryDriver::isLow() const
    {
        return LV <= LowBatteryThreshold;
    }

    bool BatteryDriver::isFull() const
    {
        return status;
    }

    int BatteryDriver::level() const
    {
        return LV;
    }

    void BatteryDriver::setLevel(int level)
    {
        LV = std::clamp(level, MinLevel, MaxLevel);
        updateStatus();
    }

} // namespace rvc