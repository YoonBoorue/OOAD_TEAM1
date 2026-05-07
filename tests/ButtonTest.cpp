#include <gtest/gtest.h>

#include "rvc/BatteryDriver.hpp"
#include "rvc/Controller.hpp"
#include "rvc/Modes.hpp"

using namespace rvc;

// =====================================================
// Controller::powerButtonPressed() / startButtonPressed()
// Button Behavior Tests
// =====================================================

// 1
TEST(ButtonTest, InitialStateIsPoweredOff)
{
    Controller controller;

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeName(), "Off");
}

// 2
TEST(ButtonTest, PowerButtonTurnsSystemOnFromOff)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_TRUE(controller.isPowerOn());
    EXPECT_TRUE(controller.hasCurrentMode());
}

// 3
TEST(ButtonTest, PowerButtonSetsInitialModeToStandby)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_EQ(controller.currentModeName(), "StandbyMode");
}

// 4
TEST(ButtonTest, PowerButtonActivatesBothSensors)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_TRUE(controller.isDustSensorActive());
    EXPECT_TRUE(controller.isObstacleSensorActive());
}

// 5
TEST(ButtonTest, PowerButtonDoesNotStartMotorOnTurnOn)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
}

// 6
TEST(ButtonTest, PowerButtonDoesNotStartCleanerOnTurnOn)
{
    Controller controller;

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isCleanerCleaning());
    EXPECT_EQ(controller.cleanerMode(), "off");
}

// 7
TEST(ButtonTest, PowerButtonTurnsSystemOffWhenOn)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_TRUE(controller.isPowerOn());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeName(), "Off");
}

// 8
TEST(ButtonTest, PowerButtonOffDeactivatesBothSensors)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_TRUE(controller.isDustSensorActive());
    ASSERT_TRUE(controller.isObstacleSensorActive());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isDustSensorActive());
    EXPECT_FALSE(controller.isObstacleSensorActive());
}

// 9
TEST(ButtonTest, PowerButtonOffStopsMotor)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
}

// 10
TEST(ButtonTest, PowerButtonOffStopsCleaner)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isCleanerCleaning());
    EXPECT_EQ(controller.cleanerMode(), "off");
}

// 11
TEST(ButtonTest, PowerButtonPreservesBatteryLevelOnCycle)
{
    Controller controller;

    controller.setBatteryLevel(75);
    controller.powerButtonPressed();

    ASSERT_EQ(controller.batteryLevel(), 75);

    controller.powerButtonPressed();

    EXPECT_EQ(controller.batteryLevel(), 75);
}

// 12
TEST(ButtonTest, PowerButtonTogglesOnOffOn)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_TRUE(controller.isPowerOn());

    controller.powerButtonPressed();

    ASSERT_FALSE(controller.isPowerOn());
    ASSERT_FALSE(controller.hasCurrentMode());

    controller.powerButtonPressed();

    EXPECT_TRUE(controller.isPowerOn());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_TRUE(controller.isDustSensorActive());
    EXPECT_TRUE(controller.isObstacleSensorActive());
    EXPECT_FALSE(controller.isCleanerCleaning());
    EXPECT_FALSE(controller.isMotorMoving());
}

// 13
TEST(ButtonTest, StartButtonWhenPowerOffDoesNothing)
{
    Controller controller;

    ASSERT_FALSE(controller.isPowerOn());

    controller.startButtonPressed();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeName(), "Off");
}

// 14
TEST(ButtonTest, StartButtonInStandbyTransitionsToNormal)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.startButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_EQ(controller.currentModeName(), "NormalMode");
}

// 15
TEST(ButtonTest, StartButtonInStandbyStartsMotor)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.startButtonPressed();

    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_TRUE(controller.isMotorForward());
}

// 16
TEST(ButtonTest, StartButtonInStandbyStartsCleanerInNormalMode)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.startButtonPressed();

    EXPECT_TRUE(controller.isCleanerCleaning());
    EXPECT_EQ(controller.cleanerMode(), "normal");
}

// 17
TEST(ButtonTest, StartButtonInNormalReturnsToStandby)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);

    controller.startButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_EQ(controller.currentModeName(), "StandbyMode");
}

// 18
TEST(ButtonTest, StartButtonInNormalStopsMotor)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    ASSERT_TRUE(controller.isMotorMoving());

    controller.startButtonPressed();

    EXPECT_FALSE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
}

// 19
TEST(ButtonTest, StartButtonInNormalStopsCleaner)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    ASSERT_TRUE(controller.isCleanerCleaning());

    controller.startButtonPressed();

    EXPECT_FALSE(controller.isCleanerCleaning());
    EXPECT_EQ(controller.cleanerMode(), "off");
}

// 20
TEST(ButtonTest, StartButtonInLowBatteryModeIsIgnored)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.startButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_FALSE(controller.isCleanerCleaning());
    EXPECT_FALSE(controller.isMotorMoving());
}

// 21
TEST(ButtonTest, StartButtonIgnoredWhileCharging)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.chargeBattery();

    ASSERT_TRUE(controller.isCharging());

    controller.startButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_FALSE(controller.isCleanerCleaning());
}

// 22
TEST(ButtonTest, StartButtonAfterPowerCycleRestoresNormalOperation)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);

    controller.powerButtonPressed();

    ASSERT_FALSE(controller.isPowerOn());

    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.startButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_TRUE(controller.isCleanerCleaning());
}

// 23
TEST(ButtonTest, DoubleStartButtonTogglesBackToStandby)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);

    controller.startButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_FALSE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isCleanerCleaning());
}

// 24
TEST(ButtonTest, PowerButtonOffFromLowBatteryStopsEverything)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_FALSE(controller.isDustSensorActive());
    EXPECT_FALSE(controller.isObstacleSensorActive());
    EXPECT_FALSE(controller.isCleanerCleaning());
    EXPECT_FALSE(controller.isMotorMoving());
}

// 25
TEST(ButtonTest, StartButtonDoesNotActivateSensorsOnOff)
{
    Controller controller;

    ASSERT_FALSE(controller.isPowerOn());

    controller.startButtonPressed();

    EXPECT_FALSE(controller.isDustSensorActive());
    EXPECT_FALSE(controller.isObstacleSensorActive());
}

// 26
TEST(ButtonTest, PowerButtonOffStopsChargingFromStandby)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.chargeBattery();

    ASSERT_TRUE(controller.isCharging());

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isCharging());
    EXPECT_FALSE(controller.isPowerOn());
}

// 27
TEST(ButtonTest, StartButtonMultiplePressesToggleCleanerState)
{
    Controller controller;

    controller.powerButtonPressed();

    ASSERT_FALSE(controller.isCleanerCleaning());

    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_TRUE(controller.isCleanerCleaning());

    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_FALSE(controller.isCleanerCleaning());
}

// 28
TEST(ButtonTest, PowerButtonOffWithLowBatteryKeepsBatteryLevel)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    ASSERT_EQ(controller.batteryLevel(), 5);

    controller.powerButtonPressed();

    EXPECT_EQ(controller.batteryLevel(), 5);
    EXPECT_FALSE(controller.isPowerOn());
}
