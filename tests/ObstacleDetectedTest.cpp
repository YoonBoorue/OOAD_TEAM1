#include <array>

#include <gtest/gtest.h>

// SD-05의 system operation인 Controller::obstacleDetected(dir) 검증

// TC-01~TC-11: 실제 ObstacleProcessor, NormalMode/BoostMode, MotorDriver 구현으로 SD-05 내부 동작 확인
// TC-12~TC-13: 실제 구현만으로 관찰하기 어려운 checkIsMoving 호출 방향/인자만 Stub 사용

// 현재 Controller 생성자는 currentMode 초기화가 없어서 테스트에서만 내부 상태 세팅
#define private public
#include "rvc/Controller.hpp"
#include "rvc/MotorDriver.hpp"
#undef private

#include "rvc/CleanerDriver.hpp"
#include "rvc/Modes.hpp"
#include "rvc/OperatingMode.hpp"

using namespace rvc;

namespace
{

using ObstacleDirection = std::array<bool, 3>; // {front, left, right}, true는 장애물 있음

// Stub: 실제 모드 구현만으로 확인하기 어려운 checkIsMoving 호출 정보 기록용
class StubOperatingMode : public OperatingMode
{
public:
    mutable int checkIsMovingCallCount = 0; // stub 기록값: checkIsMoving 호출 횟수
    mutable Direction receivedDirection = Direction::FRONT; // stub 기록값: ObstacleProcessor가 결정한 방향
    mutable MotorDriver *receivedMotor = nullptr; // stub 기록값: Controller가 전달한 MotorDriver 주소

    void checkIsMoving(Direction direction, MotorDriver &motor) const override
    {
        ++checkIsMovingCallCount; // stub 기록: OperatingMode까지 호출 도달 여부 확인
        receivedDirection = direction; // stub 기록: 결정된 회피 방향 확인
        receivedMotor = &motor; // stub 기록: motor 인자 확인
    }

    OperatingMode &startButtonPressed(MotorDriver &, CleanerDriver &) override { return *this; } // stub: 미사용
    OperatingMode &lowBatteryDetected(MotorDriver &, CleanerDriver &) override { return *this; } // stub: 미사용
    OperatingMode &lowBatteryCleared() override { return *this; } // stub: 미사용
    OperatingMode &dustDetected(CleanerDriver &) override { return *this; } // stub: 미사용
    bool canCharge() const override { return false; } // stub: 미사용
    OperatingMode &timerExpired(CleanerDriver &) override { return *this; } // stub: 미사용
};

// Driver: Obstacle Sensor Driver actor 대신 system operation 호출
class ObstacleDetectedDriver
{
public:
    explicit ObstacleDetectedDriver(Controller &controller) : controller(controller) {}

    void detectObstacle(const ObstacleDirection &direction)
    {
        controller.obstacleDetected(direction);
    }

private:
    Controller &controller;
};

class ControllerObstacleDetectedTest : public testing::Test
{
protected:
    Controller controller;
    ObstacleDetectedDriver driver{controller};

    OperatingMode *setRealMode(OperatingMode *mode)
    {
        controller.currentMode = mode; // 실제 구현 주입: SD 내부 ref 동작 관찰용
        return mode;
    }

