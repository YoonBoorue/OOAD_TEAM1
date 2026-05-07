#include <gtest/gtest.h>

#include "rvc/BatteryDriver.hpp"
#include "rvc/Controller.hpp"
#include "rvc/Modes.hpp"

using namespace rvc;

// =====================================================
// Controller::powerButtonPressed()
// SD-11 Turn Off System System Operation Tests
// =====================================================

// 1
TEST(ControllerTurnOffSystemTest, TurnOffChangesPowerOnToPowerOff)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_TRUE(controller.isPowerOn());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isPowerOn());
}

// 2
TEST(ControllerTurnOffSystemTest, TurnOffRemovesCurrentMode)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_TRUE(controller.hasCurrentMode());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeName(), "Off");
}

// 3
TEST(ControllerTurnOffSystemTest, TurnOffStopsCharging)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.chargeBattery();

    ASSERT_TRUE(controller.isCharging());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isCharging());
}

// 4
TEST(ControllerTurnOffSystemTest, TurnOffDoesNotChangeBatteryLevelExceptPreviousChargingStep)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();

    ASSERT_EQ(controller.batteryLevel(), 50);

    controller.powerButtonPressed();

    EXPECT_EQ(controller.batteryLevel(), 50);
}

// 5
TEST(ControllerTurnOffSystemTest, TurnOffDeactivatesDustSensor)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_TRUE(controller.isDustSensorActive());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isDustSensorActive());
}

// 6
TEST(ControllerTurnOffSystemTest, TurnOffDeactivatesObstacleSensor)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_TRUE(controller.isObstacleSensorActive());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isObstacleSensorActive());
}

// 7
TEST(ControllerTurnOffSystemTest, TurnOffStopsCleanerFromNormalMode)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    ASSERT_TRUE(controller.isCleanerCleaning());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isCleanerCleaning());
    EXPECT_EQ(controller.cleanerMode(), "off");
}

// 8
TEST(ControllerTurnOffSystemTest, TurnOffStopsMotorFromNormalMode)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    ASSERT_TRUE(controller.isMotorMoving());
    ASSERT_TRUE(controller.isMotorForward());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
}

// 9
TEST(ControllerTurnOffSystemTest, TurnOffFromStandbyKeepsCleanerOff)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);
    ASSERT_FALSE(controller.isCleanerCleaning());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isCleanerCleaning());
    EXPECT_EQ(controller.cleanerMode(), "off");
}

// 10
TEST(ControllerTurnOffSystemTest, TurnOffFromStandbyKeepsMotorStopped)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);
    ASSERT_FALSE(controller.isMotorMoving());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
}

// 11
TEST(ControllerTurnOffSystemTest, TurnOffFromLowBatteryModeKeepsCleanerStopped)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    ASSERT_FALSE(controller.isCleanerCleaning());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isCleanerCleaning());
    EXPECT_EQ(controller.cleanerMode(), "off");
}

// 12
TEST(ControllerTurnOffSystemTest, TurnOffFromLowBatteryModeKeepsMotorStopped)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    ASSERT_FALSE(controller.isMotorMoving());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
}

// 13
TEST(ControllerTurnOffSystemTest, TurnOffWhileChargingInStandbyStopsChargingAndKeepsBatteryLevel)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.chargeBattery();

    ASSERT_TRUE(controller.isCharging());
    ASSERT_EQ(controller.batteryLevel(), 60);

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.batteryLevel(), 60);
}

// 14
TEST(ControllerTurnOffSystemTest, TurnOffAfterChargingFullKeepsBatteryFull)
{
    Controller controller;

    controller.setBatteryLevel(90);
    controller.powerButtonPressed();
    controller.chargeBattery();

    ASSERT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    ASSERT_FALSE(controller.isCharging());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    EXPECT_FALSE(controller.isCharging());
}

// 15
TEST(ControllerTurnOffSystemTest, TurnOffThenStartButtonDoesNothing)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.powerButtonPressed();

    ASSERT_FALSE(controller.isPowerOn());

    controller.startButtonPressed();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeName(), "Off");
}

// 16
TEST(ControllerTurnOffSystemTest, TurnOffThenLowBatteryDetectedDoesNothing)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.powerButtonPressed();

    ASSERT_FALSE(controller.isPowerOn());

    controller.lowBatteryDetected();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeName(), "Off");
}

// 17
TEST(ControllerTurnOffSystemTest, TurnOffThenObstacleDetectedDoesNothing)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_TRUE(controller.isMotorMoving());

    controller.powerButtonPressed();

    ASSERT_FALSE(controller.isPowerOn());

    const bool direction[3] = {
        true,
        false,
        true};

    controller.obstacleDetected(direction);

    EXPECT_FALSE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
}

// 18
TEST(ControllerTurnOffSystemTest, TurnOffThenChargingCanStillStartWhenBatteryIsNotFull)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.powerButtonPressed();

    ASSERT_FALSE(controller.isPowerOn());

    controller.chargeBattery();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_EQ(controller.batteryLevel(), 60);
    EXPECT_TRUE(controller.isCharging());
}

// 19
TEST(ControllerTurnOffSystemTest, TurnOffCanBeCalledAfterPowerOnWithoutStart)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_TRUE(controller.isPowerOn());
    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_FALSE(controller.isDustSensorActive());
    EXPECT_FALSE(controller.isObstacleSensorActive());
}

// 20
TEST(ControllerTurnOffSystemTest, PowerButtonAfterTurnOffTurnsSystemOnAgain)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.powerButtonPressed();

    ASSERT_FALSE(controller.isPowerOn());
    ASSERT_FALSE(controller.hasCurrentMode());

    controller.powerButtonPressed();

    EXPECT_TRUE(controller.isPowerOn());
    EXPECT_TRUE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_TRUE(controller.isDustSensorActive());
    EXPECT_TRUE(controller.isObstacleSensorActive());
}