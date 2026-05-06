#include <gtest/gtest.h>

#include "rvc/BatteryDriver.hpp"
#include "rvc/Controller.hpp"
#include "rvc/Modes.hpp"

using namespace rvc;

TEST(ControllerPowerBatteryTest, PowerButtonTurnsOnSystem)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_TRUE(controller.isPowerOn());
    EXPECT_TRUE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_EQ(controller.currentModeName(), "StandbyMode");
}

TEST(ControllerPowerBatteryTest, PowerButtonTurnsOffSystem)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_TRUE(controller.isPowerOn());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeName(), "Off");
}

TEST(ControllerPowerBatteryTest, StartButtonChangesStandbyToNormal)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.startButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_EQ(controller.currentModeName(), "NormalMode");
}

TEST(ControllerPowerBatteryTest, StartButtonDoesNothingWhenPowerIsOff)
{
    Controller controller;

    controller.startButtonPressed();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeName(), "Off");
}

TEST(ControllerPowerBatteryTest, ChargeBatteryIsAllowedWhenPowerIsOff)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.chargeBattery();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_EQ(controller.batteryLevel(), 60);
    EXPECT_TRUE(controller.isCharging());
}

TEST(ControllerPowerBatteryTest, ChargeBatteryIsAllowedInStandbyMode)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 60);
    EXPECT_TRUE(controller.isCharging());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

TEST(ControllerPowerBatteryTest, ChargeBatteryIsRejectedInNormalMode)
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

TEST(ControllerPowerBatteryTest, StopChargingStopsCharging)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.chargeBattery();

    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();

    EXPECT_FALSE(controller.isCharging());
}

TEST(ControllerPowerBatteryTest, ChargingStopsAutomaticallyWhenBatteryBecomesFull)
{
    Controller controller;

    controller.setBatteryLevel(90);
    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    EXPECT_FALSE(controller.isCharging());
}

TEST(ControllerPowerBatteryTest, LowBatteryDetectedChangesCurrentModeToLowBattery)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);

    controller.setBatteryLevel(5);
    controller.lowBatteryDetected();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_EQ(controller.currentModeName(), "LowBatteryMode");
}

TEST(ControllerPowerBatteryTest, ChargingLowBatteryClearsLowBatteryMode)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 15);
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_EQ(controller.currentModeName(), "StandbyMode");
}

TEST(ControllerPowerBatteryTest, StartButtonDoesNothingWhileCharging)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.chargeBattery();

    ASSERT_TRUE(controller.isCharging());
    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.startButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}