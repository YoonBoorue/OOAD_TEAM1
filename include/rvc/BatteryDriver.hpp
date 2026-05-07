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

        void updateStatus();

    public:
        BatteryDriver();

        void initialize();
        void turnOffBattery();

        void startCharging();
        void stopCharging();
        bool inclineLV();
        void declineLV();

        /// TEST
        bool isLow() const;
        bool isFull() const;
        int level() const;

        // Test / simulator helper
        void setLevel(int level);
    };

} // namespace rvc