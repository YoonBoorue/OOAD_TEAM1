#include <gtest/gtest.h>

#include "rvc/BatteryDriver.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/Controller.hpp"
#include "rvc/Modes.hpp"
#include "rvc/MotorDriver.hpp"

using namespace rvc;

// =====================================================
// SD-15 Enter Low Battery Mode
// SD-16 Stop Charging
// Combined System Operation Tests
// =====================================================

// 1
TEST(ControllerLowBatteryStopChargingTest, LowBatteryDetectedDoesNothingWhenPowerIsOff)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.lowBatteryDetected();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeName(), "Off");
}

// 2
TEST(ControllerLowBatteryStopChargingTest, LowBatteryDetectedChangesStandbyModeToLowBatteryMode)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.lowBatteryDetected();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_EQ(controller.currentModeName(), "LowBatteryMode");
}

// 3
TEST(ControllerLowBatteryStopChargingTest, LowBatteryDetectedChangesNormalModeToLowBatteryMode)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);

    controller.lowBatteryDetected();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_EQ(controller.currentModeName(), "LowBatteryMode");
}

// 4
TEST(ControllerLowBatteryStopChargingTest, LowBatteryDetectedKeepsLowBatteryModeWhenAlreadyLowBattery)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.lowBatteryDetected();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
}

// 5
TEST(ControllerLowBatteryStopChargingTest, LowBatteryModeRejectsStartButtonAfterLowBatteryDetected)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.startButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
}

// 6
TEST(ControllerLowBatteryStopChargingTest, LowBatteryDetectedFromNormalModeCanBeClearedByCharging)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);

    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 15);
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 7
TEST(ControllerLowBatteryStopChargingTest, LowBatteryDetectedFromStandbyModeCanBeClearedByCharging)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 15);
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 8
TEST(ControllerLowBatteryStopChargingTest, LowBatteryDetectedSetsModeEvenWhenBatteryIsExactlyThreshold)
{
    Controller controller;

    controller.setBatteryLevel(BatteryDriver::LowBatteryThreshold);
    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.lowBatteryDetected();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
}

// 9
TEST(ControllerLowBatteryStopChargingTest, ModeLowBatteryDetectedStopsMotorAndCleaner)
{
    CleanerDriver cleaner;
    MotorDriver motor;

    cleaner.initialize();
    motor.initialize();

    normalMode().apply(cleaner, motor);

    ASSERT_TRUE(cleaner.isCleaning());
    ASSERT_TRUE(motor.isMoving());

    const OperatingMode &nextMode = normalMode().lowBatteryDetected(cleaner, motor);
    nextMode.apply(cleaner, motor);

    EXPECT_EQ(nextMode.kind(), ModeKind::LowBattery);
    EXPECT_FALSE(cleaner.isCleaning());
    EXPECT_EQ(cleaner.currentMode(), "off");
    EXPECT_FALSE(motor.isMoving());
    EXPECT_FALSE(motor.checkIsForward());
}

// 10
TEST(ControllerLowBatteryStopChargingTest, LowBatteryClearedChangesLowBatteryModeToStandbyMode)
{
    const OperatingMode &nextMode = lowBatteryMode().lowBatteryCleared();

    EXPECT_EQ(nextMode.kind(), ModeKind::Standby);
}

// 11
TEST(ControllerLowBatteryStopChargingTest, StopChargingStopsChargingWhenPowerIsOff)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.chargeBattery();

    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();

    EXPECT_FALSE(controller.isCharging());
    EXPECT_FALSE(controller.isPowerOn());
}

// 12
TEST(ControllerLowBatteryStopChargingTest, StopChargingStopsChargingInStandbyMode)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.chargeBattery();

    ASSERT_TRUE(controller.isCharging());
    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.stopCharging();

    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 13
TEST(ControllerLowBatteryStopChargingTest, StopChargingStopsChargingInLowBatteryMode)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.chargeBattery();

    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();

    EXPECT_FALSE(controller.isCharging());
}

// 14
TEST(ControllerLowBatteryStopChargingTest, StopChargingDoesNothingWhenBatteryIsNotCharging)
{
    Controller controller;

    controller.setBatteryLevel(50);

    ASSERT_FALSE(controller.isCharging());

    controller.stopCharging();

    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.batteryLevel(), 50);
}

// 15
TEST(ControllerLowBatteryStopChargingTest, StopChargingDoesNotChangeBatteryLevel)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.chargeBattery();

    ASSERT_EQ(controller.batteryLevel(), 60);
    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();

    EXPECT_EQ(controller.batteryLevel(), 60);
    EXPECT_FALSE(controller.isCharging());
}

// 16
TEST(ControllerLowBatteryStopChargingTest, ChargingTickDoesNothingAfterStopCharging)
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

// 17
TEST(ControllerLowBatteryStopChargingTest, StopChargingDoesNotTurnOffSystem)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.chargeBattery();

    ASSERT_TRUE(controller.isPowerOn());
    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();

    EXPECT_TRUE(controller.isPowerOn());
    EXPECT_FALSE(controller.isCharging());
}

// 18
TEST(ControllerLowBatteryStopChargingTest, StopChargingDoesNotRemoveCurrentMode)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.powerButtonPressed();
    controller.chargeBattery();

    ASSERT_TRUE(controller.hasCurrentMode());
    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.stopCharging();

    EXPECT_TRUE(controller.hasCurrentMode());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
}

// 19
TEST(ControllerLowBatteryStopChargingTest, StopChargingAfterFullBatteryKeepsChargingStopped)
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

// 20
TEST(ControllerLowBatteryStopChargingTest, CanChargeAgainAfterStopChargingWhenBatteryIsNotFull)
{
    Controller controller;

    controller.setBatteryLevel(50);
    controller.chargeBattery();

    ASSERT_EQ(controller.batteryLevel(), 60);
    ASSERT_TRUE(controller.isCharging());

    controller.stopCharging();

    ASSERT_FALSE(controller.isCharging());

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 70);
    EXPECT_TRUE(controller.isCharging());
}