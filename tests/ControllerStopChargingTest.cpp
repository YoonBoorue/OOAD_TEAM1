#include <gtest/gtest.h>

#include "rvc/BatteryDriver.hpp"
#include "rvc/Controller.hpp"
#include "rvc/Modes.hpp"

using namespace rvc;

// =====================================================
// Controller::stopCharging()
// SD-16 Stop Charging System Operation Tests
// =====================================================

// 1
TEST(ControllerStopChargingTest, PowerOffNotChargingStopChargingKeepsNotCharging)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.stopCharging();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ(controller.batteryLevel(), 50);
    EXPECT_FALSE(controller.isCharging());
}

// 2
TEST(ControllerStopChargingTest, PowerOffChargingStopChargingStopsCharging)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.chargeBattery();

    ASSERT_EQ(controller.batteryLevel(), 60);
    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();

    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.batteryLevel(), 60);
    EXPECT_FALSE(controller.isPowerOn());
}

// 3
TEST(ControllerStopChargingTest, StopChargingDoesNotChangeBatteryLevel)
{
    Controller controller;

    controller.setBatteryLevel(30);
    controller.chargeBattery();

    ASSERT_EQ(controller.batteryLevel(), 40);
    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();

    EXPECT_EQ(controller.batteryLevel(), 40);
    EXPECT_FALSE(controller.isCharging());
}

// 4
TEST(ControllerStopChargingTest, ChargingTickAfterStopChargingDoesNotIncreaseBattery)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.chargeBattery();

    ASSERT_EQ(controller.batteryLevel(), 60);
    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();
    controller.chargingTick();

    EXPECT_EQ(controller.batteryLevel(), 60);
    EXPECT_FALSE(controller.isCharging());
}

// 5
TEST(ControllerStopChargingTest, RepeatedStopChargingIsIdempotent)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.chargeBattery();

    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();
    controller.stopCharging();
    controller.stopCharging();

    EXPECT_EQ(controller.batteryLevel(), 60);
    EXPECT_FALSE(controller.isCharging());
}

// 6
TEST(ControllerStopChargingTest, StopChargingAtFullBatteryKeepsFullBattery)
{
    Controller controller;

    controller.setBatteryLevel(BatteryDriver::MaxLevel);
    controller.stopCharging();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    EXPECT_FALSE(controller.isCharging());
}

// 7
TEST(ControllerStopChargingTest, StopChargingAfterChargingToFullKeepsNotCharging)
{
    Controller controller;

    controller.setBatteryLevel(90);
    controller.chargeBattery();

    ASSERT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    ASSERT_FALSE(controller.isCharging());

    controller.stopCharging();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    EXPECT_FALSE(controller.isCharging());
}

// 8
TEST(ControllerStopChargingTest, StandbyChargingStopChargingStopsCharging)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.chargeBattery();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);
    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();

    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.batteryLevel(), 60);
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 9
TEST(ControllerStopChargingTest, StandbyStopChargingPreservesStandbyMode)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);
    ASSERT_FALSE(controller.isCharging());

    controller.stopCharging();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.batteryLevel(), 50);
}

// 10
TEST(ControllerStopChargingTest, NormalModeStopChargingIsNoOpWhenNotCharging)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    ASSERT_FALSE(controller.isCharging());

    controller.stopCharging();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_EQ(controller.batteryLevel(), 50);
    EXPECT_FALSE(controller.isCharging());
}

// 11
TEST(ControllerStopChargingTest, NormalModeRejectedChargeThenStopChargingKeepsNormalMode)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);

    controller.chargeBattery();
    controller.stopCharging();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_EQ(controller.batteryLevel(), 50);
    EXPECT_FALSE(controller.isCharging());
}

// 12
TEST(ControllerStopChargingTest, BoostModeStopChargingIsNoOpWhenNotCharging)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.startButtonPressed();
    controller.dustDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Boost);
    ASSERT_FALSE(controller.isCharging());

    controller.stopCharging();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Boost);
    EXPECT_EQ(controller.batteryLevel(), 50);
    EXPECT_FALSE(controller.isCharging());
}

// 13
TEST(ControllerStopChargingTest, LowBatteryChargingThenStopChargingStopsCharging)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();
    controller.chargeBattery();

    ASSERT_EQ(controller.batteryLevel(), 15);
    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();

    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.batteryLevel(), 15);
}

// 14
TEST(ControllerStopChargingTest, LowBatteryChargingStopPreventsFurtherTick)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();
    controller.chargeBattery();

    ASSERT_EQ(controller.batteryLevel(), 15);
    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();
    controller.chargingTick();

    EXPECT_EQ(controller.batteryLevel(), 15);
    EXPECT_FALSE(controller.isCharging());
}

// 15
TEST(ControllerStopChargingTest, StopChargingDoesNotPowerOnSystem)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.stopCharging();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_FALSE(controller.isCharging());
}

// 16
TEST(ControllerStopChargingTest, StopChargingDoesNotCreateModeWhenPowerOff)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.chargeBattery();

    ASSERT_TRUE(controller.isCharging());
    ASSERT_FALSE(controller.hasCurrentMode());

    controller.stopCharging();

    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_FALSE(controller.isCharging());
}

// 17
TEST(ControllerStopChargingTest, StopChargingBeforeChargeBatteryDoesNotBlockFutureCharging)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.stopCharging();

    ASSERT_FALSE(controller.isCharging());

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 60);
    EXPECT_TRUE(controller.isCharging());
}

// 18
TEST(ControllerStopChargingTest, StopChargingAfterOneChargingTickFreezesBatteryLevel)
{
    Controller controller;

    controller.setBatteryLevel(40);
    controller.chargeBattery();
    controller.chargingTick();

    ASSERT_EQ(controller.batteryLevel(), 60);
    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();
    controller.chargingTick();

    EXPECT_EQ(controller.batteryLevel(), 60);
    EXPECT_FALSE(controller.isCharging());
}

// 19
TEST(ControllerStopChargingTest, StopChargingAfterMultipleChargingTicksFreezesBatteryLevel)
{
    Controller controller;

    controller.setBatteryLevel(20);
    controller.chargeBattery();
    controller.chargingTick();
    controller.chargingTick();

    ASSERT_EQ(controller.batteryLevel(), 50);
    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();
    controller.chargingTick();
    controller.chargingTick();

    EXPECT_EQ(controller.batteryLevel(), 50);
    EXPECT_FALSE(controller.isCharging());
}

// 20
TEST(ControllerStopChargingTest, StartButtonWorksAfterStoppingChargingInStandbyMode)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.chargeBattery();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);
    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();
    controller.startButtonPressed();

    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
}
