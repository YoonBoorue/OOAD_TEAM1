#include <gtest/gtest.h>

#include "rvc/BatteryDriver.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/Controller.hpp"
#include "rvc/Direction.hpp"
#include "rvc/DustProcessor.hpp"
#include "rvc/DustSensorDriver.hpp"
#include "rvc/MotorDriver.hpp"
#include "rvc/ObstacleProcessor.hpp"
#include "rvc/ObstacleSensorDriver.hpp"
#include "rvc/OperatingMode.hpp"

#include <memory>
#include <vector>

namespace
{
using namespace rvc;

template <typename Mode>
bool IsMode(const Controller& controller)
{
    return dynamic_cast<Mode*>(controller.currentMode) != nullptr;
}

bool IsOff(const Controller& controller)
{
    return controller.currentMode == nullptr;
}

std::vector<std::unique_ptr<Controller>>& ControllerPool()
{
    static auto* pool = new std::vector<std::unique_ptr<Controller>>();
    return *pool;
}

Controller& NewController()
{
    auto& pool = ControllerPool();
    pool.push_back(std::make_unique<Controller>());
    return *pool.back();
}

void InitializeOff(Controller& controller)
{
    controller.clockTick();
}

Controller& NewInitializedOffController()
{
    Controller& controller = NewController();
    InitializeOff(controller);
    return controller;
}

Controller& NewPoweredOnController()
{
    Controller& controller = NewController();
    controller.powerButtonPressed();
    return controller;
}

Controller& NewNormalController()
{
    Controller& controller = NewPoweredOnController();
    controller.startButtonPressed();
    return controller;
}

Controller& NewBoostController()
{
    Controller& controller = NewNormalController();
    controller.dustSensorDriver.dustDetected = true;
    controller.dustDetected();
    return controller;
}

Controller& NewLowBatteryController()
{
    Controller& controller = NewPoweredOnController();
    controller.lowBatteryDetected();
    return controller;
}

void SendObstacle(Controller& controller, bool front, bool left, bool right)
{
    const bool direction[3] = {front, left, right};
    controller.obstacleDetected(direction);
}

std::unique_ptr<OperatingMode> OwnIfNew(OperatingMode* nextMode, OperatingMode* self)
{
    if (nextMode == self)
    {
        return nullptr;
    }

    return std::unique_ptr<OperatingMode>(nextMode);
}

class ControllerFixture : public ::testing::Test
{
protected:
    Controller& offController() { return NewInitializedOffController(); }
    Controller& poweredOnController() { return NewPoweredOnController(); }
    Controller& normalController() { return NewNormalController(); }
    Controller& boostController() { return NewBoostController(); }
    Controller& lowBatteryController() { return NewLowBatteryController(); }
};

class ControllerPowerButtonPressedTest : public ControllerFixture
{
};

TEST_F(ControllerPowerButtonPressedTest, TurnsOnFromOffIntoStandbyWithIdleHardware)
{
    Controller& controller = NewController();

    controller.powerButtonPressed();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Forward);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerPowerButtonPressedTest, PowerOnInitializesDirtyOffState)
{
    Controller& controller = offController();
    controller.batteryDriver.isCharging = true;
    controller.batteryDriver.isLowBattery = true;
    controller.batteryDriver.level = 20;
    controller.dustSensorDriver.dustDetected = true;
    controller.obstacleSensorDriver.front = true;
    controller.motorDriver.start(Direction::Backward);
    controller.cleanerDriver.decideSetting(true);

    controller.powerButtonPressed();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_FALSE(controller.batteryDriver.isCharging);
    EXPECT_FALSE(controller.batteryDriver.isLowBattery);
    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::FullBatteryLevel);
    EXPECT_FALSE(controller.dustSensorDriver.dustDetected);
    EXPECT_FALSE(controller.obstacleSensorDriver.hasObstacle());
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerPowerButtonPressedTest, TurnsOffFromStandbyClearsModeAndSensors)
{
    Controller& controller = poweredOnController();
    controller.dustSensorDriver.dustDetected = true;
    controller.obstacleSensorDriver.front = true;

    controller.powerButtonPressed();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_FALSE(controller.dustSensorDriver.dustDetected);
    EXPECT_FALSE(controller.obstacleSensorDriver.hasObstacle());
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerPowerButtonPressedTest, TurnsOffFromNormalStopsMotorAndCleaner)
{
    Controller& controller = normalController();

    ASSERT_TRUE(IsMode<NormalMode>(controller));
    ASSERT_TRUE(controller.motorDriver.isRunning);
    ASSERT_TRUE(controller.cleanerDriver.isRunning);

    controller.powerButtonPressed();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerPowerButtonPressedTest, TurnsOffFromBoostStopsBoostCleaning)
{
    Controller& controller = boostController();

    ASSERT_TRUE(IsMode<BoostMode>(controller));
    ASSERT_TRUE(controller.cleanerDriver.isRunning);
    ASSERT_TRUE(controller.cleanerDriver.isBoosting);

    controller.powerButtonPressed();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerPowerButtonPressedTest, TurnsOffFromLowBatteryPreservesBatteryWarning)
{
    Controller& controller = lowBatteryController();

    ASSERT_TRUE(IsMode<LowBatteryMode>(controller));
    ASSERT_TRUE(controller.batteryDriver.isLowBattery);

    controller.powerButtonPressed();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_TRUE(controller.batteryDriver.isLowBattery);
    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::LowBatteryThreshold);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerPowerButtonPressedTest, PowerCycleReinitializesToStandby)
{
    Controller& controller = normalController();
    controller.batteryDriver.level = 40;

    controller.powerButtonPressed();
    ASSERT_TRUE(IsOff(controller));

    controller.powerButtonPressed();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::FullBatteryLevel);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Forward);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerPowerButtonPressedTest, PowerOffStopsChargingFromStandby)
{
    Controller& controller = poweredOnController();
    controller.batteryDriver.level = 50;
    controller.chargeBattery();

    ASSERT_TRUE(controller.batteryDriver.isCharging);

    controller.powerButtonPressed();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_FALSE(controller.batteryDriver.isCharging);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

class ControllerStartButtonPressedTest : public ControllerFixture
{
};

TEST_F(ControllerStartButtonPressedTest, OffStateIgnoresStartButton)
{
    Controller& controller = offController();

    controller.startButtonPressed();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerStartButtonPressedTest, StandbyTransitionsToNormalAndStartsCleaning)
{
    Controller& controller = poweredOnController();

    controller.startButtonPressed();

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Forward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerStartButtonPressedTest, NormalTransitionsToStandbyAndStopsHardware)
{
    Controller& controller = normalController();

    controller.startButtonPressed();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerStartButtonPressedTest, BoostTransitionsToStandbyAndStopsBoost)
{
    Controller& controller = boostController();

    controller.startButtonPressed();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerStartButtonPressedTest, LowBatteryModeBlocksCleaningStart)
{
    Controller& controller = lowBatteryController();
    controller.motorDriver.start(Direction::Forward);
    controller.cleanerDriver.startCleaning();

    controller.startButtonPressed();

    EXPECT_TRUE(IsMode<LowBatteryMode>(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.batteryDriver.isLowBattery);
}

TEST_F(ControllerStartButtonPressedTest, SecondPressAfterStartingReturnsToStandby)
{
    Controller& controller = poweredOnController();

    controller.startButtonPressed();
    ASSERT_TRUE(IsMode<NormalMode>(controller));

    controller.startButtonPressed();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerStartButtonPressedTest, StartWhileChargingStillUsesCurrentModePolicy)
{
    Controller& controller = poweredOnController();
    controller.batteryDriver.level = 50;
    controller.chargeBattery();

    ASSERT_TRUE(controller.batteryDriver.isCharging);

    controller.startButtonPressed();

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.batteryDriver.isCharging);
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerStartButtonPressedTest, StartAfterLowBatteryClearedBeginsNormalCleaning)
{
    Controller& controller = lowBatteryController();
    controller.batteryDriver.level = 20;
    controller.lowBatteryCleared();

    ASSERT_TRUE(IsMode<StandbyMode>(controller));

    controller.startButtonPressed();

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.batteryDriver.isLowBattery);
}

