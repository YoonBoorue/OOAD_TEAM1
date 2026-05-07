#include <gtest/gtest.h>
#include <thread>
#include <chrono>

// SD-06의 system operation인 Controller::dustDetected() 검증

#define private public
#include "rvc/Controller.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/DustSensorDriver.hpp"
#undef private

#include "rvc/Modes.hpp"
#include "rvc/OperatingMode.hpp"

using namespace rvc;

namespace
{

// Driver: DustSensorDriver actor 대신 system operation 호출
class DustDetectedDriver
{
public:
    explicit DustDetectedDriver(Controller &controller) : controller(controller) {}

    void detectDust()
    {
        controller.dustDetected();
    }

private:
    Controller &controller;
};

// Stub: Controller -> OperatingMode 위임 정보 기록용
class StubOperatingMode : public OperatingMode
{
public:
    int dustDetectedCallCount = 0;
    int timerExpiredCallCount = 0;
    CleanerDriver *receivedCleaner = nullptr;
    OperatingMode *nextMode = nullptr;

    explicit StubOperatingMode(OperatingMode *next = nullptr)
        : nextMode(next == nullptr ? this : next) {}

    void checkIsMoving(Direction, MotorDriver &) const override {}
    OperatingMode &startButtonPressed(MotorDriver &, CleanerDriver &) override { return *this; }
    OperatingMode &lowBatteryDetected(MotorDriver &, CleanerDriver &) override { return *this; }
    OperatingMode &lowBatteryCleared() override { return *this; }
    OperatingMode &dustDetected(CleanerDriver &cleaner) override
    {
        ++dustDetectedCallCount;
        receivedCleaner = &cleaner;
        return *nextMode;
    }
    bool canCharge() const override { return false; }
    OperatingMode &timerExpired(CleanerDriver &cleaner) override
    {
        ++timerExpiredCallCount;
        return *this;
    }
};

class ControllerDustDetectedTest : public testing::Test
{
protected:
    Controller controller;
    DustDetectedDriver driver{controller};

    void SetUp() override
    {
        controller.dustSensorDriver->dust = true;
    }

    void setRealMode(OperatingMode *mode)
    {
        controller.currentMode = mode;
    }

