#ifndef VIBE_RVC_BATTERY_DRIVER_HPP
#define VIBE_RVC_BATTERY_DRIVER_HPP

namespace rvc
{

class BatteryDriver
{
public:
    static constexpr int LowBatteryThreshold = 10;
    static constexpr int FullBatteryLevel = 100;

    bool isCharging;
    bool isLowBattery;
    int level;

    BatteryDriver() = default;

    void initialize();
    void charge();
    void startCharging();
    void stopCharging();
    void turnOffBattery();
    void setLowBattery(bool lowBattery);
    bool inclineLV();
    bool inclineLevel();
    bool isFull() const;
};

} // namespace rvc

#endif // VIBE_RVC_BATTERY_DRIVER_HPP