class ControllerDustDetectedTest : public ControllerFixture
{
};

TEST_F(ControllerDustDetectedTest, OffStateIgnoresDustSignal)
{
    Controller& controller = offController();
    controller.dustSensorDriver.dustDetected = true;

    controller.dustDetected();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerDustDetectedTest, StandbyIgnoresDustSignal)
{
    Controller& controller = poweredOnController();
    controller.dustSensorDriver.dustDetected = true;

    controller.dustDetected();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerDustDetectedTest, NormalWithoutSensorFlagDoesNothing)
{
    Controller& controller = normalController();

    controller.dustDetected();

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerDustDetectedTest, NormalWithDustEntersBoost)
{
    Controller& controller = normalController();
    controller.dustSensorDriver.dustDetected = true;

    controller.dustDetected();

    EXPECT_TRUE(IsMode<BoostMode>(controller));
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.cleanerDriver.isBoosting);
    EXPECT_TRUE(controller.motorDriver.isRunning);
}

TEST_F(ControllerDustDetectedTest, BoostIgnoresRepeatedDustSignal)
{
    Controller& controller = boostController();

    controller.dustDetected();

    EXPECT_TRUE(IsMode<BoostMode>(controller));
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerDustDetectedTest, ChargingSuppressesBoostTransition)
{
    Controller& controller = poweredOnController();
    controller.batteryDriver.level = 50;
    controller.chargeBattery();
    controller.startButtonPressed();
    controller.dustSensorDriver.dustDetected = true;

    ASSERT_TRUE(controller.batteryDriver.isCharging);
    ASSERT_TRUE(IsMode<NormalMode>(controller));

    controller.dustDetected();

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.batteryDriver.isCharging);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerDustDetectedTest, LowBatteryIgnoresDustSignal)
{
    Controller& controller = lowBatteryController();
    controller.dustSensorDriver.dustDetected = true;

    controller.dustDetected();

    EXPECT_TRUE(IsMode<LowBatteryMode>(controller));
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerDustDetectedTest, DustCanBoostAgainAfterTimerReturnsToNormal)
{
    Controller& controller = boostController();

    controller.timerExpiredNow();
    ASSERT_TRUE(IsMode<NormalMode>(controller));
    ASSERT_FALSE(controller.cleanerDriver.isBoosting);

    controller.dustDetected();

    EXPECT_TRUE(IsMode<BoostMode>(controller));
    EXPECT_TRUE(controller.cleanerDriver.isBoosting);
}

