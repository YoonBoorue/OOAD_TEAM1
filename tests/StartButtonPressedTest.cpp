#include <gtest/gtest.h>

// SD-02의 system operation인 Controller::startButtonPressed() 검증

// TC-01~TC-06: 실제 StandbyMode, MotorDriver, CleanerDriver 구현으로 SD 내부 동작 확인
// TC-07~TC-15: 실제 구현만으로 관찰하기 어려운 위임 횟수, 전달 인자, 반환 모드 갱신만 Stub 사용

// 현재 Controller 생성자는 currentMode 초기화가 없어서 테스트에서만 내부 상태 세팅
#define private public
#include "rvc/Controller.hpp"
#include "rvc/CleanerDriver.hpp"
#include "rvc/MotorDriver.hpp"
#undef private

#include "rvc/Modes.hpp"
#include "rvc/OperatingMode.hpp"

using namespace rvc;

namespace
{

// Stub: 실제 구현만으로 확인하기 어려운 Controller -> OperatingMode 위임 정보 기록용
class StubOperatingMode : public OperatingMode
{
public:
    explicit StubOperatingMode(OperatingMode *nextMode = nullptr)
        : nextMode(nextMode == nullptr ? this : nextMode) // stub 설정값: 반환할 모드 제어
    {
    }

    int startButtonPressedCallCount = 0; // stub 기록값: startButtonPressed 호출 횟수
    MotorDriver *receivedMotor = nullptr; // stub 기록값: Controller가 전달한 MotorDriver 주소
    CleanerDriver *receivedCleaner = nullptr; // stub 기록값: Controller가 전달한 CleanerDriver 주소
    OperatingMode *nextMode = nullptr; // stub 설정값: currentMode 갱신 결과 제어

    void checkIsMoving(Direction, MotorDriver &) const override {} // stub: SD-02 테스트 범위 밖

    OperatingMode &startButtonPressed(MotorDriver &motor, CleanerDriver &cleaner) override
    {
        ++startButtonPressedCallCount; // stub 기록: 위임 호출 여부 확인
        receivedMotor = &motor; // stub 기록: motor 인자 확인
        receivedCleaner = &cleaner; // stub 기록: cleaner 인자 확인
        return *nextMode; // stub 반환: currentMode 대입 확인
    }

    OperatingMode &lowBatteryDetected(MotorDriver &, CleanerDriver &) override { return *this; } // stub: 미사용
    OperatingMode &lowBatteryCleared() override { return *this; } // stub: 미사용
    OperatingMode &dustDetected(CleanerDriver &) override { return *this; } // stub: 미사용
    bool canCharge() const override { return false; } // stub: 미사용
    OperatingMode &timerExpired(CleanerDriver &) override { return *this; } // stub: 미사용
};

// Driver: 사람 모양 actor나 실제 Start Button UI 대신 system operation 호출
class StartButtonPressedDriver
{
public:
    explicit StartButtonPressedDriver(Controller &controller) : controller(controller) {}

    void pressStartButton()
    {
        controller.startButtonPressed();
    }

private:
    Controller &controller;
};

class ControllerStartButtonPressedTest : public testing::Test
{
protected:
    Controller controller;
    StartButtonPressedDriver driver{controller};

    StubOperatingMode *setStubMode(StubOperatingMode *mode)
    {
        controller.currentMode = mode; // stub 주입: 위임 정보 관찰용
        return mode;
    }

    OperatingMode *setRealMode(OperatingMode *mode)
    {
        controller.currentMode = mode; // 실제 구현 주입: SD 내부 동작 관찰용
        return mode;
    }

