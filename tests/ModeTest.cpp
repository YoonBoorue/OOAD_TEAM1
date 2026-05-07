#include <gtest/gtest.h>

#include "rvc/CleanerDriver.hpp"
#include "rvc/Direction.hpp"
#include "rvc/Modes.hpp"
#include "rvc/MotorDriver.hpp"

using namespace rvc;

TEST(ModeTest, StandbyModeStartButtonChangesToNormalMode)
{
    OperatingMode &nextMode = standbyMode().startButtonPressed();

    EXPECT_EQ(nextMode.kind(), ModeKind::Normal);
}

TEST(ModeTest, NormalModeStartButtonChangesToStandbyMode)
{
    OperatingMode &nextMode = normalMode().startButtonPressed();

    EXPECT_EQ(nextMode.kind(), ModeKind::Standby);
}

TEST(ModeTest, LowBatteryModeIgnoresStartButton)
{
    OperatingMode &nextMode = lowBatteryMode().startButtonPressed();

    EXPECT_EQ(nextMode.kind(), ModeKind::LowBattery);
}

TEST(ModeTest, StandbyModeAllowsCharging)
{
    EXPECT_TRUE(standbyMode().canCharge());
}

TEST(ModeTest, NormalModeRejectsCharging)
{
    EXPECT_FALSE(normalMode().canCharge());
}

TEST(ModeTest, BoostModeRejectsCharging)
{
    EXPECT_FALSE(boostMode().canCharge());
}

TEST(ModeTest, LowBatteryModeAllowsCharging)
{
    EXPECT_TRUE(lowBatteryMode().canCharge());
}

TEST(ModeTest, NormalModeApplyStartsMotorAndCleaner)
{
    CleanerDriver cleaner;
    MotorDriver motor;

    cleaner.initialize();
    motor.initialize();

    normalMode().apply(cleaner, motor);

    EXPECT_TRUE(cleaner.isCleaning());
    EXPECT_EQ(cleaner.currentMode(), "normal");
    EXPECT_TRUE(motor.isMoving());
    EXPECT_TRUE(motor.checkIsForward());
    EXPECT_EQ(motor.currentDirection(), Direction::FRONT);
}

TEST(ModeTest, StandbyModeApplyStopsMotorAndCleaner)
{
    CleanerDriver cleaner;
    MotorDriver motor;

    cleaner.initialize();
    motor.initialize();

    normalMode().apply(cleaner, motor);
    ASSERT_TRUE(cleaner.isCleaning());
    ASSERT_TRUE(motor.isMoving());

    standbyMode().apply(cleaner, motor);

    EXPECT_FALSE(cleaner.isCleaning());
    EXPECT_EQ(cleaner.currentMode(), "off");
    EXPECT_FALSE(motor.isMoving());
    EXPECT_FALSE(motor.checkIsForward());
}

TEST(ModeTest, LowBatteryDetectedChangesToLowBatteryModeAndStopsMotorCleaner)
{
    CleanerDriver cleaner;
    MotorDriver motor;

    cleaner.initialize();
    motor.initialize();

    normalMode().apply(cleaner, motor);
    ASSERT_TRUE(cleaner.isCleaning());
    ASSERT_TRUE(motor.isMoving());

    OperatingMode &nextMode = normalMode().lowBatteryDetected(cleaner, motor);

    EXPECT_EQ(nextMode.kind(), ModeKind::LowBattery);
    EXPECT_FALSE(cleaner.isCleaning());
    EXPECT_EQ(cleaner.currentMode(), "off");
    EXPECT_FALSE(motor.isMoving());
    EXPECT_FALSE(motor.checkIsForward());
}

TEST(ModeTest, LowBatteryClearedChangesToStandbyMode)
{
    OperatingMode &nextMode = lowBatteryMode().lowBatteryCleared();

    EXPECT_EQ(nextMode.kind(), ModeKind::Standby);
}