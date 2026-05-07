#include <gtest/gtest.h>

#include "rvc/BatteryDriver.hpp"
#include "rvc/Controller.hpp"
#include "rvc/Modes.hpp"

using namespace rvc;

// =====================================================
// Controller::chargeBattery()
// SD-10 Charge Battery System Operation Tests
// =====================================================

// 1
TEST(ControllerChargeBatteryTest, PowerOffAllowsChargingWhenBatteryIsBelowMax)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.chargeBattery();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_EQ(controller.batteryLevel(), 60);
    EXPECT_TRUE(controller.isCharging());
}

// 2
TEST(ControllerChargeBatteryTest, PowerOffRejectsChargingWhenBatteryIsFull)
{
    Controller controller;

    controller.setBatteryLevel(BatteryDriver::MaxLevel);
    controller.chargeBattery();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    EXPECT_FALSE(controller.isCharging());
}

// 3
TEST(ControllerChargeBatteryTest, PowerOffChargingFromNinetyStopsAtFull)
{
    Controller controller;

    controller.setBatteryLevel(90);
    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    EXPECT_FALSE(controller.isCharging());
}

// 4
TEST(ControllerChargeBatteryTest, PowerOffChargingFromZeroIncreasesBattery)
{
    Controller controller;

    controller.setBatteryLevel(0);
    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 10);
    EXPECT_TRUE(controller.isCharging());
}

// 5
TEST(ControllerChargeBatteryTest, PowerOffChargingFromLowBatteryLevelDoesNotCreateMode)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 15);
    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_TRUE(controller.isCharging());
}

// 6
TEST(ControllerChargeBatteryTest, StandbyModeAllowsChargingWhenBatteryIsBelowMax)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();

    ASSERT_TRUE(controller.isPowerOn());
    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 60);
    EXPECT_TRUE(controller.isCharging());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 7
TEST(ControllerChargeBatteryTest, StandbyModeRejectsChargingWhenBatteryIsFull)
{
    Controller controller;

    controller.setBatteryLevel(BatteryDriver::MaxLevel);
    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 8
TEST(ControllerChargeBatteryTest, StandbyModeChargingFromNinetyStopsAtFull)
{
    Controller controller;

    controller.setBatteryLevel(90);
    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 9
TEST(ControllerChargeBatteryTest, NormalModeRejectsCharging)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 50);
    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
}

// 10
TEST(ControllerChargeBatteryTest, NormalModeRejectsChargingEvenWhenBatteryIsLow)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 5);
    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
}

// 11
TEST(ControllerChargeBatteryTest, LowBatteryModeAllowsCharging)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 15);
    EXPECT_TRUE(controller.isCharging());
}

// 12
TEST(ControllerChargeBatteryTest, LowBatteryModeChargingClearsLowBatteryWhenLevelExceedsThreshold)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 15);
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 13
TEST(ControllerChargeBatteryTest, LowBatteryModeAtThresholdClearsAfterOneChargeStep)
{
    Controller controller;

    controller.setBatteryLevel(BatteryDriver::LowBatteryThreshold);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 20);
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 14
TEST(ControllerChargeBatteryTest, LowBatteryModeWithFullBatteryDoesNotStartCharging)
{
    Controller controller;

    controller.setBatteryLevel(BatteryDriver::MaxLevel);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
}

// 15
TEST(ControllerChargeBatteryTest, ChargingTickContinuesChargingAfterChargeBattery)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.chargeBattery();

    ASSERT_EQ(controller.batteryLevel(), 60);
    ASSERT_TRUE(controller.isCharging());

    controller.chargingTick();

    EXPECT_EQ(controller.batteryLevel(), 70);
    EXPECT_TRUE(controller.isCharging());
}

// 16
TEST(ControllerChargeBatteryTest, RepeatedChargingTicksStopAtFull)
{
    Controller controller;

    controller.setBatteryLevel(70);
    controller.chargeBattery();

    ASSERT_EQ(controller.batteryLevel(), 80);
    ASSERT_TRUE(controller.isCharging());

    controller.chargingTick();
    controller.chargingTick();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    EXPECT_FALSE(controller.isCharging());
}

// 17
TEST(ControllerChargeBatteryTest, ChargingTickDoesNothingWhenNotCharging)
{
    Controller controller;

    controller.setBatteryLevel(50);

    ASSERT_FALSE(controller.isCharging());

    controller.chargingTick();

    EXPECT_EQ(controller.batteryLevel(), 50);
    EXPECT_FALSE(controller.isCharging());
}

// 18
TEST(ControllerChargeBatteryTest, StopChargingPreventsFurtherChargingTickIncrease)
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

// 19
TEST(ControllerChargeBatteryTest, StartButtonIsIgnoredWhileChargingInStandbyMode)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.chargeBattery();

    ASSERT_TRUE(controller.isCharging());

    controller.startButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 20
TEST(ControllerChargeBatteryTest, SecondChargeBatteryCallWhileAlreadyChargingAdvancesOneMoreStep)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.chargeBattery();

    ASSERT_EQ(controller.batteryLevel(), 60);
    ASSERT_TRUE(controller.isCharging());

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 70);
    EXPECT_TRUE(controller.isCharging());
}