    void setCharging(bool charging)
    {
        controller.isNowCharging = charging;
    }
};

TEST_F(ControllerStartButtonPressedTest, StandbyModeMovesMotorForwardWhenStartButtonPressed)
{
    // TC-01
    // 목적: SD-02 내부 ref UC3 MoveForward가 실제 구현으로 수행되는지 확인
    // 상황: currentMode는 실제 StandbyMode, 로봇은 충전 중 아님
    // 실행: Driver가 Start Button 입력 전달
    // 기대값: MotorDriver status true, forward true, 방향 FRONT 유지
    setRealMode(new StandbyMode());
    setCharging(false);

    driver.pressStartButton();

    EXPECT_TRUE(controller.motorDriver->status);
    EXPECT_TRUE(controller.motorDriver->forward);
    EXPECT_EQ(controller.motorDriver->moveDirection, Direction::FRONT);
}

TEST_F(ControllerStartButtonPressedTest, StandbyModeStartsCleanerWhenStartButtonPressed)
{
    // TC-02
    // 목적: SD-02 내부 ref UC4 StartCleaning이 실제 구현으로 수행되는지 확인
    // 상황: currentMode는 실제 StandbyMode, CleanerDriver 초기 mode는 off
    // 실행: Driver가 Start Button 입력 전달
    // 기대값: CleanerDriver mode는 normal
    setRealMode(new StandbyMode());
    setCharging(false);

    driver.pressStartButton();

    EXPECT_EQ(controller.cleanerDriver->mode, "normal");
}

TEST_F(ControllerStartButtonPressedTest, StandbyModeKeepsCurrentModeAfterStartButtonPressed)
{
    // TC-03
    // 목적: 실제 StandbyMode 반환값이 currentMode에 반영된 뒤에도 StandbyMode 유지 확인
    // 상황: currentMode는 실제 StandbyMode, 로봇은 충전 중 아님
    // 실행: Driver가 Start Button 입력 전달
    // 기대값: currentMode 포인터는 기존 StandbyMode 객체 그대로 유지
    auto *mode = setRealMode(new StandbyMode());
    setCharging(false);

    driver.pressStartButton();

    EXPECT_EQ(controller.currentMode, mode);
}

TEST_F(ControllerStartButtonPressedTest, ChargingBlocksMoveForwardInRealStandbyMode)
{
    // TC-04
    // 목적: 충전 중이면 SD-02 opt 조건으로 실제 MoveForward까지 도달하지 않는지 확인
    // 상황: currentMode는 실제 StandbyMode, 로봇은 충전 중
    // 실행: Driver가 Start Button 입력 전달
    // 기대값: MotorDriver status false, forward false, 방향 FRONT 유지
    setRealMode(new StandbyMode());
    setCharging(true);

    driver.pressStartButton();

    EXPECT_FALSE(controller.motorDriver->status);
    EXPECT_FALSE(controller.motorDriver->forward);
    EXPECT_EQ(controller.motorDriver->moveDirection, Direction::FRONT);
}

TEST_F(ControllerStartButtonPressedTest, ChargingBlocksStartCleaningInRealStandbyMode)
{
    // TC-05
    // 목적: 충전 중이면 SD-02 opt 조건으로 실제 StartCleaning까지 도달하지 않는지 확인
    // 상황: currentMode는 실제 StandbyMode, 로봇은 충전 중
    // 실행: Driver가 Start Button 입력 전달
    // 기대값: CleanerDriver mode는 초기값 off 그대로 유지
    setRealMode(new StandbyMode());
    setCharging(true);

    driver.pressStartButton();

    EXPECT_EQ(controller.cleanerDriver->mode, "off");
}

TEST_F(ControllerStartButtonPressedTest, ChargingKeepsCurrentModeInRealStandbyMode)
{
    // TC-06
    // 목적: 충전 중 차단 시 실제 currentMode가 변경되지 않는지 확인
    // 상황: currentMode는 실제 StandbyMode, 로봇은 충전 중
    // 실행: Driver가 Start Button 입력 전달
    // 기대값: currentMode 포인터는 기존 StandbyMode 객체 그대로 유지
    auto *mode = setRealMode(new StandbyMode());
    setCharging(true);

    driver.pressStartButton();

    EXPECT_EQ(controller.currentMode, mode);
}

TEST_F(ControllerStartButtonPressedTest, DelegatesToCurrentModeOnceWhenNotCharging)
{
    // TC-07
    // 목적: Controller가 SD-02의 2번 메시지를 정확히 1회 호출하는지 확인
    // 상황: 호출 횟수는 실제 모드에서 관찰하기 어려워 stub mode 사용, 로봇은 충전 중 아님
    // 실행: Driver가 Start Button 입력 1회 전달
    // 기대값: stub mode의 startButtonPressed 호출 1회
    auto *mode = setStubMode(new StubOperatingMode());
    setCharging(false);

    driver.pressStartButton();

    EXPECT_EQ(mode->startButtonPressedCallCount, 1);
}

TEST_F(ControllerStartButtonPressedTest, DoesNotDelegateToCurrentModeWhenCharging)
{
    // TC-08
    // 목적: 충전 중이면 SD-02의 2번 메시지 호출 자체가 없는지 확인
    // 상황: 호출 여부는 실제 모드에서 직접 관찰하기 어려워 stub mode 사용, 로봇은 충전 중
    // 실행: Driver가 Start Button 입력 전달
    // 기대값: stub mode의 startButtonPressed 호출 0회
    auto *mode = setStubMode(new StubOperatingMode());
    setCharging(true);

    driver.pressStartButton();

    EXPECT_EQ(mode->startButtonPressedCallCount, 0);
}

TEST_F(ControllerStartButtonPressedTest, PassesControllerMotorDriverToCurrentMode)
{
    // TC-09
    // 목적: SD-02의 2번 메시지 인자 motor가 Controller의 MotorDriver인지 확인
    // 상황: 전달 인자 주소는 실제 StandbyMode에서 관찰하기 어려워 stub mode 사용
    // 실행: Driver가 Start Button 입력 전달
    // 기대값: stub이 받은 motor 주소와 controller.motorDriver 주소 일치
    auto *mode = setStubMode(new StubOperatingMode());
    setCharging(false);

    driver.pressStartButton();

    EXPECT_EQ(mode->receivedMotor, controller.motorDriver);
}

TEST_F(ControllerStartButtonPressedTest, PassesControllerCleanerDriverToCurrentMode)
{
    // TC-10
    // 목적: SD-02의 2번 메시지 인자 cleaner가 Controller의 CleanerDriver인지 확인
    // 상황: 전달 인자 주소는 실제 StandbyMode에서 관찰하기 어려워 stub mode 사용
    // 실행: Driver가 Start Button 입력 전달
    // 기대값: stub이 받은 cleaner 주소와 controller.cleanerDriver 주소 일치
    auto *mode = setStubMode(new StubOperatingMode());
    setCharging(false);

    driver.pressStartButton();

    EXPECT_EQ(mode->receivedCleaner, controller.cleanerDriver);
}

TEST_F(ControllerStartButtonPressedTest, ChangesCurrentModeToReturnedMode)
{
    // TC-11
    // 목적: SD-02의 currentMode = startButtonPressed(...) 대입 동작 확인
    // 상황: 반환 모드를 임의로 제어해야 해서 stub mode 사용
    // 실행: firstMode가 secondMode를 반환하도록 설정 후 Start Button 입력
    // 기대값: controller.currentMode는 secondMode
    auto *secondMode = new StubOperatingMode();
    auto *firstMode = setStubMode(new StubOperatingMode(secondMode));
    setCharging(false);

    driver.pressStartButton();

    EXPECT_EQ(controller.currentMode, secondMode);
    delete firstMode;
}

TEST_F(ControllerStartButtonPressedTest, KeepsCurrentModeWhenReturnedModeIsSameStub)
{
    // TC-12
    // 목적: 반환 모드가 자기 자신일 때 currentMode 유지 확인
    // 상황: 반환값 제어가 필요해서 stub mode 사용
    // 실행: stub mode가 자기 자신을 반환하도록 설정 후 Start Button 입력
    // 기대값: controller.currentMode는 기존 stub mode
    auto *mode = setStubMode(new StubOperatingMode());
    setCharging(false);

    driver.pressStartButton();

    EXPECT_EQ(controller.currentMode, mode);
}

TEST_F(ControllerStartButtonPressedTest, SecondPressUsesReturnedCurrentMode)
{
    // TC-13
    // 목적: 첫 입력에서 바뀐 currentMode가 다음 입력에 사용되는지 확인
    // 상황: 모드 전환과 호출 횟수 관찰이 필요해서 stub mode 사용
    // 실행: firstMode가 secondMode를 반환하도록 설정 후 Start Button 입력 2회
    // 기대값: firstMode 호출 1회, secondMode 호출 1회, currentMode는 secondMode
    auto *secondMode = new StubOperatingMode();
    auto *firstMode = setStubMode(new StubOperatingMode(secondMode));
    setCharging(false);

    driver.pressStartButton();
    driver.pressStartButton();

    EXPECT_EQ(firstMode->startButtonPressedCallCount, 1);
    EXPECT_EQ(secondMode->startButtonPressedCallCount, 1);
    EXPECT_EQ(controller.currentMode, secondMode);
    delete firstMode;
}

TEST_F(ControllerStartButtonPressedTest, CallsModeOncePerButtonPressWhenNotCharging)
{
    // TC-14
    // 목적: 반복 입력 시 SD-02의 2번 메시지가 입력 횟수만큼 호출되는지 확인
    // 상황: 호출 횟수 관찰이 필요해서 stub mode 사용, 로봇은 충전 중 아님
    // 실행: Driver가 Start Button 입력 2회 전달
    // 기대값: stub mode의 startButtonPressed 호출 총 2회
    auto *mode = setStubMode(new StubOperatingMode());
    setCharging(false);

    driver.pressStartButton();
    driver.pressStartButton();

    EXPECT_EQ(mode->startButtonPressedCallCount, 2);
}

TEST_F(ControllerStartButtonPressedTest, ChargingStateCanBlockAfterSuccessfulPress)
{
    // TC-15
    // 목적: 실행 중 충전 상태로 바뀐 뒤 다음 Start Button 입력 차단 확인
    // 상황: 호출 횟수 관찰이 필요해서 stub mode 사용, 첫 입력 후 충전 상태로 변경
    // 실행: Start Button 입력 1회 성공 후 다시 입력
    // 기대값: 첫 입력만 모드 호출, 두 번째 입력은 차단
    auto *mode = setStubMode(new StubOperatingMode());
    setCharging(false);
    driver.pressStartButton();

    setCharging(true);
    driver.pressStartButton();

    EXPECT_EQ(mode->startButtonPressedCallCount, 1);
}

} // namespace