class ControllerTimerExpiredTest : public ControllerFixture
{
};

TEST_F(ControllerTimerExpiredTest, OffStateIgnoresTimer)
{
    Controller& controller = offController();

    controller.timerExpired();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerTimerExpiredTest, StandbyTimerIsNoOp)
{
    Controller& controller = poweredOnController();

    controller.timerExpired();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerTimerExpiredTest, NormalTimerKeepsNormalCleaning)
{
    Controller& controller = normalController();

    controller.timerExpired();

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Forward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerTimerExpiredTest, BoostTimerReturnsToNormalCleaning)
{
    Controller& controller = boostController();

    controller.timerExpired();

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
    EXPECT_TRUE(controller.motorDriver.isRunning);
}

TEST_F(ControllerTimerExpiredTest, LowBatteryTimerIsNoOp)
{
    Controller& controller = lowBatteryController();

    controller.timerExpired();

    EXPECT_TRUE(IsMode<LowBatteryMode>(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.batteryDriver.isLowBattery);
}

TEST_F(ControllerTimerExpiredTest, TimerExpiredNowDelegatesToTimerExpiredBehavior)
{
    Controller& controller = boostController();

    controller.timerExpiredNow();
    ASSERT_TRUE(IsMode<NormalMode>(controller));

    controller.timerExpiredNow();

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

class ControllerLowBatteryDetectedTest : public ControllerFixture
{
};

TEST_F(ControllerLowBatteryDetectedTest, OffStateIgnoresLowBatteryEvent)
{
    Controller& controller = offController();

    controller.lowBatteryDetected();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_FALSE(controller.batteryDriver.isLowBattery);
    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::FullBatteryLevel);
}

TEST_F(ControllerLowBatteryDetectedTest, StandbyEntersLowBatteryMode)
{
    Controller& controller = poweredOnController();

    controller.lowBatteryDetected();

    EXPECT_TRUE(IsMode<LowBatteryMode>(controller));
    EXPECT_TRUE(controller.batteryDriver.isLowBattery);
    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::LowBatteryThreshold);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerLowBatteryDetectedTest, NormalEntersLowBatteryAndStopsCleaning)
{
    Controller& controller = normalController();

    controller.lowBatteryDetected();

    EXPECT_TRUE(IsMode<LowBatteryMode>(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.batteryDriver.isLowBattery);
}

TEST_F(ControllerLowBatteryDetectedTest, BoostEntersLowBatteryAndClearsBoost)
{
    Controller& controller = boostController();

    controller.lowBatteryDetected();

    EXPECT_TRUE(IsMode<LowBatteryMode>(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
    EXPECT_TRUE(controller.batteryDriver.isLowBattery);
}

TEST_F(ControllerLowBatteryDetectedTest, RepeatedLowBatteryEventIsIdempotent)
{
    Controller& controller = lowBatteryController();

    controller.lowBatteryDetected();

    EXPECT_TRUE(IsMode<LowBatteryMode>(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.batteryDriver.isLowBattery);
    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::LowBatteryThreshold);
}

TEST_F(ControllerLowBatteryDetectedTest, LowBatteryDuringChargingKeepsChargingState)
{
    Controller& controller = poweredOnController();
    controller.batteryDriver.level = 50;
    controller.chargeBattery();

    ASSERT_TRUE(controller.batteryDriver.isCharging);

    controller.lowBatteryDetected();

    EXPECT_TRUE(IsMode<LowBatteryMode>(controller));
    EXPECT_TRUE(controller.batteryDriver.isCharging);
    EXPECT_TRUE(controller.batteryDriver.isLowBattery);
    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::LowBatteryThreshold);
}

TEST_F(ControllerLowBatteryDetectedTest, LowBatteryStopsMovementButKeepsLastDirection)
{
    Controller& controller = normalController();
    SendObstacle(controller, true, true, true);

    ASSERT_EQ(controller.motorDriver.direction, Direction::Backward);

    controller.lowBatteryDetected();

    EXPECT_TRUE(IsMode<LowBatteryMode>(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Backward);
}

class ControllerLowBatteryClearedTest : public ControllerFixture
{
};

TEST_F(ControllerLowBatteryClearedTest, OffStateIgnoresClearEvent)
{
    Controller& controller = offController();
    controller.batteryDriver.isLowBattery = true;
    controller.batteryDriver.level = BatteryDriver::LowBatteryThreshold;

    controller.lowBatteryCleared();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_TRUE(controller.batteryDriver.isLowBattery);
    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::LowBatteryThreshold);
}

TEST_F(ControllerLowBatteryClearedTest, StandbyClearOnlyUpdatesBatteryFlag)
{
    Controller& controller = poweredOnController();
    controller.batteryDriver.isLowBattery = true;

    controller.lowBatteryCleared();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_FALSE(controller.batteryDriver.isLowBattery);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerLowBatteryClearedTest, LowBatteryClearReturnsToStandby)
{
    Controller& controller = lowBatteryController();
    controller.batteryDriver.level = 20;

    controller.lowBatteryCleared();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_FALSE(controller.batteryDriver.isLowBattery);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerLowBatteryClearedTest, NormalClearKeepsNormalCleaning)
{
    Controller& controller = normalController();
    controller.batteryDriver.isLowBattery = true;

    controller.lowBatteryCleared();

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_FALSE(controller.batteryDriver.isLowBattery);
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
}

class ControllerChargeBatteryTest : public ControllerFixture
{
};

TEST_F(ControllerChargeBatteryTest, OffFullBatteryDoesNotStartCharging)
{
    Controller& controller = offController();

    controller.chargeBattery();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::FullBatteryLevel);
    EXPECT_FALSE(controller.batteryDriver.isCharging);
}

TEST_F(ControllerChargeBatteryTest, OffBelowFullChargesOneStepAndStaysOff)
{
    Controller& controller = offController();
    controller.batteryDriver.level = 50;

    controller.chargeBattery();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_EQ(controller.batteryDriver.level, 60);
    EXPECT_TRUE(controller.batteryDriver.isCharging);
}

TEST_F(ControllerChargeBatteryTest, OffChargingFromNinetyStopsAtFull)
{
    Controller& controller = offController();
    controller.batteryDriver.level = 90;

    controller.chargeBattery();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::FullBatteryLevel);
    EXPECT_FALSE(controller.batteryDriver.isCharging);
}

