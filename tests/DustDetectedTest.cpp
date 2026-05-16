#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "rvc/Controller.hpp"
#include "rvc/DustProcessor.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/DustSensorDriver.hpp"
#include "rvc/Modes.hpp"

using namespace rvc;

namespace
{
    void waitForBoostTimer()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5600));
    }

    void powerOnAndEnterNormalMode(Controller &controller)
    {
        controller.powerButtonPressed(); // Power OFF -> StandbyMode
        controller.startButtonPressed(); // StandbyMode -> NormalMode
    }
}

// 1. 목적: 전원이 꺼져 있을 때 dustDetected()가 무시되는지 확인
// 기대값: power off 유지, currentMode 없음, modeName은 "Off"
TEST(DustDetectedControllerTest, DustDetectedDoesNothingWhenPowerIsOff)
{
    Controller controller;

    ASSERT_FALSE(controller.isPowerOn());
    ASSERT_FALSE(controller.hasCurrentMode());

    controller.dustDetected();

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ("Off", controller.currentModeName());
}

// 2. 목적: StandbyMode에서 먼지가 감지되어도 BoostMode로 전환되지 않는지 확인
// 기대값: StandbyMode 유지, cleanerMode는 "off"
TEST(DustDetectedControllerTest, DustDetectedInStandbyModeKeepsStandbyMode)
{
    Controller controller;
    controller.powerButtonPressed();

    ASSERT_TRUE(controller.isPowerOn());
    ASSERT_EQ(ModeKind::Standby, controller.currentModeKind());

    controller.dustDetected();

    EXPECT_EQ(ModeKind::Standby, controller.currentModeKind());
    EXPECT_EQ("off", controller.cleanerMode());
}

// 3. 목적: NormalMode에서 먼지 감지 시 BoostMode로 전환되는지 확인
// 기대값: currentMode가 BoostMode
TEST(DustDetectedControllerTest, DustDetectedInNormalModeChangesToBoostMode)
{
    Controller controller;
    powerOnAndEnterNormalMode(controller);

    ASSERT_EQ(ModeKind::Normal, controller.currentModeKind());

    controller.dustDetected();

    EXPECT_EQ(ModeKind::Boost, controller.currentModeKind());

    waitForBoostTimer();
}

// 4. 목적: NormalMode에서 먼지 감지 시 CleanerDriver 설정이 boost로 바뀌는지 확인
// 기대값: cleanerMode가 "boost", cleaning 상태 true
TEST(DustDetectedControllerTest, DustDetectedInNormalModeSetsCleanerToBoost)
{
    Controller controller;
    powerOnAndEnterNormalMode(controller);

    controller.dustDetected();

    EXPECT_EQ("boost", controller.cleanerMode());
    EXPECT_TRUE(controller.isCleanerCleaning());

    waitForBoostTimer();
}

// 5. 목적: 먼지 감지 후에도 시스템 전원이 유지되는지 확인
// 기대값: power on 유지
TEST(DustDetectedControllerTest, DustDetectedInNormalModeKeepsPowerOn)
{
    Controller controller;
    powerOnAndEnterNormalMode(controller);

    controller.dustDetected();

    EXPECT_TRUE(controller.isPowerOn());

    waitForBoostTimer();
}

// 6. 목적: 먼지 감지 후에도 currentMode 포인터가 유지되는지 확인
// 기대값: hasCurrentMode() == true
TEST(DustDetectedControllerTest, DustDetectedInNormalModeKeepsCurrentModeExists)
{
    Controller controller;
    powerOnAndEnterNormalMode(controller);

    controller.dustDetected();

    EXPECT_TRUE(controller.hasCurrentMode());

    waitForBoostTimer();
}

// 7. 목적: BoostMode 진입 후 타이머 만료 시 NormalMode로 복귀하는지 확인
// 기대값: timerExpired 이후 currentMode가 NormalMode
TEST(DustDetectedControllerTest, BoostModeReturnsToNormalModeAfterTimerExpired)
{
    Controller controller;
    powerOnAndEnterNormalMode(controller);

    controller.dustDetected();
    ASSERT_EQ(ModeKind::Boost, controller.currentModeKind());

    waitForBoostTimer();

    EXPECT_EQ(ModeKind::Normal, controller.currentModeKind());
}

