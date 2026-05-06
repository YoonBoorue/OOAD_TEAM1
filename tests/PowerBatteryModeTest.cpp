#include <array>
#include <gtest/gtest.h>

#include "rvc/BatteryDriver.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/Controller.hpp"
#include "rvc/Direction.hpp"
#include "rvc/Modes.hpp"
#include "rvc/MotorDriver.hpp"
#include "rvc/ObstacleProcessor.hpp"

using namespace rvc;

// =========================
// BatteryDriver Unit Tests
// =========================

TEST(BatteryDriverTest, InitialBatteryIsFullAndNotChargeable)
{
    BatteryDriver battery;

    EXPECT_EQ(battery.level(), BatteryDriver::MaxLevel);
    EXPECT_TRUE(battery.isFull());
    EXPECT_FALSE(battery.canCharge());
    EXPECT_FALSE(battery.isCharging());
    EXPECT_FALSE(battery.isLow());
}

TEST(BatteryDriverTest, BatteryBecomesChargeableWhenLevelIsBelowMax)
{
    BatteryDriver battery;

    battery.setLevel(50);

    EXPECT_EQ(battery.level(), 50);
    EXPECT_FALSE(battery.isFull());
    EXPECT_TRUE(battery.canCharge());
    EXPECT_FALSE(battery.isCharging());
}

TEST(BatteryDriverTest, StartChargingOnlyWorksWhenBatteryCanCharge)
{
    BatteryDriver battery;

    battery.setLevel(50);
    battery.startCharging();

    EXPECT_TRUE(battery.canCharge());
    EXPECT_TRUE(battery.isCharging());
}

TEST(BatteryDriverTest, StartChargingDoesNotWorkWhenBatteryIsFull)
{
    BatteryDriver battery;

    battery.setLevel(BatteryDriver::MaxLevel);
    battery.startCharging();

    EXPECT_TRUE(battery.isFull());
    EXPECT_FALSE(battery.canCharge());
    EXPECT_FALSE(battery.isCharging());
}

TEST(BatteryDriverTest, InclineLevelWhileCharging)
{
    BatteryDriver battery;

    battery.setLevel(50);
    battery.startCharging();
    battery.inclineLV();

    EXPECT_EQ(battery.level(), 60);
    EXPECT_TRUE(battery.isCharging());
}

TEST(BatteryDriverTest, ChargingStopsAutomaticallyWhenBatteryBecomesFull)
{
    BatteryDriver battery;

    battery.setLevel(90);
    battery.startCharging();
    battery.inclineLV();

    EXPECT_EQ(battery.level(), BatteryDriver::MaxLevel);
    EXPECT_TRUE(battery.isFull());
    EXPECT_FALSE(battery.canCharge());
    EXPECT_FALSE(battery.isCharging());
}

TEST(BatteryDriverTest, StopChargingStopsChargingState)
{
    BatteryDriver battery;

    battery.setLevel(50);
    battery.startCharging();
    ASSERT_TRUE(battery.isCharging());

    battery.stopCharging();

    EXPECT_FALSE(battery.isCharging());
    EXPECT_TRUE(battery.canCharge());
}

TEST(BatteryDriverTest, DeclineLevelDoesNotWorkWhileCharging)
{
    BatteryDriver battery;

    battery.setLevel(50);
    battery.startCharging();
    battery.declineLV();

    EXPECT_EQ(battery.level(), 50);
}

TEST(BatteryDriverTest, LowBatteryThresholdIsTenOrLess)
{
    BatteryDriver battery;

    battery.setLevel(10);
    EXPECT_TRUE(battery.isLow());

    battery.setLevel(11);
    EXPECT_FALSE(battery.isLow());
}

// =========================
// Mode Unit Tests
// =========================

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

// =========================
// Controller Power/Battery UC Tests
// =========================

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

// =========================
// ObstacleProcessor + Mode Tests
// =========================

TEST(ObstacleProcessorTest, NormalModeKeepsMovingForwardWhenFrontIsClear)
{
    ObstacleProcessor processor;
    MotorDriver motor;

    motor.initialize();
    motor.moveForward();

    std::array<bool, 3> dir = {
        false, // front is clear
        false, // left is clear
        false  // right is clear
    };

    processor.decideDirection(dir, normalMode(), motor);

    EXPECT_TRUE(motor.isMoving());
    EXPECT_TRUE(motor.checkIsForward());
    EXPECT_EQ(motor.currentDirection(), Direction::FRONT);
}

TEST(ObstacleProcessorTest, NormalModeTurnsLeftWhenFrontBlockedAndLeftClear)
{
    ObstacleProcessor processor;
    MotorDriver motor;

    motor.initialize();
    motor.moveForward();

    std::array<bool, 3> dir = {
        true,  // front blocked
        false, // left clear
        true   // right blocked
    };

    processor.decideDirection(dir, normalMode(), motor);

    EXPECT_TRUE(motor.isMoving());
    EXPECT_TRUE(motor.checkIsForward());
    EXPECT_EQ(motor.currentDirection(), Direction::LEFT);
}

TEST(ObstacleProcessorTest, NormalModeTurnsRightWhenFrontAndLeftBlockedAndRightClear)
{
    ObstacleProcessor processor;
    MotorDriver motor;

    motor.initialize();
    motor.moveForward();

    std::array<bool, 3> dir = {
        true, // front blocked
        true, // left blocked
        false // right clear
    };

    processor.decideDirection(dir, normalMode(), motor);

    EXPECT_TRUE(motor.isMoving());
    EXPECT_TRUE(motor.checkIsForward());
    EXPECT_EQ(motor.currentDirection(), Direction::RIGHT);
}

TEST(ObstacleProcessorTest, NormalModeMovesBackwardWhenAllDirectionsBlocked)
{
    ObstacleProcessor processor;
    MotorDriver motor;

    motor.initialize();
    motor.moveForward();

    std::array<bool, 3> dir = {
        true, // front blocked
        true, // left blocked
        true  // right blocked
    };

    processor.decideDirection(dir, normalMode(), motor);

    EXPECT_TRUE(motor.isMoving());
    EXPECT_FALSE(motor.checkIsForward());
}

TEST(ObstacleProcessorTest, BoostModeAlsoAvoidsObstacle)
{
    ObstacleProcessor processor;
    MotorDriver motor;

    motor.initialize();
    motor.moveForward();

    std::array<bool, 3> dir = {
        true,  // front blocked
        false, // left clear
        true   // right blocked
    };

    processor.decideDirection(dir, boostMode(), motor);

    EXPECT_TRUE(motor.isMoving());
    EXPECT_TRUE(motor.checkIsForward());
    EXPECT_EQ(motor.currentDirection(), Direction::LEFT);
}

TEST(ObstacleProcessorTest, LowBatteryModeDoesNotMove)
{
    ObstacleProcessor processor;
    CleanerDriver cleaner;
    MotorDriver motor;

    cleaner.initialize();
    motor.initialize();

    normalMode().apply(cleaner, motor);
    ASSERT_TRUE(motor.isMoving());

    lowBatteryMode().apply(cleaner, motor);
    ASSERT_FALSE(motor.isMoving());

    std::array<bool, 3> dir = {
        true,
        false,
        true};

    processor.decideDirection(dir, lowBatteryMode(), motor);

    EXPECT_FALSE(motor.isMoving());
    EXPECT_FALSE(motor.checkIsForward());
}