TEST_F(ControllerChargeBatteryTest, StandbyBelowFullChargesAndRemainsIdle)
{
    Controller& controller = poweredOnController();
    controller.batteryDriver.level = 50;

    controller.chargeBattery();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_EQ(controller.batteryDriver.level, 60);
    EXPECT_TRUE(controller.batteryDriver.isCharging);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerChargeBatteryTest, StandbyFullBatteryDoesNotCharge)
{
    Controller& controller = poweredOnController();

    controller.chargeBattery();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::FullBatteryLevel);
    EXPECT_FALSE(controller.batteryDriver.isCharging);
}

TEST_F(ControllerChargeBatteryTest, NormalRejectsCharging)
{
    Controller& controller = normalController();
    controller.batteryDriver.level = 50;

    controller.chargeBattery();

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_EQ(controller.batteryDriver.level, 50);
    EXPECT_FALSE(controller.batteryDriver.isCharging);
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerChargeBatteryTest, BoostRejectsCharging)
{
    Controller& controller = boostController();
    controller.batteryDriver.level = 50;

    controller.chargeBattery();

    EXPECT_TRUE(IsMode<BoostMode>(controller));
    EXPECT_EQ(controller.batteryDriver.level, 50);
    EXPECT_FALSE(controller.batteryDriver.isCharging);
    EXPECT_TRUE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerChargeBatteryTest, LowBatteryChargingClearsToStandby)
{
    Controller& controller = lowBatteryController();

    controller.chargeBattery();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_EQ(controller.batteryDriver.level, 20);
    EXPECT_TRUE(controller.batteryDriver.isCharging);
    EXPECT_FALSE(controller.batteryDriver.isLowBattery);
}

TEST_F(ControllerChargeBatteryTest, RepeatedChargingTicksStopAtFull)
{
    Controller& controller = offController();
    controller.batteryDriver.level = 80;

    controller.chargeBattery();
    ASSERT_EQ(controller.batteryDriver.level, 90);
    ASSERT_TRUE(controller.batteryDriver.isCharging);

    controller.chargingTick();

    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::FullBatteryLevel);
    EXPECT_FALSE(controller.batteryDriver.isCharging);
}

class ControllerStopChargingTest : public ControllerFixture
{
};