    StubOperatingMode *setStubMode(StubOperatingMode *mode)
    {
        controller.currentMode = mode;
        return mode;
    }
};

// TC-01
// 목적: NormalMode에서 먼지 감지 시 CleanerDriver mode가 boost로 변경되는지 확인
// 상황: currentMode는 실제 NormalMode
// 실행: Driver가 dustDetected 전달
// 기대값: CleanerDriver mode == "boost"
TEST_F(ControllerDustDetectedTest, NormalModeChangesCleanerToBoostOnDustDetected)
{
    setRealMode(new NormalMode());
    driver.detectDust();
    EXPECT_EQ(controller.cleanerDriver->mode, "boost");
}

// TC-02
// 목적: NormalMode에서 먼지 감지 시 currentMode가 BoostMode로 전환되는지 확인
// 상황: currentMode는 실제 NormalMode
// 실행: Driver가 dustDetected 전달
// 기대값: currentMode는 BoostMode 타입
TEST_F(ControllerDustDetectedTest, NormalModeTransitionsToBoostModeOnDustDetected)
{
    setRealMode(new NormalMode());
    driver.detectDust();
    EXPECT_NE(dynamic_cast<BoostMode *>(controller.currentMode), nullptr);
}

// TC-03
// 목적: BoostMode에서 dustDetected 호출 시 startTimer가 실행되는지 확인
// 상황: currentMode는 실제 BoostMode
// 실행: Driver가 dustDetected 전달
// 기대값: currentMode는 여전히 BoostMode 타입 (Boost→Boost 유지)
TEST_F(ControllerDustDetectedTest, BoostModeStaysBoostOnDustDetected)
{
    setRealMode(new BoostMode());
    driver.detectDust();
    EXPECT_NE(dynamic_cast<BoostMode *>(controller.currentMode), nullptr);
}

// TC-04
// 목적: Controller가 currentMode->dustDetected()를 정확히 1회 호출하는지 확인
// 상황: stub mode 사용
// 실행: Driver가 dustDetected 1회 전달
// 기대값: stub의 dustDetected 호출 1회
TEST_F(ControllerDustDetectedTest, DelegatesToCurrentModeOnce)
{
    auto *mode = setStubMode(new StubOperatingMode());
    driver.detectDust();
    EXPECT_EQ(mode->dustDetectedCallCount, 1);
}

// TC-05
// 목적: Controller가 dustDetected에 자신의 CleanerDriver를 전달하는지 확인
// 상황: stub mode 사용
// 실행: Driver가 dustDetected 전달
// 기대값: stub이 받은 cleaner 주소 == controller.cleanerDriver
TEST_F(ControllerDustDetectedTest, PassesControllerCleanerDriverToCurrentMode)
{
    auto *mode = setStubMode(new StubOperatingMode());
    driver.detectDust();
    EXPECT_EQ(mode->receivedCleaner, controller.cleanerDriver);
}

// TC-06
// 목적: dustDetected 반환 모드가 currentMode에 반영되는지 확인
// 상황: stub mode가 다른 stub을 반환하도록 설정
// 실행: Driver가 dustDetected 전달
// 기대값: currentMode == nextMode
TEST_F(ControllerDustDetectedTest, ChangesCurrentModeToReturnedMode)
{
    auto *nextMode = new StubOperatingMode();
    auto *firstMode = setStubMode(new StubOperatingMode(nextMode));
    driver.detectDust();
    EXPECT_EQ(controller.currentMode, nextMode);
    delete firstMode;
}

// TC-07
// 목적: StandbyMode에서 dustDetected 호출 시 모드 변경 없는지 확인
// 상황: currentMode는 실제 StandbyMode
// 실행: Driver가 dustDetected 전달
// 기대값: currentMode는 여전히 StandbyMode 타입
TEST_F(ControllerDustDetectedTest, StandbyModeDoesNotTransitionOnDustDetected)
{
    setRealMode(new StandbyMode());
    driver.detectDust();
    EXPECT_NE(dynamic_cast<StandbyMode *>(controller.currentMode), nullptr);
}

// TC-08
// 목적: StandbyMode에서 dustDetected 호출 시 CleanerDriver mode 변경 없는지 확인
// 상황: currentMode는 실제 StandbyMode
// 실행: Driver가 dustDetected 전달
// 기대값: CleanerDriver mode == "off"
TEST_F(ControllerDustDetectedTest, StandbyModeDoesNotChangeCleanerOnDustDetected)
{
    setRealMode(new StandbyMode());
    driver.detectDust();
    EXPECT_EQ(controller.cleanerDriver->mode, "off");
}

// TC-09
// 목적: dustDetected 반복 호출 시 매번 currentMode->dustDetected 호출되는지 확인
// 상황: stub mode 사용
// 실행: Driver가 dustDetected 2회 전달
// 기대값: stub의 dustDetected 호출 2회
TEST_F(ControllerDustDetectedTest, MultipleDustDetectedCallsDelegateEachTime)
{
    auto *mode = setStubMode(new StubOperatingMode());
    driver.detectDust();
    driver.detectDust();
    EXPECT_EQ(mode->dustDetectedCallCount, 2);
}

// TC-10
// 목적: NormalMode → Boost 전환 후 다시 dustDetected 호출 시 크래시 없는지 확인
// 상황: currentMode는 실제 NormalMode
// 실행: Driver가 dustDetected 2회 전달
// 기대값: 정상 종료
TEST_F(ControllerDustDetectedTest, MultipleDustDetectedCallsDoNotCrash)
{
    setRealMode(new NormalMode());
    EXPECT_NO_THROW({
        driver.detectDust();
        driver.detectDust();
    });
}

// TC-11
// 목적: LowBatteryMode에서 dustDetected 호출 시 모드 변경 없는지 확인
// 상황: currentMode는 실제 LowBatteryMode
// 실행: Driver가 dustDetected 전달
// 기대값: currentMode는 여전히 LowBatteryMode 타입
TEST_F(ControllerDustDetectedTest, LowBatteryModeDoesNotTransitionOnDustDetected)
{
    setRealMode(new LowBatteryMode());
    driver.detectDust();
    EXPECT_NE(dynamic_cast<LowBatteryMode *>(controller.currentMode), nullptr);
}

// TC-12
// 목적: LowBatteryMode에서 dustDetected 호출 시 CleanerDriver mode 변경 없는지 확인
// 상황: currentMode는 실제 LowBatteryMode
// 실행: Driver가 dustDetected 전달
// 기대값: CleanerDriver mode == "off"
TEST_F(ControllerDustDetectedTest, LowBatteryModeDoesNotChangeCleanerOnDustDetected)
{
    setRealMode(new LowBatteryMode());
    driver.detectDust();
    EXPECT_EQ(controller.cleanerDriver->mode, "off");
}

// TC-13
// 목적: 첫 dustDetected에서 바뀐 currentMode가 다음 호출에 사용되는지 확인
// 상황: stub mode 사용, firstMode가 secondMode 반환
// 실행: Driver가 dustDetected 2회 전달
// 기대값: firstMode 1회, secondMode 1회 호출
TEST_F(ControllerDustDetectedTest, SecondCallUsesUpdatedCurrentMode)
{
    auto *secondMode = new StubOperatingMode();
    auto *firstMode = setStubMode(new StubOperatingMode(secondMode));
    driver.detectDust();
    driver.detectDust();
    EXPECT_EQ(firstMode->dustDetectedCallCount, 1);
    EXPECT_EQ(secondMode->dustDetectedCallCount, 1);
    delete firstMode;
}

// TC-14
// 목적: NormalMode에서 dustDetected 전후 CleanerDriver mode 확인
// 상황: currentMode는 실제 NormalMode, CleanerDriver 초기 mode는 off
// 실행: Driver가 dustDetected 전달
// 기대값: 전 == "off", 후 == "boost"
TEST_F(ControllerDustDetectedTest, CleanerModeIsOffBeforeAndBoostAfterDustDetected)
{
    setRealMode(new NormalMode());
    EXPECT_EQ(controller.cleanerDriver->mode, "off");
    driver.detectDust();
    EXPECT_EQ(controller.cleanerDriver->mode, "boost");
}

// TC-15
// 목적: stub mode가 자기 자신을 반환할 때 currentMode 유지 확인
// 상황: stub mode가 자기 자신 반환
// 실행: Driver가 dustDetected 전달
// 기대값: currentMode == 기존 stub mode
TEST_F(ControllerDustDetectedTest, KeepsCurrentModeWhenReturnedModeIsSame)
{
    auto *mode = setStubMode(new StubOperatingMode());
    driver.detectDust();
    EXPECT_EQ(controller.currentMode, mode);
}

// TC-16
// 목적: NormalMode → Boost 전환 시 BoostMode canCharge가 false인지 확인
// 상황: currentMode는 실제 NormalMode
// 실행: Driver가 dustDetected 전달
// 기대값: currentMode->canCharge() == false
TEST_F(ControllerDustDetectedTest, BoostModeCannotChargeAfterDustDetected)
{
    setRealMode(new NormalMode());
    driver.detectDust();
    EXPECT_FALSE(controller.currentMode->canCharge());
}

// TC-17
// 목적: 먼지 미감지 시 모드 변경 없는지 확인
// 상황: dustSensorDriver->dust == false
// 실행: Driver가 dustDetected 전달
// 기대값: currentMode 변경 없음
TEST_F(ControllerDustDetectedTest, NoTransitionWhenDustNotDetected)
{
    controller.dustSensorDriver->dust = false;
    auto *mode = new NormalMode();
    setRealMode(mode);
    driver.detectDust();
    EXPECT_EQ(controller.currentMode, mode);
}

// TC-18
// 목적: 먼지 미감지 시 CleanerDriver mode 변경 없는지 확인
// 상황: dustSensorDriver->dust == false
// 실행: Driver가 dustDetected 전달
// 기대값: CleanerDriver mode == "off"
TEST_F(ControllerDustDetectedTest, CleanerModeUnchangedWhenDustNotDetected)
{
    controller.dustSensorDriver->dust = false;
    setRealMode(new NormalMode());
    driver.detectDust();
    EXPECT_EQ(controller.cleanerDriver->mode, "off");
}

// TC-19
// 목적: NormalMode에서 dustDetected 후 BoostMode 상태 종합 확인
// 상황: currentMode는 실제 NormalMode
// 실행: Driver가 dustDetected 전달
// 기대값: canCharge == false, cleaner mode == "boost"
TEST_F(ControllerDustDetectedTest, NormalModeAfterDustDetectedBoostModeStateIsCorrect)
{
    setRealMode(new NormalMode());
    driver.detectDust();
    EXPECT_FALSE(controller.currentMode->canCharge());
    EXPECT_EQ(controller.cleanerDriver->mode, "boost");
}

// TC-20
// 목적: dust값 true/false 반복 시 각각 올바르게 동작하는지 확인
// 상황: 첫 번째는 dust=true, 두 번째는 dust=false
// 실행: Driver가 dustDetected 2회 전달
// 기대값: 첫 번째 후 boost, 두 번째 후 변경 없음
TEST_F(ControllerDustDetectedTest, DustTrueThenFalseHandledCorrectly)
{
    setRealMode(new NormalMode());
    controller.dustSensorDriver->dust = true;
    driver.detectDust();
    EXPECT_EQ(controller.cleanerDriver->mode, "boost");

    controller.dustSensorDriver->dust = false;
    driver.detectDust();
    EXPECT_EQ(controller.cleanerDriver->mode, "boost");
}

} // namespace