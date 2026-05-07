#include <gtest/gtest.h>

#include "rvc/BatteryDriver.hpp"

using namespace rvc;

TEST(BatteryDriverTest, InitialBatteryIsFullAndNotCharging)
{
    BatteryDriver battery;

    EXPECT_EQ(battery.level(), BatteryDriver::MaxLevel);
    EXPECT_TRUE(battery.isFull());
    EXPECT_FALSE(battery.isCharging());
    EXPECT_FALSE(battery.isLow());
}

TEST(BatteryDriverTest, StartChargingSucceedsWhenBatteryIsNotFull)
{
    BatteryDriver battery;

    battery.setLevel(50);

    EXPECT_TRUE(battery.startCharging());
    EXPECT_TRUE(battery.isCharging());
    EXPECT_FALSE(battery.isFull());
}

TEST(BatteryDriverTest, StartChargingFailsWhenBatteryIsFull)
{
    BatteryDriver battery;

    battery.setLevel(BatteryDriver::MaxLevel);

    EXPECT_FALSE(battery.startCharging());
    EXPECT_FALSE(battery.isCharging());
    EXPECT_TRUE(battery.isFull());
}

TEST(BatteryDriverTest, InclineLevelWhileCharging)
{
    BatteryDriver battery;

    battery.setLevel(50);
    ASSERT_TRUE(battery.startCharging());

    EXPECT_TRUE(battery.inclineLV());
    EXPECT_EQ(battery.level(), 60);
    EXPECT_TRUE(battery.isCharging());
}

TEST(BatteryDriverTest, ChargingStopsAutomaticallyWhenBatteryBecomesFull)
{
    BatteryDriver battery;

    battery.setLevel(90);
    ASSERT_TRUE(battery.startCharging());

    battery.inclineLV();

    EXPECT_EQ(battery.level(), BatteryDriver::MaxLevel);
    EXPECT_TRUE(battery.isFull());
    EXPECT_FALSE(battery.isCharging());
}

TEST(BatteryDriverTest, StopChargingStopsChargingState)
{
    BatteryDriver battery;

    battery.setLevel(50);
    ASSERT_TRUE(battery.startCharging());

    battery.stopCharging();

    EXPECT_FALSE(battery.isCharging());
    EXPECT_EQ(battery.level(), 50);
}

TEST(BatteryDriverTest, DeclineLevelDoesNotWorkWhileCharging)
{
    BatteryDriver battery;

    battery.setLevel(50);
    ASSERT_TRUE(battery.startCharging());

    battery.declineLV();

    EXPECT_EQ(battery.level(), 50);
}

TEST(BatteryDriverTest, DeclineLevelWorksWhenNotCharging)
{
    BatteryDriver battery;

    battery.setLevel(50);

    battery.declineLV();

    EXPECT_EQ(battery.level(), 49);
}

TEST(BatteryDriverTest, LowBatteryThresholdIsTenOrLess)
{
    BatteryDriver battery;

    battery.setLevel(10);
    EXPECT_TRUE(battery.isLow());

    battery.setLevel(11);
    EXPECT_FALSE(battery.isLow());
}