TEST_F(ControllerStopChargingTest, StopChargingWithoutModeCreatesStandbySafeState)
{
    Controller& controller = offController();

    controller.stopCharging();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_FALSE(controller.batteryDriver.isCharging);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerStopChargingTest, StopChargingFromOffChargingClearsChargingAndCreatesStandby)
{
    Controller& controller = offController();
    controller.batteryDriver.level = 50;
    controller.chargeBattery();

    ASSERT_TRUE(IsOff(controller));
    ASSERT_TRUE(controller.batteryDriver.isCharging);

    controller.stopCharging();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_FALSE(controller.batteryDriver.isCharging);
    EXPECT_EQ(controller.batteryDriver.level, 60);
}

TEST_F(ControllerStopChargingTest, StopChargingInStandbyKeepsStandby)
{
    Controller& controller = poweredOnController();
    controller.batteryDriver.level = 50;
    controller.chargeBattery();

    ASSERT_TRUE(controller.batteryDriver.isCharging);

    controller.stopCharging();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_FALSE(controller.batteryDriver.isCharging);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerStopChargingTest, StopChargingDoesNotChangeActiveCleaningMode)
{
    Controller& controller = poweredOnController();
    controller.batteryDriver.level = 50;
    controller.chargeBattery();
    controller.startButtonPressed();

    ASSERT_TRUE(IsMode<NormalMode>(controller));
    ASSERT_TRUE(controller.batteryDriver.isCharging);

    controller.stopCharging();

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_FALSE(controller.batteryDriver.isCharging);
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
}

class ControllerObstacleDetectedTest : public ControllerFixture
{
};

TEST_F(ControllerObstacleDetectedTest, OffStateIgnoresObstacleArray)
{
    Controller& controller = offController();

    SendObstacle(controller, true, false, false);

    EXPECT_TRUE(IsOff(controller));
    EXPECT_FALSE(controller.obstacleSensorDriver.hasObstacle());
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerObstacleDetectedTest, NullObstacleInputIsNoOp)
{
    Controller& controller = normalController();

    controller.obstacleDetected(nullptr);

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Forward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerObstacleDetectedTest, NormalFrontClearKeepsForwardCleaning)
{
    Controller& controller = normalController();

    SendObstacle(controller, false, true, true);

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Forward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
    EXPECT_TRUE(controller.obstacleSensorDriver.left);
    EXPECT_TRUE(controller.obstacleSensorDriver.right);
}

TEST_F(ControllerObstacleDetectedTest, NormalFrontBlockedLeftClearResumesForwardAfterTurn)
{
    Controller& controller = normalController();

    SendObstacle(controller, true, false, false);

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Forward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.obstacleSensorDriver.front);
}

TEST_F(ControllerObstacleDetectedTest, NormalFrontAndLeftBlockedRightClearResumesForwardAfterTurn)
{
    Controller& controller = normalController();

    SendObstacle(controller, true, true, false);

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Forward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.obstacleSensorDriver.front);
    EXPECT_TRUE(controller.obstacleSensorDriver.left);
}

TEST_F(ControllerObstacleDetectedTest, NormalAllBlockedMovesBackwardAndResumesCleaning)
{
    Controller& controller = normalController();

    SendObstacle(controller, true, true, true);

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Backward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerObstacleDetectedTest, NoArgumentObstacleUsesStoredSensorState)
{
    Controller& controller = normalController();
    controller.obstacleSensorDriver.front = true;
    controller.obstacleSensorDriver.left = true;
    controller.obstacleSensorDriver.right = true;

    controller.obstacleDetected();

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Backward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerObstacleDetectedTest, BoostFrontClearKeepsBoostCleaning)
{
    Controller& controller = boostController();

    SendObstacle(controller, false, false, false);

    EXPECT_TRUE(IsMode<BoostMode>(controller));
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Forward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerObstacleDetectedTest, BoostAllBlockedMovesBackwardAndRestoresBoost)
{
    Controller& controller = boostController();

    SendObstacle(controller, true, true, true);

    EXPECT_TRUE(IsMode<BoostMode>(controller));
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Backward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerObstacleDetectedTest, StandbyObstacleStopsMovementAndDoesNotStartCleaner)
{
    Controller& controller = poweredOnController();
    controller.motorDriver.start(Direction::Forward);

    SendObstacle(controller, true, false, false);

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.obstacleSensorDriver.front);
}

TEST_F(ControllerObstacleDetectedTest, LowBatteryObstacleKeepsSafeStoppedState)
{
    Controller& controller = lowBatteryController();
    controller.motorDriver.start(Direction::Forward);
    controller.cleanerDriver.startCleaning();

    SendObstacle(controller, true, true, true);

    EXPECT_TRUE(IsMode<LowBatteryMode>(controller));
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Forward);
}

