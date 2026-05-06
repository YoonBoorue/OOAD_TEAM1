#include "rvc/BatteryDriver.hpp"

#include <algorithm>

namespace rvc
{

    BatteryDriver::BatteryDriver()
        : LV(MaxLevel), status(true), charging(false) {}

    void BatteryDriver::updateStatus()
    {
        // SD-10 기준:
        // status == false → 충전 가능
        // status == true  → 완충 상태, 충전 불가
        status = (LV >= MaxLevel);

        if (status)
        {
            charging = false;
        }
    }

    void BatteryDriver::initialize()
    {
        updateStatus();
    }

    void BatteryDriver::turnOffBattery()
    {
        charging = false;
        updateStatus();
    }

    void BatteryDriver::declineLV()
    {
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

        // status == true means full, so charging cannot start.
        if (status)
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

    bool BatteryDriver::canCharge() const { return !status; }

    bool BatteryDriver::isLow() const { return LV <= LowBatteryThreshold; }

    bool BatteryDriver::isFull() const { return status; }

    bool BatteryDriver::isCharging() const { return charging; }

    int BatteryDriver::level() const { return LV; }

    void BatteryDriver::setLevel(int level)
    {
        LV = std::clamp(level, MinLevel, MaxLevel);
        updateStatus();
    }

} // namespace rvc