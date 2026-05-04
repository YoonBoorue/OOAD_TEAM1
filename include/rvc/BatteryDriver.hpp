#pragma once

namespace rvc
{

    class BatteryDriver
    {
    private:
        int LV;
        bool status;
        bool charging;
        bool powered;

    public:
        static constexpr int MaxLevel = 100;
        static constexpr int MinLevel = 0;
        static constexpr int LowBatteryThreshold = 10;
        static constexpr int ChargeStep = 10;
        static constexpr int DischargeStep = 1;

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

        void setLevel(int level);
    };

}