TEST_F(ControllerObstacleDetectedTest, ObstacleDuringNormalStopsThenRestartsNormalCleaner)
{
    Controller& controller = normalController();
    controller.cleanerDriver.decideSetting(true);

    SendObstacle(controller, true, true, true);

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Backward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

class ControllerClockTickTest : public ControllerFixture
{
};

TEST_F(ControllerClockTickTest, OffClockTickInitializesAndRemainsOff)
{
    Controller& controller = NewController();

    controller.clockTick();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::FullBatteryLevel);
    EXPECT_FALSE(controller.batteryDriver.isCharging);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerClockTickTest, OffClockTickAdvancesCharging)
{
    Controller& controller = offController();
    controller.batteryDriver.level = 50;
    controller.batteryDriver.startCharging();

    controller.clockTick();

    EXPECT_TRUE(IsOff(controller));
    EXPECT_EQ(controller.batteryDriver.level, 60);
    EXPECT_TRUE(controller.batteryDriver.isCharging);
}

TEST_F(ControllerClockTickTest, StandbyLowBatteryFlagEntersLowBatteryMode)
{
    Controller& controller = poweredOnController();
    controller.batteryDriver.isLowBattery = true;

    controller.clockTick();

    EXPECT_TRUE(IsMode<LowBatteryMode>(controller));
    EXPECT_EQ(controller.batteryDriver.level, BatteryDriver::LowBatteryThreshold);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerClockTickTest, LowBatteryHasPriorityOverDustAndObstacle)
{
    Controller& controller = normalController();
    controller.batteryDriver.isLowBattery = true;
    controller.dustSensorDriver.dustDetected = true;
    controller.obstacleSensorDriver.front = true;
    controller.obstacleSensorDriver.left = true;
    controller.obstacleSensorDriver.right = true;

    controller.clockTick();

    EXPECT_TRUE(IsMode<LowBatteryMode>(controller));
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
    EXPECT_FALSE(controller.motorDriver.isRunning);
}

TEST_F(ControllerClockTickTest, NormalDustFlagEntersBoost)
{
    Controller& controller = normalController();
    controller.dustSensorDriver.dustDetected = true;

    controller.clockTick();

    EXPECT_TRUE(IsMode<BoostMode>(controller));
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerClockTickTest, NormalObstacleFlagsAreProcessed)
{
    Controller& controller = normalController();
    controller.obstacleSensorDriver.front = true;
    controller.obstacleSensorDriver.left = true;
    controller.obstacleSensorDriver.right = true;

    controller.clockTick();

    EXPECT_TRUE(IsMode<NormalMode>(controller));
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Backward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
}

TEST_F(ControllerClockTickTest, NormalDustAndObstacleAreBothProcessedInOrder)
{
    Controller& controller = normalController();
    controller.dustSensorDriver.dustDetected = true;
    controller.obstacleSensorDriver.front = true;
    controller.obstacleSensorDriver.left = true;
    controller.obstacleSensorDriver.right = true;

    controller.clockTick();

    EXPECT_TRUE(IsMode<BoostMode>(controller));
    EXPECT_EQ(controller.motorDriver.direction, Direction::Backward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.cleanerDriver.isBoosting);
}

TEST_F(ControllerClockTickTest, LowBatteryChargingTickClearsToStandby)
{
    Controller& controller = lowBatteryController();
    controller.batteryDriver.startCharging();

    controller.clockTick();

    EXPECT_TRUE(IsMode<StandbyMode>(controller));
    EXPECT_EQ(controller.batteryDriver.level, 20);
    EXPECT_TRUE(controller.batteryDriver.isCharging);
    EXPECT_FALSE(controller.batteryDriver.isLowBattery);
}

class OperatingModeTransitionTest : public ControllerFixture
{
};

TEST_F(OperatingModeTransitionTest, StandbyStartReturnsNormalAndStartsDrivers)
{
    Controller& controller = offController();
    StandbyMode mode;

    OperatingMode* next = mode.startButtonPressed(controller);
    auto owned = OwnIfNew(next, &mode);

    EXPECT_TRUE(dynamic_cast<NormalMode*>(next) != nullptr);
    EXPECT_TRUE(controller.motorDriver.isRunning);
    EXPECT_EQ(controller.motorDriver.direction, Direction::Forward);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
}