    StubOperatingMode *setStubMode(StubOperatingMode *mode)
    {
        controller.currentMode = mode; // stub 주입: checkIsMoving 호출 정보 관찰용
        return mode;
    }
};

TEST_F(ControllerObstacleDetectedTest, NormalModeTurnsLeftAndMovesForwardWhenFrontBlockedAndLeftOpen)
{
    // TC-01
    // 목적: UC5 좌회전 우선 규칙과 SD-05의 Turn Left + Move Forward 확인
    // 상황: currentMode는 실제 NormalMode, front 장애물 있음, left/right 열림
    // 실행: Obstacle Sensor 입력 {front=T, left=F, right=F}
    // 기대값: MotorDriver 방향 LEFT, forward true, status true
    setRealMode(new NormalMode());
    controller.motorDriver->moveForward();

    driver.detectObstacle({true, false, false});

    EXPECT_EQ(controller.motorDriver->moveDirection, Direction::LEFT);
    EXPECT_TRUE(controller.motorDriver->forward);
    EXPECT_TRUE(controller.motorDriver->status);
}

TEST_F(ControllerObstacleDetectedTest, NormalModeTurnsLeftWhenLeftOpenEvenIfRightBlocked)
{
    // TC-02
    // 목적: UC5 좌회전 우선 규칙 확인
    // 상황: currentMode는 실제 NormalMode, front/right 장애물 있음, left 열림
    // 실행: Obstacle Sensor 입력 {front=T, left=F, right=T}
    // 기대값: MotorDriver 방향 LEFT, forward true
    setRealMode(new NormalMode());
    controller.motorDriver->moveForward();

    driver.detectObstacle({true, false, true});

    EXPECT_EQ(controller.motorDriver->moveDirection, Direction::LEFT);
    EXPECT_TRUE(controller.motorDriver->forward);
}

TEST_F(ControllerObstacleDetectedTest, NormalModeTurnsRightWhenLeftBlockedAndRightOpen)
{
    // TC-03
    // 목적: SD-05의 Turn Right + Move Forward 확인
    // 상황: currentMode는 실제 NormalMode, front/left 장애물 있음, right 열림
    // 실행: Obstacle Sensor 입력 {front=T, left=T, right=F}
    // 기대값: MotorDriver 방향 RIGHT, forward true
    setRealMode(new NormalMode());
    controller.motorDriver->moveForward();

    driver.detectObstacle({true, true, false});

    EXPECT_EQ(controller.motorDriver->moveDirection, Direction::RIGHT);
    EXPECT_TRUE(controller.motorDriver->forward);
}

TEST_F(ControllerObstacleDetectedTest, NormalModeMovesBackwardWhenFrontLeftRightBlocked)
{
    // TC-04
    // 목적: SD-05의 Move Backward 확인
    // 상황: currentMode는 실제 NormalMode, front/left/right 모두 장애물 있음
    // 실행: Obstacle Sensor 입력 {front=T, left=T, right=T}
    // 기대값: MotorDriver forward false
    setRealMode(new NormalMode());
    controller.motorDriver->moveForward();

    driver.detectObstacle({true, true, true});

    EXPECT_FALSE(controller.motorDriver->forward);
}

TEST_F(ControllerObstacleDetectedTest, BoostModeTurnsLeftAndMovesForwardWhenFrontBlockedAndLeftOpen)
{
    // TC-05
    // 목적: UC5 Precondition의 BoostMode에서도 좌회전 회피 수행 확인
    // 상황: currentMode는 실제 BoostMode, front 장애물 있음, left/right 열림
    // 실행: Obstacle Sensor 입력 {front=T, left=F, right=F}
    // 기대값: MotorDriver 방향 LEFT, forward true
    setRealMode(new BoostMode());
    controller.motorDriver->moveForward();

    driver.detectObstacle({true, false, false});

    EXPECT_EQ(controller.motorDriver->moveDirection, Direction::LEFT);
    EXPECT_TRUE(controller.motorDriver->forward);
}

TEST_F(ControllerObstacleDetectedTest, BoostModeTurnsRightWhenLeftBlockedAndRightOpen)
{
    // TC-06
    // 목적: BoostMode에서도 SD-05의 Turn Right + Move Forward 수행 확인
    // 상황: currentMode는 실제 BoostMode, front/left 장애물 있음, right 열림
    // 실행: Obstacle Sensor 입력 {front=T, left=T, right=F}
    // 기대값: MotorDriver 방향 RIGHT, forward true
    setRealMode(new BoostMode());
    controller.motorDriver->moveForward();

    driver.detectObstacle({true, true, false});

    EXPECT_EQ(controller.motorDriver->moveDirection, Direction::RIGHT);
    EXPECT_TRUE(controller.motorDriver->forward);
}

TEST_F(ControllerObstacleDetectedTest, BoostModeMovesBackwardWhenFrontLeftRightBlocked)
{
    // TC-07
    // 목적: BoostMode에서도 SD-05의 Move Backward 수행 확인
    // 상황: currentMode는 실제 BoostMode, front/left/right 모두 장애물 있음
    // 실행: Obstacle Sensor 입력 {front=T, left=T, right=T}
    // 기대값: MotorDriver forward false
    setRealMode(new BoostMode());
    controller.motorDriver->moveForward();

    driver.detectObstacle({true, true, true});

    EXPECT_FALSE(controller.motorDriver->forward);
}

TEST_F(ControllerObstacleDetectedTest, StandbyModeDoesNotMoveBecauseUseCasePreconditionNotMet)
{
    // TC-08
    // 목적: UC5 Precondition이 아닌 StandbyMode에서는 실제 MotorDriver 명령 없음 확인
    // 상황: currentMode는 실제 StandbyMode, front 장애물 있음, left 열림
    // 실행: Obstacle Sensor 입력 {front=T, left=F, right=F}
    // 기대값: MotorDriver 방향 FRONT 유지, forward false, status false
    setRealMode(new StandbyMode());

    driver.detectObstacle({true, false, false});

    EXPECT_EQ(controller.motorDriver->moveDirection, Direction::FRONT);
    EXPECT_FALSE(controller.motorDriver->forward);
    EXPECT_FALSE(controller.motorDriver->status);
}

TEST_F(ControllerObstacleDetectedTest, LowBatteryModeDoesNotMoveBecauseUseCasePreconditionNotMet)
{
    // TC-09
    // 목적: UC5 Precondition이 아닌 LowBatteryMode에서는 실제 MotorDriver 명령 없음 확인
    // 상황: currentMode는 실제 LowBatteryMode, front 장애물 있음, left 열림
    // 실행: Obstacle Sensor 입력 {front=T, left=F, right=F}
    // 기대값: MotorDriver 방향 FRONT 유지, forward false, status false
    setRealMode(new LowBatteryMode());

    driver.detectObstacle({true, false, false});

    EXPECT_EQ(controller.motorDriver->moveDirection, Direction::FRONT);
    EXPECT_FALSE(controller.motorDriver->forward);
    EXPECT_FALSE(controller.motorDriver->status);
}

TEST_F(ControllerObstacleDetectedTest, NormalModeTurnsLeftAfterBackwardEscapeWhenLeftOpen)
{
    // TC-10
    // 목적: UC5의 "모두 막힘 후 후진, 좌/우 중 열린 방향으로 회피" 흐름 중 left open 확인
    // 상황: currentMode는 실제 NormalMode, 후진 상태에서 left 열림
    // 실행: Obstacle Sensor 입력 {front=T, left=F, right=T}
    // 기대값: MotorDriver 방향 LEFT, forward true
    setRealMode(new NormalMode());
    controller.motorDriver->moveBackward();

    driver.detectObstacle({true, false, true});

    EXPECT_EQ(controller.motorDriver->moveDirection, Direction::LEFT);
    EXPECT_TRUE(controller.motorDriver->forward);
}

TEST_F(ControllerObstacleDetectedTest, NormalModeTurnsRightAfterBackwardEscapeWhenRightOpen)
{
    // TC-11
    // 목적: UC5의 "모두 막힘 후 후진, 좌/우 중 열린 방향으로 회피" 흐름 중 right open 확인
    // 상황: currentMode는 실제 NormalMode, 후진 상태에서 left 막힘, right 열림
    // 실행: Obstacle Sensor 입력 {front=T, left=T, right=F}
    // 기대값: MotorDriver 방향 RIGHT, forward true
    setRealMode(new NormalMode());
    controller.motorDriver->moveBackward();

    driver.detectObstacle({true, true, false});

    EXPECT_EQ(controller.motorDriver->moveDirection, Direction::RIGHT);
    EXPECT_TRUE(controller.motorDriver->forward);
}

TEST_F(ControllerObstacleDetectedTest, DelegatesDecidedLeftDirectionToOperatingMode)
{
    // TC-12
    // 목적: Controller -> ObstacleProcessor -> OperatingMode.checkIsMoving 연결 확인
    // 상황: 실제 모드 명령 결과만으로 checkIsMoving 인자를 직접 보기 어려워 stub mode 사용
    // 실행: Obstacle Sensor 입력 {front=T, left=F, right=F}
    // 기대값: checkIsMoving 호출 1회, 결정 방향 LEFT, 전달 motor 주소 일치
    auto *mode = setStubMode(new StubOperatingMode());

    driver.detectObstacle({true, false, false});

    EXPECT_EQ(mode->checkIsMovingCallCount, 1);
    EXPECT_EQ(mode->receivedDirection, Direction::LEFT);
    EXPECT_EQ(mode->receivedMotor, controller.motorDriver);
}

TEST_F(ControllerObstacleDetectedTest, DelegatesBackDirectionToOperatingModeWhenEverySideBlocked)
{
    // TC-13
    // 목적: 모든 방향이 막힌 경우 ObstacleProcessor가 BACK을 OperatingMode에 전달하는지 확인
    // 상황: 실제 MotorDriver 결과만으로 전달 direction을 직접 보기 어려워 stub mode 사용
    // 실행: Obstacle Sensor 입력 {front=T, left=T, right=T}
    // 기대값: checkIsMoving 호출 1회, 결정 방향 BACK
    auto *mode = setStubMode(new StubOperatingMode());

    driver.detectObstacle({true, true, true});

    EXPECT_EQ(mode->checkIsMovingCallCount, 1);
    EXPECT_EQ(mode->receivedDirection, Direction::BACK);
}

} // namespace