// 8. 목적: BoostMode 진입 후 타이머가 만료되면 currentMode는 NormalMode로 돌아가지만,
//      현재 구현에서는 CleanerDriver의 mode 문자열은 "boost"로 유지되는지 확인
// 기대값: currentMode는 NormalMode, cleanerMode는 "boost"
TEST(DustDetectedControllerTest, CleanerModeStaysBoostAfterTimerExpired)
{
    Controller controller;
    powerOnAndEnterNormalMode(controller);

    controller.dustDetected();
    ASSERT_EQ(ModeKind::Boost, controller.currentModeKind());
    ASSERT_EQ("boost", controller.cleanerMode());

    waitForBoostTimer();

    EXPECT_EQ(ModeKind::Normal, controller.currentModeKind());
    EXPECT_EQ("normal", controller.cleanerMode());
}

// 9. 목적: BoostMode 종료 후에도 청소 동작은 계속 유지되는지 확인
// 기대값: timerExpired 이후에도 isCleanerCleaning() == true
TEST(DustDetectedControllerTest, CleanerStillCleaningAfterTimerExpired)
{
    Controller controller;
    powerOnAndEnterNormalMode(controller);

    controller.dustDetected();

    waitForBoostTimer();

    EXPECT_TRUE(controller.isCleanerCleaning());
}

// 10. 목적: BoostMode 상태에서 dustDetected()를 다시 호출해도 즉시 BoostMode가 유지되는지 확인
// 기대값: currentMode는 BoostMode, cleanerMode는 "boost"
TEST(DustDetectedControllerTest, RepeatedDustDetectedInBoostModeKeepsBoostModeImmediately)
{
    Controller controller;
    powerOnAndEnterNormalMode(controller);

    controller.dustDetected();
    ASSERT_EQ(ModeKind::Boost, controller.currentModeKind());

    controller.dustDetected();

    EXPECT_EQ(ModeKind::Boost, controller.currentModeKind());
    EXPECT_EQ("boost", controller.cleanerMode());

    waitForBoostTimer();
}

// 11. 목적: 타이머 만료 후 다시 먼지가 감지되면 다시 BoostMode로 진입하는지 확인
// 기대값: NormalMode 복귀 후 dustDetected() 재호출 시 BoostMode
TEST(DustDetectedControllerTest, DustDetectedCanBoostAgainAfterTimerExpired)
{
    Controller controller;
    powerOnAndEnterNormalMode(controller);

    controller.dustDetected();
    waitForBoostTimer();

    ASSERT_EQ(ModeKind::Normal, controller.currentModeKind());

    controller.dustDetected();

    EXPECT_EQ(ModeKind::Boost, controller.currentModeKind());
    EXPECT_EQ("boost", controller.cleanerMode());

    waitForBoostTimer();
}

// 12. 목적: 충전 중에는 dustDetected()가 무시되는지 확인
// 기대값: charging 유지, StandbyMode 유지, cleanerMode는 "off"
TEST(DustDetectedControllerTest, DustDetectedDoesNothingWhileCharging)
{
    Controller controller;

    controller.powerButtonPressed();
    controller.setBatteryLevel(50);
    controller.chargeBattery();

    ASSERT_TRUE(controller.isPowerOn());
    ASSERT_TRUE(controller.isCharging());

    controller.dustDetected();

    EXPECT_TRUE(controller.isCharging());
    EXPECT_EQ(ModeKind::Standby, controller.currentModeKind());
    EXPECT_EQ("off", controller.cleanerMode());
}

// 13. 목적: 전원 OFF 이후 dustDetected()가 무시되는지 확인
// 기대값: currentMode 없음, modeName은 "Off"
TEST(DustDetectedControllerTest, DustDetectedDoesNothingAfterPowerOff)
{
    Controller controller;
    powerOnAndEnterNormalMode(controller);

    controller.powerButtonPressed(); // Power ON -> OFF
    ASSERT_FALSE(controller.isPowerOn());

    controller.dustDetected();

    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_EQ("Off", controller.currentModeName());
}

