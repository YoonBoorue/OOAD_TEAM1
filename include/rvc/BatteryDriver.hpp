#pragma once

namespace rvc
{

    class BatteryDriver
    {
    public:
        static constexpr int MinLevel = 0;
        static constexpr int MaxLevel = 100;
        static constexpr int LowBatteryThreshold = 10;
        static constexpr int ChargeStep = 10;
        static constexpr int DischargeStep = 1;

    private:
        int LV;
        bool status;   // true when battery is full
        bool charging; // true while charging is in progress
        bool powered;  // true while supplying power to the system

        void updateStatus();

    public:
        BatteryDriver();

        void initialize();
        void turnOffBattery();
        void declineLV();
        void inclineLV();
        void startCharging();
        void stopCharging();

        int level() const;
        bool isFull() const;
        bool isLow() const;
        bool isCharging() const;
        bool isPowered() const;

        // Simulator/test support
        void setLevel(int level);
    };

} // namespace rvc
