#include <gtest/gtest.h>

#include "rvc/BatteryDriver.hpp"
#include "rvc/Controller.hpp"
#include "rvc/Modes.hpp"

using namespace rvc;

// =====================================================
// Controller::lowBatteryDetected()
// SD-15 Enter Low Battery Mode System Operation Tests
// =====================================================

// 1
TEST(ControllerEnterLowBatteryModeTest, PowerOffIgnoresLowBatteryDetected)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.lowBatteryDetected();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ(controller.batteryLevel(), 5);
    EXPECT_FALSE(controller.isCharging());
}

// 2
TEST(ControllerEnterLowBatteryModeTest, StandbyModeEntersLowBatteryMode)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();

    ASSERT_TRUE(controller.isPowerOn());
    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.lowBatteryDetected();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_EQ(controller.batteryLevel(), 5);
    EXPECT_FALSE(controller.isCharging());
}

// 3
TEST(ControllerEnterLowBatteryModeTest, NormalModeEntersLowBatteryMode)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);

    controller.lowBatteryDetected();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_EQ(controller.batteryLevel(), 5);
    EXPECT_FALSE(controller.isCharging());
}

// 4
TEST(ControllerEnterLowBatteryModeTest, BoostModeEntersLowBatteryMode)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.startButtonPressed();
    controller.dustDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Boost);

    controller.lowBatteryDetected();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_EQ(controller.batteryLevel(), 5);
    EXPECT_FALSE(controller.isCharging());
}

// 5
TEST(ControllerEnterLowBatteryModeTest, RepeatedLowBatteryDetectedKeepsLowBatteryMode)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.lowBatteryDetected();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_EQ(controller.batteryLevel(), 5);
    EXPECT_FALSE(controller.isCharging());
}

// 6
TEST(ControllerEnterLowBatteryModeTest, LowBatteryDetectedDoesNotChangeBatteryLevel)
{
    Controller controller;

    controller.setBatteryLevel(BatteryDriver::LowBatteryThreshold);
    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);

    controller.lowBatteryDetected();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::LowBatteryThreshold);
    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
}

// 7
TEST(ControllerEnterLowBatteryModeTest, LowBatteryDetectedDoesNotStartCharging)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.startButtonPressed();

    ASSERT_FALSE(controller.isCharging());

    controller.lowBatteryDetected();

    EXPECT_FALSE(controller.isCharging());
    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
}

// 8
TEST(ControllerEnterLowBatteryModeTest, LowBatteryModeAllowsChargingAfterEnteringFromStandby)
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

// 9
TEST(ControllerEnterLowBatteryModeTest, LowBatteryModeAllowsChargingAfterEnteringFromNormal)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.startButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 15);
    EXPECT_TRUE(controller.isCharging());
}

// 10
TEST(ControllerEnterLowBatteryModeTest, StartButtonIsIgnoredInLowBatteryMode)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.startButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.startButtonPressed();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_FALSE(controller.isCharging());
}

// 11
TEST(ControllerEnterLowBatteryModeTest, DustDetectedIsIgnoredInLowBatteryMode)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.dustDetected();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_FALSE(controller.isCharging());
}

// 12
TEST(ControllerEnterLowBatteryModeTest, TimerExpiredKeepsLowBatteryMode)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_FALSE(controller.isCharging());
}

// 13
TEST(ControllerEnterLowBatteryModeTest, ChargeBatteryClearsLowBatteryModeWhenLevelExceedsThreshold)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), 15);
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_TRUE(controller.isCharging());
}

// 14
TEST(ControllerEnterLowBatteryModeTest, LowBatteryModeAtThresholdClearsAfterCharging)
{
    Controller controller;

    controller.setBatteryLevel(BatteryDriver::LowBatteryThreshold);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::LowBatteryThreshold + 10);
    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_TRUE(controller.isCharging());
}

// 15
TEST(ControllerEnterLowBatteryModeTest, FullBatteryEventStillEntersLowBatteryModeWhenDetectedEventArrives)
{
    Controller controller;

    controller.setBatteryLevel(BatteryDriver::MaxLevel);
    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.lowBatteryDetected();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    EXPECT_FALSE(controller.isCharging());
}

// 16
TEST(ControllerEnterLowBatteryModeTest, FullBatteryLowBatteryModeDoesNotStartCharging)
{
    Controller controller;

    controller.setBatteryLevel(BatteryDriver::MaxLevel);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.chargeBattery();

    EXPECT_EQ(controller.batteryLevel(), BatteryDriver::MaxLevel);
    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_FALSE(controller.isCharging());
}

// 17
TEST(ControllerEnterLowBatteryModeTest, PowerButtonTurnsOffFromLowBatteryMode)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_TRUE(controller.isPowerOn());
    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_FALSE(controller.isCharging());
}

// 18
TEST(ControllerEnterLowBatteryModeTest, LowBatteryDetectedAfterClearedCanEnterAgain)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.lowBatteryDetected();
    controller.chargeBattery();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);

    controller.lowBatteryDetected();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_EQ(controller.batteryLevel(), 15);
}

// 19
TEST(ControllerEnterLowBatteryModeTest, LowBatteryDetectedFromBoostHasPriorityOverBoostTimer)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.startButtonPressed();
    controller.dustDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Boost);

    controller.lowBatteryDetected();

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_FALSE(controller.isCharging());
}

// 20
TEST(ControllerEnterLowBatteryModeTest, LowBatteryDetectedDoesNotPreventManualPowerOff)
{
    Controller controller;

    controller.setBatteryLevel(5);
    controller.powerButtonPressed();
    controller.startButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);

    controller.powerButtonPressed();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
}