// 14. 목적: LowBatteryMode에서는 먼지가 감지되어도 BoostMode로 전환되지 않는지 확인
// 기대값: LowBatteryMode 유지, cleanerMode는 "off"
TEST(DustDetectedControllerTest, DustDetectedInLowBatteryModeKeepsLowBatteryMode)
{
    Controller controller;
    powerOnAndEnterNormalMode(controller);

    controller.setBatteryLevel(5);
    controller.lowBatteryDetected();

    ASSERT_EQ(ModeKind::LowBattery, controller.currentModeKind());

    controller.dustDetected();

    EXPECT_EQ(ModeKind::LowBattery, controller.currentModeKind());
    EXPECT_EQ("off", controller.cleanerMode());
}

// 15. 목적: dustDetected()가 배터리 잔량을 직접 변경하지 않는지 확인
// 기대값: 호출 전후 batteryLevel 동일
TEST(DustDetectedControllerTest, DustDetectedDoesNotChangeBatteryLevel)
{
    Controller controller;
    powerOnAndEnterNormalMode(controller);

    const int beforeLevel = controller.batteryLevel();

    controller.dustDetected();

    EXPECT_EQ(beforeLevel, controller.batteryLevel());

    waitForBoostTimer();
}

// 16. 목적: dustDetected() 후에도 DustSensorDriver가 활성 상태를 유지하는지 확인
// 기대값: isDustSensorActive() == true
TEST(DustDetectedControllerTest, DustSensorStaysActiveAfterDustDetected)
{
    Controller controller;
    powerOnAndEnterNormalMode(controller);

    ASSERT_TRUE(controller.isDustSensorActive());

    controller.dustDetected();

    EXPECT_TRUE(controller.isDustSensorActive());

    waitForBoostTimer();
}

// 17. 목적: DustProcessor가 StandbyMode에서 먼지 감지 처리를 했을 때 StandbyMode를 유지하는지 확인
// 기대값: 반환 mode kind가 Standby
TEST(DustDetectedControllerTest, DustProcessorInStandbyModeReturnsStandbyMode)
{
    CleanerDriver cleaner;
    DustProcessor processor;

    OperatingMode &nextMode = processor.decideIsDusted(cleaner, standbyMode());

    EXPECT_EQ(ModeKind::Standby, nextMode.kind());
}

// 18. 목적: DustProcessor가 NormalMode에서 먼지 감지 처리를 했을 때 BoostMode를 반환하는지 확인
// 기대값: 반환 mode kind가 Boost, cleanerMode가 "boost"
TEST(DustDetectedControllerTest, DustProcessorInNormalModeReturnsBoostMode)
{
    CleanerDriver cleaner;
    cleaner.initialize();

    DustProcessor processor;

    OperatingMode &nextMode = processor.decideIsDusted(cleaner, normalMode());

    EXPECT_EQ(ModeKind::Boost, nextMode.kind());
    EXPECT_EQ("boost", cleaner.currentMode());
}

// 19. 목적: DustProcessor가 BoostMode에서 먼지 감지 처리를 했을 때 BoostMode를 유지하는지 확인
// 기대값: 반환 mode kind가 Boost
TEST(DustDetectedControllerTest, DustProcessorInBoostModeKeepsBoostMode)
{
    CleanerDriver cleaner;
    DustProcessor processor;

    OperatingMode &nextMode = processor.decideIsDusted(cleaner, boostMode());

    EXPECT_EQ(ModeKind::Boost, nextMode.kind());
}

// 20. 목적: DustProcessor가 LowBatteryMode에서 먼지 감지 처리를 했을 때 LowBatteryMode를 유지하는지 확인
// 기대값: 반환 mode kind가 LowBattery
TEST(DustDetectedControllerTest, DustProcessorInLowBatteryModeKeepsLowBatteryMode)
{
    CleanerDriver cleaner;
    DustProcessor processor;

    OperatingMode &nextMode = processor.decideIsDusted(cleaner, lowBatteryMode());

    EXPECT_EQ(ModeKind::LowBattery, nextMode.kind());
}