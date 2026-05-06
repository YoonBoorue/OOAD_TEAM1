#include <gtest/gtest.h>

#include "rvc/BatteryDriver.hpp"

using namespace rvc;

TEST(BatteryDriverTest, InitialBatteryIsFullAndNotChargeable)
{
    BatteryDriver battery;

    EXPECT_EQ(battery.level(), BatteryDriver::MaxLevel);
    EXPECT_TRUE(battery.isFull());
    EXPECT_FALSE(battery.canCharge());
    EXPECT_FALSE(battery.isCharging());
    EXPECT_FALSE(battery.isLow());
}

TEST(BatteryDriverTest, BatteryBecomesChargeableWhenLevelIsBelowMax)
{
    BatteryDriver battery;

    battery.setLevel(50);

    EXPECT_EQ(battery.level(), 50);
    EXPECT_FALSE(battery.isFull());
    EXPECT_TRUE(battery.canCharge());
    EXPECT_FALSE(battery.isCharging());
}

TEST(BatteryDriverTest, StartChargingOnlyWorksWhenBatteryCanCharge)
{
    BatteryDriver battery;

    battery.setLevel(50);
    battery.startCharging();

    EXPECT_TRUE(battery.canCharge());
    EXPECT_TRUE(battery.isCharging());
}

TEST(BatteryDriverTest, StartChargingDoesNotWorkWhenBatteryIsFull)
{
    BatteryDriver battery;

    battery.setLevel(BatteryDriver::MaxLevel);
    battery.startCharging();

    EXPECT_TRUE(battery.isFull());
    EXPECT_FALSE(battery.canCharge());
    EXPECT_FALSE(battery.isCharging());
}

TEST(BatteryDriverTest, InclineLevelWhileCharging)
{
    BatteryDriver battery;

    battery.setLevel(50);
    battery.startCharging();
    battery.inclineLV();

    EXPECT_EQ(battery.level(), 60);
    EXPECT_TRUE(battery.isCharging());
}

TEST(BatteryDriverTest, ChargingStopsAutomaticallyWhenBatteryBecomesFull)
{
    BatteryDriver battery;

    battery.setLevel(90);
    battery.startCharging();
    battery.inclineLV();

    EXPECT_EQ(battery.level(), BatteryDriver::MaxLevel);
    EXPECT_TRUE(battery.isFull());
    EXPECT_FALSE(battery.canCharge());
    EXPECT_FALSE(battery.isCharging());
}

TEST(BatteryDriverTest, StopChargingStopsChargingState)
{
    BatteryDriver battery;

    battery.setLevel(50);
    battery.startCharging();

    ASSERT_TRUE(battery.isCharging());

    battery.stopCharging();

    EXPECT_FALSE(battery.isCharging());
    EXPECT_TRUE(battery.canCharge());
}

TEST(BatteryDriverTest, DeclineLevelDoesNotWorkWhileCharging)
{
    BatteryDriver battery;

    battery.setLevel(50);
    battery.startCharging();
    battery.declineLV();

    EXPECT_EQ(battery.level(), 50);
}

TEST(BatteryDriverTest, LowBatteryThresholdIsTenOrLess)
{
    BatteryDriver battery;

    battery.setLevel(10);
    EXPECT_TRUE(battery.isLow());

    battery.setLevel(11);
    EXPECT_FALSE(battery.isLow());
}