TEST_F(OperatingModeTransitionTest, NormalStartReturnsStandbyAndStopsDrivers)
{
    Controller& controller = offController();
    controller.motorDriver.moveForward();
    controller.cleanerDriver.startCleaning();
    NormalMode mode;

    OperatingMode* next = mode.startButtonPressed(controller);
    auto owned = OwnIfNew(next, &mode);

    EXPECT_TRUE(dynamic_cast<StandbyMode*>(next) != nullptr);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(OperatingModeTransitionTest, BoostStartReturnsStandbyAndStopsBoost)
{
    Controller& controller = offController();
    controller.motorDriver.moveForward();
    controller.cleanerDriver.decideSetting(true);
    BoostMode mode;

    OperatingMode* next = mode.startButtonPressed(controller);
    auto owned = OwnIfNew(next, &mode);

    EXPECT_TRUE(dynamic_cast<StandbyMode*>(next) != nullptr);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(OperatingModeTransitionTest, LowBatteryStartReturnsSelfAndStopsDrivers)
{
    Controller& controller = offController();
    controller.motorDriver.moveForward();
    controller.cleanerDriver.startCleaning();
    LowBatteryMode mode;

    OperatingMode* next = mode.startButtonPressed(controller);
    auto owned = OwnIfNew(next, &mode);

    EXPECT_EQ(next, &mode);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(OperatingModeTransitionTest, NormalDustReturnsBoostAndEnablesBoostCleaner)
{
    Controller& controller = offController();
    controller.cleanerDriver.startCleaning();
    NormalMode mode;

    OperatingMode* next = mode.dustDetected(controller);
    auto owned = OwnIfNew(next, &mode);

    EXPECT_TRUE(dynamic_cast<BoostMode*>(next) != nullptr);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_TRUE(controller.cleanerDriver.isBoosting);
}

TEST_F(OperatingModeTransitionTest, BoostTimerReturnsNormalAndNormalCleaner)
{
    Controller& controller = offController();
    controller.cleanerDriver.decideSetting(true);
    BoostMode mode;

    OperatingMode* next = mode.timerExpired(controller);
    auto owned = OwnIfNew(next, &mode);

    EXPECT_TRUE(dynamic_cast<NormalMode*>(next) != nullptr);
    EXPECT_TRUE(controller.cleanerDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isBoosting);
}

TEST_F(OperatingModeTransitionTest, StandbyLowBatteryReturnsLowBatteryAndStopsDrivers)
{
    Controller& controller = offController();
    controller.motorDriver.moveForward();
    controller.cleanerDriver.startCleaning();
    StandbyMode mode;

    OperatingMode* next = mode.lowBatteryDetected(controller);
    auto owned = OwnIfNew(next, &mode);

    EXPECT_TRUE(dynamic_cast<LowBatteryMode*>(next) != nullptr);
    EXPECT_FALSE(controller.motorDriver.isRunning);
    EXPECT_FALSE(controller.cleanerDriver.isRunning);
}

TEST_F(OperatingModeTransitionTest, LowBatteryClearedReturnsStandby)
{
    Controller& controller = offController();
    LowBatteryMode mode;

    OperatingMode* next = mode.lowBatteryCleared(controller);
    auto owned = OwnIfNew(next, &mode);

    EXPECT_TRUE(dynamic_cast<StandbyMode*>(next) != nullptr);
}

class ObstacleProcessorTest : public ::testing::Test
{
protected:
    ObstacleProcessor processor;

    Direction decide(bool front, bool left, bool right) const
    {
        ObstacleSensorDriver sensor;
        sensor.initialize();
        sensor.front = front;
        sensor.left = left;
        sensor.right = right;
        return processor.decideDirection(sensor);
    }
};

TEST_F(ObstacleProcessorTest, AllClearChoosesForward)
{
    EXPECT_EQ(decide(false, false, false), Direction::Forward);
}

TEST_F(ObstacleProcessorTest, FrontClearIgnoresLeftObstacle)
{
    EXPECT_EQ(decide(false, true, false), Direction::Forward);
}

TEST_F(ObstacleProcessorTest, FrontClearIgnoresRightObstacle)
{
    EXPECT_EQ(decide(false, false, true), Direction::Forward);
}

TEST_F(ObstacleProcessorTest, FrontClearIgnoresSideObstacles)
{
    EXPECT_EQ(decide(false, true, true), Direction::Forward);
}

TEST_F(ObstacleProcessorTest, FrontBlockedWithBothSidesClearChoosesLeft)
{
    EXPECT_EQ(decide(true, false, false), Direction::Left);
}

TEST_F(ObstacleProcessorTest, FrontBlockedWithOnlyLeftClearChoosesLeft)
{
    EXPECT_EQ(decide(true, false, true), Direction::Left);
}

TEST_F(ObstacleProcessorTest, FrontAndLeftBlockedWithRightClearChoosesRight)
{
    EXPECT_EQ(decide(true, true, false), Direction::Right);
}

TEST_F(ObstacleProcessorTest, AllDirectionsBlockedChoosesBackward)
{
    EXPECT_EQ(decide(true, true, true), Direction::Backward);
}

class DustProcessorTest : public ::testing::Test
{
protected:
    DustProcessor processor;
    CleanerDriver cleaner;

    void SetUp() override
    {
        cleaner.initialize();
    }
};

TEST_F(DustProcessorTest, StoppedCleanerShouldNotBoost)
{
    EXPECT_FALSE(processor.shouldBoost(cleaner));
}

TEST_F(DustProcessorTest, RunningNormalCleanerShouldBoost)
{
    cleaner.startCleaning();

    EXPECT_TRUE(processor.shouldBoost(cleaner));
}

TEST_F(DustProcessorTest, AlreadyBoostingCleanerShouldNotBoostAgain)
{
    cleaner.decideSetting(true);

    EXPECT_FALSE(processor.shouldBoost(cleaner));
}

TEST_F(DustProcessorTest, StoppedAfterBoostShouldNotBoost)
{
    cleaner.decideSetting(true);
    cleaner.stopCleaning();

    EXPECT_FALSE(processor.shouldBoost(cleaner));
}

class DriverStateTest : public ::testing::Test
{
};

TEST_F(DriverStateTest, BatteryInitializeSetsFullNonChargingState)
{
    BatteryDriver battery;

    battery.initialize();

    EXPECT_EQ(battery.level, BatteryDriver::FullBatteryLevel);
    EXPECT_FALSE(battery.isCharging);
    EXPECT_FALSE(battery.isLowBattery);
    EXPECT_TRUE(battery.isFull());
}

TEST_F(DriverStateTest, BatteryStartsAndStopsChargingBelowFull)
{
    BatteryDriver battery;
    battery.initialize();
    battery.level = 50;

    battery.startCharging();
    ASSERT_TRUE(battery.isCharging);

    battery.stopCharging();

    EXPECT_FALSE(battery.isCharging);
    EXPECT_EQ(battery.level, 50);
}

TEST_F(DriverStateTest, BatteryFullDoesNotChargeOrIncrease)
{
    BatteryDriver battery;
    battery.initialize();

    battery.startCharging();
    const bool increased = battery.inclineLevel();

    EXPECT_FALSE(battery.isCharging);
    EXPECT_FALSE(increased);
    EXPECT_EQ(battery.level, BatteryDriver::FullBatteryLevel);
}

TEST_F(DriverStateTest, BatteryLowFlagSetsThresholdLevel)
{
    BatteryDriver battery;
    battery.initialize();
    battery.level = 70;

    battery.setLowBattery(true);

    EXPECT_TRUE(battery.isLowBattery);
    EXPECT_EQ(battery.level, BatteryDriver::LowBatteryThreshold);
}

TEST_F(DriverStateTest, CleanerInitializeAndStartCleaning)
{
    CleanerDriver cleaner;

    cleaner.initialize();
    ASSERT_FALSE(cleaner.isRunning);

    cleaner.startCleaning();

    EXPECT_TRUE(cleaner.isRunning);
    EXPECT_FALSE(cleaner.isBoosting);
}

TEST_F(DriverStateTest, CleanerBoostNormalAndStopTransitions)
{
    CleanerDriver cleaner;
    cleaner.initialize();

    cleaner.boost();
    ASSERT_TRUE(cleaner.isRunning);
    ASSERT_TRUE(cleaner.isBoosting);

    cleaner.normal();
    ASSERT_TRUE(cleaner.isRunning);
    ASSERT_FALSE(cleaner.isBoosting);

    cleaner.stopCleaning();

    EXPECT_FALSE(cleaner.isRunning);
    EXPECT_FALSE(cleaner.isBoosting);
}

TEST_F(DriverStateTest, MotorInitializeAndMoveForward)
{
    MotorDriver motor;

    motor.initialize();
    ASSERT_FALSE(motor.isRunning);
    ASSERT_EQ(motor.direction, Direction::Forward);

    motor.moveForward();

    EXPECT_TRUE(motor.isRunning);
    EXPECT_EQ(motor.direction, Direction::Forward);
}

TEST_F(DriverStateTest, MotorTurnsAndMovesBackward)
{
    MotorDriver motor;
    motor.initialize();

    motor.turnLeft();
    ASSERT_TRUE(motor.isRunning);
    ASSERT_EQ(motor.direction, Direction::Left);

    motor.turnRight();
    ASSERT_EQ(motor.direction, Direction::Right);

    motor.moveBackward();

    EXPECT_TRUE(motor.isRunning);
    EXPECT_EQ(motor.direction, Direction::Backward);
}

TEST_F(DriverStateTest, ObstacleSensorClearAndHasObstacle)
{
    ObstacleSensorDriver sensor;

    sensor.initialize();
    ASSERT_FALSE(sensor.hasObstacle());

    sensor.front = true;
    ASSERT_TRUE(sensor.hasObstacle());

    sensor.clear();

    EXPECT_FALSE(sensor.front);
    EXPECT_FALSE(sensor.left);
    EXPECT_FALSE(sensor.right);
    EXPECT_FALSE(sensor.hasObstacle());
}

TEST_F(DriverStateTest, DustSensorInitializeDeactivateAndClear)
{
    DustSensorDriver sensor;

    sensor.initialize();
    ASSERT_FALSE(sensor.dustDetected);

    sensor.dustDetected = true;
    sensor.deactivateDustSensor();

    EXPECT_FALSE(sensor.dustDetected);
}

} // namespace
