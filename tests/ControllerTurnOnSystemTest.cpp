#include <gtest/gtest.h>

#include "rvc/BatteryDriver.hpp"
#include "rvc/Controller.hpp"
#include "rvc/Modes.hpp"

using namespace rvc;

// =====================================================
// Controller::powerButtonPressed()
// SD-01 Turn On System System Operation Tests
// =====================================================

// 1
TEST(ControllerTurnOnSystemTest, InitialControllerIsPoweredOff)
{
    Controller controller;

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeName(), "Off");
}

// 2
TEST(ControllerTurnOnSystemTest, PowerButtonTurnsSystemOn)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_TRUE(controller.isPowerOn());
}

// 3
TEST(ControllerTurnOnSystemTest, TurnOnCreatesCurrentMode)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_TRUE(controller.hasCurrentMode());
}

// 4
TEST(ControllerTurnOnSystemTest, TurnOnSetsCurrentModeToStandby)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 5
TEST(ControllerTurnOnSystemTest, TurnOnReportsStandbyModeName)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_EQ(controller.currentModeName(), "StandbyMode");
}

// 6
TEST(ControllerTurnOnSystemTest, TurnOnInitializesBatteryWithoutStartingChargingWhenFull)
{
    Controller controller;

    controller.setBatteryLevel(BatteryDriver::MaxLevel);

    controller.powerButtonPressed();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    EXPECT_FALSE(controller.isCharging());
}

// 7
TEST(ControllerTurnOnSystemTest, TurnOnKeepsHalfBatteryLevel)
{
    Controller controller;

    controller.setBatteryLevel(50);

    controller.powerButtonPressed();

    EXPECT_EQ(controller.batteryLevel(), 50);
    EXPECT_TRUE(controller.isPowerOn());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 8
TEST(ControllerTurnOnSystemTest, TurnOnKeepsLowBatteryLevel)
{
    Controller controller;

    controller.setBatteryLevel(5);

    controller.powerButtonPressed();

    EXPECT_EQ(controller.batteryLevel(), 5);
    EXPECT_TRUE(controller.isPowerOn());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 9
TEST(ControllerTurnOnSystemTest, TurnOnKeepsThresholdBatteryLevel)
{
    Controller controller;

    controller.setBatteryLevel(BatteryDriver::LowBatteryThreshold);

    controller.powerButtonPressed();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::LowBatteryThreshold);
    EXPECT_TRUE(controller.isPowerOn());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 10
TEST(ControllerTurnOnSystemTest, TurnOnKeepsEmptyBatteryLevel)
{
    Controller controller;

    controller.setBatteryLevel(BatteryDriver::MinLevel);

    controller.powerButtonPressed();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MinLevel);
    EXPECT_TRUE(controller.isPowerOn());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 11
TEST(ControllerTurnOnSystemTest, TurnOnActivatesDustSensor)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_TRUE(controller.isDustSensorActive());
}

// 12
TEST(ControllerTurnOnSystemTest, TurnOnActivatesObstacleSensor)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_TRUE(controller.isObstacleSensorActive());
}

// 13
TEST(ControllerTurnOnSystemTest, TurnOnInitializesCleanerToOff)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_EQ(controller.cleanerMode(), "off");
}

// 14
TEST(ControllerTurnOnSystemTest, TurnOnDoesNotStartCleaner)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isCleanerCleaning());
}

// 15
TEST(ControllerTurnOnSystemTest, TurnOnInitializesMotorStopped)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isMotorMoving());
}

// 16
TEST(ControllerTurnOnSystemTest, TurnOnDoesNotMoveForward)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isMotorForward());
}

// 17
TEST(ControllerTurnOnSystemTest, TurnOnFromPowerOffThenStartButtonCanEnterNormalMode)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_TRUE(controller.isPowerOn());
    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.startButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
}

// 18
TEST(ControllerTurnOnSystemTest, TurnOnDoesNotActivateCleanerBeforeStartButton)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    EXPECT_FALSE(controller.isCleanerCleaning());
    EXPECT_EQ(controller.cleanerMode(), "off");
}

// 19
TEST(ControllerTurnOnSystemTest, TurnOnDoesNotActivateMotorBeforeStartButton)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    EXPECT_FALSE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
}

// 20
TEST(ControllerTurnOnSystemTest, SystemCanBeTurnedOnAgainAfterTurnOff)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_TRUE(controller.isPowerOn());

    controller.powerButtonPressed();

    ASSERT_FALSE(controller.isPowerOn());
    ASSERT_FALSE(controller.hasCurrentMode());

    controller.powerButtonPressed();

    EXPECT_TRUE(controller.isPowerOn());
    EXPECT_TRUE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_TRUE(controller.isDustSensorActive());
    EXPECT_TRUE(controller.isObstacleSensorActive());
    EXPECT_FALSE(controller.isCleanerCleaning());
    EXPECT_FALSE(controller.isMotorMoving());
}