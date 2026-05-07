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

        // true  = battery is full and cannot be charged
        // false = battery can be charged
        bool status;

        // true  = battery is currently charging
        // false = battery is not currently charging
        bool charging;

        void updateStatus();

    public:
        BatteryDriver();

        void initialize();
        void turnOffBattery();

        bool startCharging();
        void stopCharging();
        bool inclineLV();
        void declineLV();

        /// TEST
        bool isLow() const;
        bool isFull() const;
        bool isCharging() const;
        int level() const;

        // Test / simulator helper
        void setLevel(int level);
    };

} // namespace rvc