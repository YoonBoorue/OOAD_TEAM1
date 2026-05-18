#ifndef VIBE_RVC_BATTERY_DRIVER_HPP
#define VIBE_RVC_BATTERY_DRIVER_HPP

namespace rvc
{

class BatteryDriver
{
public:
    bool isCharging;
    bool isLowBattery;

    BatteryDriver() = default;

    void charge();
    void stopCharging();
    void setLowBattery(bool lowBattery);
};

} // namespace rvc

#endif // VIBE_RVC_BATTERY_DRIVER_HPP
