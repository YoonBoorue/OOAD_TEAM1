#include <gtest/gtest.h>

#include "rvc/CleanerDriver.hpp"
#include "rvc/Direction.hpp"
#include "rvc/Modes.hpp"
#include "rvc/MotorDriver.hpp"

#define private public
#include "rvc/Controller.hpp"
#undef private

using namespace rvc;

// =====================================================
// Controller::obstacleDetected()
// SD-05 Avoid Obstacle System Operation Tests
//
// TC-01~TC-07: 실제 NormalMode, ObstacleProcessor, MotorDriver 구현으로 SD-05 내부 동작 확인
// TC-08~TC-14: 실제 BoostMode, ObstacleProcessor, MotorDriver 구현으로 SD-05 내부 동작 확인
// TC-15~TC-18: Off/Standby/LowBattery/null 입력처럼 이동이 발생하지 않아야 하는 경계 확인
// TC-19~TC-21: 실제 구현만으로 관찰하기 어려운 위임 횟수, 전달 인자, MotorDriver 참조만 Stub 사용
//
// 현재 Controller 생성자는 currentMode를 nullptr로 시작하므로 테스트에서 필요한 모드는
// system operation 또는 테스트용 내부 상태 세팅으로 준비
// =====================================================

namespace
{

    void sendObstacleDetected(Controller &controller, bool front, bool left, bool right)
    {
        const bool direction[3] = {front, left, right};
        controller.obstacleDetected(direction);
    }

    Direction motorDirection(const Controller &controller)
    {
        return controller.motorDriver->currentDirection();
    }

    void enterNormalMode(Controller &controller)
    {
        controller.powerButtonPressed();
        controller.startButtonPressed();
    }

    void enterBoostMode(Controller &controller)
    {
        controller.powerButtonPressed();
        controller.startButtonPressed();
        controller.currentMode = &boostMode();
        controller.cleanerDriver->decideSetting(true);
    }

    void enterBackwardEscapeState(Controller &controller)
    {
        sendObstacleDetected(controller, true, true, true);
    }

    class StubOperatingMode final : public OperatingMode
    {
    public:
        mutable int checkIsMovingCallCount = 0;
        mutable Direction lastDirection = Direction::FRONT;
        mutable MotorDriver *lastMotorDriver = nullptr;

        void checkIsMoving(Direction direction, MotorDriver &motorDriver) const override
        {
            ++checkIsMovingCallCount;
            lastDirection = direction;
            lastMotorDriver = &motorDriver;
        }

        OperatingMode &startButtonPressed(CleanerDriver &, MotorDriver &) override { return *this; }
        OperatingMode &lowBatteryDetected(CleanerDriver &, MotorDriver &) override { return *this; }
        OperatingMode &lowBatteryCleared() override { return *this; }
        OperatingMode &dustDetected(CleanerDriver &) override { return *this; }
        bool canCharge() const override { return false; }
        OperatingMode &timerExpired(CleanerDriver &) override { return *this; }
        ModeKind kind() const override { return ModeKind::Normal; }
        const char *name() const override { return "StubOperatingMode"; }
    };

    void attachStubMode(Controller &controller, StubOperatingMode &stubMode)
    {
        controller.power = true;
        controller.currentMode = &stubMode;
        controller.motorDriver->initialize();
    }

} // namespace

// TC-01
// 목적: SD-05에서 전진 중 전방이 비어 있으면 UC3 Move Forward가 유지되는지 확인
// 상황: currentMode는 실제 NormalMode, MotorDriver는 FRONT 방향으로 전진 중, front=false, left=true, right=true
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: MotorDriver direction은 FRONT이고 forward 상태 유지
TEST(ControllerObstacleDetectedTest, NormalModeFrontClearKeepsMovingForward)
{
    Controller controller;
    enterNormalMode(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    ASSERT_TRUE(controller.isMotorForward());
    ASSERT_EQ(motorDirection(controller), Direction::FRONT);

    sendObstacleDetected(controller, false, true, true);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_TRUE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::FRONT);
}

// TC-02
// 목적: SD-05 내부 alt [direction = LEFT]에서 UC12 Turn Left와 UC3 Move Forward가 수행되는지 확인
// 상황: currentMode는 실제 NormalMode, MotorDriver는 FRONT 방향으로 전진 중, front=true, left=false, right=false
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: MotorDriver direction은 LEFT이고 forward 상태 유지
TEST(ControllerObstacleDetectedTest, NormalModeForwardFrontBlockedTurnsLeft)
{
    Controller controller;
    enterNormalMode(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    ASSERT_TRUE(controller.isMotorForward());
    ASSERT_EQ(motorDirection(controller), Direction::FRONT);

    sendObstacleDetected(controller, true, false, false);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_TRUE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::LEFT);
}

// TC-03
// 목적: SD-05 내부 alt [direction = RIGHT]에서 UC13 Turn Right와 UC3 Move Forward가 수행되는지 확인
// 상황: currentMode는 실제 NormalMode, MotorDriver는 FRONT 방향으로 전진 중, front=true, left=true, right=false
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: MotorDriver direction은 RIGHT이고 forward 상태 유지
TEST(ControllerObstacleDetectedTest, NormalModeForwardFrontAndLeftBlockedTurnsRight)
{
    Controller controller;
    enterNormalMode(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    ASSERT_TRUE(controller.isMotorForward());
    ASSERT_EQ(motorDirection(controller), Direction::FRONT);

    sendObstacleDetected(controller, true, true, false);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_TRUE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::RIGHT);
}

// TC-04
// 목적: SD-05 내부 alt [direction = BACK]에서 UC14 Move Backward가 수행되는지 확인
// 상황: currentMode는 실제 NormalMode, MotorDriver는 전진 중, front=true, left=true, right=true
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: MotorDriver는 moving 상태이지만 forward는 false
TEST(ControllerObstacleDetectedTest, NormalModeForwardAllBlockedMovesBackward)
{
    Controller controller;
    enterNormalMode(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    ASSERT_TRUE(controller.isMotorForward());

    sendObstacleDetected(controller, true, true, true);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
}

// TC-05
// 목적: SD-05에서 후진 탈출 중 왼쪽이 비어 있으면 LEFT 경로로 다시 전진하는지 확인
// 상황: currentMode는 실제 NormalMode, MotorDriver는 backward escape 상태, front=true, left=false, right=true
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: MotorDriver direction은 LEFT이고 forward는 true
TEST(ControllerObstacleDetectedTest, NormalModeBackwardLeftClearTurnsLeftAndMovesForward)
{
    Controller controller;
    enterNormalMode(controller);
    enterBackwardEscapeState(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    ASSERT_TRUE(controller.isMotorMoving());
    ASSERT_FALSE(controller.isMotorForward());

    sendObstacleDetected(controller, true, false, true);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_TRUE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::LEFT);
}

// TC-06
// 목적: SD-05에서 후진 탈출 중 오른쪽이 비어 있으면 RIGHT 경로로 다시 전진하는지 확인
// 상황: currentMode는 실제 NormalMode, MotorDriver는 backward escape 상태, front=true, left=true, right=false
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: MotorDriver direction은 RIGHT이고 forward는 true
TEST(ControllerObstacleDetectedTest, NormalModeBackwardRightClearTurnsRightAndMovesForward)
{
    Controller controller;
    enterNormalMode(controller);
    enterBackwardEscapeState(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    ASSERT_TRUE(controller.isMotorMoving());
    ASSERT_FALSE(controller.isMotorForward());

    sendObstacleDetected(controller, true, true, false);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_TRUE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::RIGHT);
}

// TC-07
// 목적: SD-05에서 후진 탈출 중 좌우가 모두 막혀 있으면 BACK 경로를 유지하는지 확인
// 상황: currentMode는 실제 NormalMode, MotorDriver는 backward escape 상태, front=false, left=true, right=true
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: MotorDriver는 moving 상태이지만 forward는 false
TEST(ControllerObstacleDetectedTest, NormalModeBackwardNoSideClearKeepsBackward)
{
    Controller controller;
    enterNormalMode(controller);
    enterBackwardEscapeState(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    ASSERT_TRUE(controller.isMotorMoving());
    ASSERT_FALSE(controller.isMotorForward());

    sendObstacleDetected(controller, false, true, true);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
}

// TC-08
// 목적: SD-05에서 BoostMode도 전방이 비어 있으면 UC3 Move Forward를 유지하는지 확인
// 상황: currentMode는 실제 BoostMode, CleanerDriver는 boost 청소 중, MotorDriver는 전진 중
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: currentMode와 CleanerDriver mode는 boost로 유지되고 MotorDriver direction은 FRONT
TEST(ControllerObstacleDetectedTest, BoostModeFrontClearKeepsMovingForward)
{
    Controller controller;
    enterBoostMode(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Boost);
    ASSERT_EQ(controller.cleanerMode(), "boost");
    ASSERT_TRUE(controller.isMotorForward());
    ASSERT_EQ(motorDirection(controller), Direction::FRONT);

    sendObstacleDetected(controller, false, true, true);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Boost);
    EXPECT_EQ(controller.cleanerMode(), "boost");
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_TRUE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::FRONT);
}

// TC-09
// 목적: SD-05 내부 alt [direction = LEFT]가 실제 BoostMode에서 수행되는지 확인
// 상황: currentMode는 실제 BoostMode, CleanerDriver는 boost 청소 중, front=true, left=false, right=false
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: currentMode와 CleanerDriver mode는 boost로 유지되고 MotorDriver direction은 LEFT
TEST(ControllerObstacleDetectedTest, BoostModeForwardFrontBlockedTurnsLeft)
{
    Controller controller;
    enterBoostMode(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Boost);
    ASSERT_EQ(controller.cleanerMode(), "boost");
    ASSERT_TRUE(controller.isMotorForward());

    sendObstacleDetected(controller, true, false, false);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Boost);
    EXPECT_EQ(controller.cleanerMode(), "boost");
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_TRUE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::LEFT);
}

// TC-10
// 목적: SD-05 내부 alt [direction = RIGHT]가 실제 BoostMode에서 수행되는지 확인
// 상황: currentMode는 실제 BoostMode, CleanerDriver는 boost 청소 중, front=true, left=true, right=false
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: currentMode와 CleanerDriver mode는 boost로 유지되고 MotorDriver direction은 RIGHT
TEST(ControllerObstacleDetectedTest, BoostModeForwardFrontAndLeftBlockedTurnsRight)
{
    Controller controller;
    enterBoostMode(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Boost);
    ASSERT_EQ(controller.cleanerMode(), "boost");
    ASSERT_TRUE(controller.isMotorForward());

    sendObstacleDetected(controller, true, true, false);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Boost);
    EXPECT_EQ(controller.cleanerMode(), "boost");
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_TRUE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::RIGHT);
}

// TC-11
// 목적: SD-05 내부 alt [direction = BACK]가 실제 BoostMode에서 수행되는지 확인
// 상황: currentMode는 실제 BoostMode, CleanerDriver는 boost 청소 중, front=true, left=true, right=true
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: currentMode와 CleanerDriver mode는 boost로 유지되고 MotorDriver forward는 false
TEST(ControllerObstacleDetectedTest, BoostModeForwardAllBlockedMovesBackward)
{
    Controller controller;
    enterBoostMode(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Boost);
    ASSERT_EQ(controller.cleanerMode(), "boost");
    ASSERT_TRUE(controller.isMotorForward());

    sendObstacleDetected(controller, true, true, true);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Boost);
    EXPECT_EQ(controller.cleanerMode(), "boost");
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
}

// TC-12
// 목적: SD-05에서 BoostMode 후진 탈출 중 왼쪽이 비어 있으면 LEFT 경로로 다시 전진하는지 확인
// 상황: currentMode는 실제 BoostMode, MotorDriver는 backward escape 상태, front=true, left=false, right=true
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: currentMode와 CleanerDriver mode는 boost로 유지되고 MotorDriver direction은 LEFT
TEST(ControllerObstacleDetectedTest, BoostModeBackwardLeftClearTurnsLeftAndMovesForward)
{
    Controller controller;
    enterBoostMode(controller);
    enterBackwardEscapeState(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Boost);
    ASSERT_EQ(controller.cleanerMode(), "boost");
    ASSERT_TRUE(controller.isMotorMoving());
    ASSERT_FALSE(controller.isMotorForward());

    sendObstacleDetected(controller, true, false, true);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Boost);
    EXPECT_EQ(controller.cleanerMode(), "boost");
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_TRUE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::LEFT);
}

// TC-13
// 목적: SD-05에서 BoostMode 후진 탈출 중 오른쪽이 비어 있으면 RIGHT 경로로 다시 전진하는지 확인
// 상황: currentMode는 실제 BoostMode, MotorDriver는 backward escape 상태, front=true, left=true, right=false
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: currentMode와 CleanerDriver mode는 boost로 유지되고 MotorDriver direction은 RIGHT
TEST(ControllerObstacleDetectedTest, BoostModeBackwardRightClearTurnsRightAndMovesForward)
{
    Controller controller;
    enterBoostMode(controller);
    enterBackwardEscapeState(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Boost);
    ASSERT_EQ(controller.cleanerMode(), "boost");
    ASSERT_TRUE(controller.isMotorMoving());
    ASSERT_FALSE(controller.isMotorForward());

    sendObstacleDetected(controller, true, true, false);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Boost);
    EXPECT_EQ(controller.cleanerMode(), "boost");
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_TRUE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::RIGHT);
}

// TC-14
// 목적: SD-05에서 BoostMode 후진 탈출 중 좌우가 모두 막혀 있으면 BACK 경로를 유지하는지 확인
// 상황: currentMode는 실제 BoostMode, MotorDriver는 backward escape 상태, front=false, left=true, right=true
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: currentMode와 CleanerDriver mode는 boost로 유지되고 MotorDriver forward는 false
TEST(ControllerObstacleDetectedTest, BoostModeBackwardNoSideClearKeepsBackward)
{
    Controller controller;
    enterBoostMode(controller);
    enterBackwardEscapeState(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Boost);
    ASSERT_EQ(controller.cleanerMode(), "boost");
    ASSERT_TRUE(controller.isMotorMoving());
    ASSERT_FALSE(controller.isMotorForward());

    sendObstacleDetected(controller, false, true, true);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Boost);
    EXPECT_EQ(controller.cleanerMode(), "boost");
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
}

// TC-15
// 목적: currentMode가 없으면 Controller::obstacleDetected()가 위임하지 않고 종료되는지 확인
// 상황: Controller는 전원 off, currentMode는 nullptr, front=true, left=true, right=true
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: currentMode는 없고 MotorDriver는 정지 상태 유지
TEST(ControllerObstacleDetectedTest, PowerOffObstacleDetectedDoesNothing)
{
    Controller controller;

    ASSERT_FALSE(controller.isPowerOn());
    ASSERT_FALSE(controller.hasCurrentMode());

    sendObstacleDetected(controller, true, true, true);

    EXPECT_FALSE(controller.isPowerOn());
    EXPECT_FALSE(controller.hasCurrentMode());
    EXPECT_FALSE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::FRONT);
}

// TC-16
// 목적: direction 포인터가 null이면 Controller::obstacleDetected()가 위임하지 않고 종료되는지 확인
// 상황: currentMode는 실제 NormalMode, MotorDriver는 FRONT 방향으로 전진 중, direction=nullptr
// 실행: Obstacle Sensor Driver가 null obstacleDetected 입력 전달
// 기대값: MotorDriver direction은 FRONT이고 forward 상태 유지
TEST(ControllerObstacleDetectedTest, NullDirectionDoesNothingInNormalMode)
{
    Controller controller;
    enterNormalMode(controller);

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Normal);
    ASSERT_TRUE(controller.isMotorForward());
    ASSERT_EQ(motorDirection(controller), Direction::FRONT);

    controller.obstacleDetected(nullptr);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Normal);
    EXPECT_TRUE(controller.isMotorMoving());
    EXPECT_TRUE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::FRONT);
}

// TC-17
// 목적: SD-05 opt [currentMode == Normal or Boost] 밖인 StandbyMode에서는 이동이 발생하지 않는지 확인
// 상황: currentMode는 실제 StandbyMode, MotorDriver는 정지 상태, front=true, left=false, right=false
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: currentMode는 StandbyMode이고 MotorDriver는 정지 상태 유지
TEST(ControllerObstacleDetectedTest, StandbyModeObstacleDetectedDoesNotMoveMotor)
{
    Controller controller;
    controller.powerButtonPressed();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::Standby);
    ASSERT_FALSE(controller.isMotorMoving());

    sendObstacleDetected(controller, true, false, false);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::Standby);
    EXPECT_FALSE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::FRONT);
}

// TC-18
// 목적: SD-05 opt [currentMode == Normal or Boost] 밖인 LowBatteryMode에서는 이동이 발생하지 않는지 확인
// 상황: currentMode는 실제 LowBatteryMode, MotorDriver는 정지 상태, front=true, left=false, right=false
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: currentMode는 LowBatteryMode이고 MotorDriver는 정지 상태 유지
TEST(ControllerObstacleDetectedTest, LowBatteryModeObstacleDetectedDoesNotMoveMotor)
{
    Controller controller;
    controller.powerButtonPressed();
    controller.lowBatteryDetected();

    ASSERT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    ASSERT_FALSE(controller.isMotorMoving());

    sendObstacleDetected(controller, true, false, false);

    EXPECT_EQ(controller.currentModeKind(), ModeKind::LowBattery);
    EXPECT_FALSE(controller.isMotorMoving());
    EXPECT_FALSE(controller.isMotorForward());
    EXPECT_EQ(motorDirection(controller), Direction::FRONT);
}

// TC-19
// 목적: Controller::obstacleDetected()가 ObstacleProcessor를 통해 currentMode.checkIsMoving()에 LEFT를 전달하는지 확인
// 상황: currentMode는 StubOperatingMode, MotorDriver는 전진 중, front=true, left=false, right=true
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: Stub의 checkIsMoving 호출 횟수는 1이고 전달 direction은 LEFT
TEST(ControllerObstacleDetectedTest, StubModeReceivesLeftDirectionWhenForwardAndFrontBlocked)
{
    Controller controller;
    StubOperatingMode stubMode;
    attachStubMode(controller, stubMode);
    controller.motorDriver->moveForward();

    sendObstacleDetected(controller, true, false, true);

    EXPECT_EQ(stubMode.checkIsMovingCallCount, 1);
    EXPECT_EQ(stubMode.lastDirection, Direction::LEFT);
    EXPECT_EQ(stubMode.lastMotorDriver, controller.motorDriver);
}

// TC-20
// 목적: Controller::obstacleDetected()가 후진 탈출 상태에서 RIGHT 전달 인자를 계산하는지 확인
// 상황: currentMode는 StubOperatingMode, MotorDriver는 backward escape 상태, front=false, left=true, right=false
// 실행: Obstacle Sensor Driver가 obstacleDetected(dir) 입력 전달
// 기대값: Stub의 checkIsMoving 호출 횟수는 1이고 전달 direction은 RIGHT
TEST(ControllerObstacleDetectedTest, StubModeReceivesRightDirectionWhenBackwardAndRightClear)
{
    Controller controller;
    StubOperatingMode stubMode;
    attachStubMode(controller, stubMode);
    controller.motorDriver->moveForward();
    controller.motorDriver->moveBackward();

    sendObstacleDetected(controller, false, true, false);

    EXPECT_EQ(stubMode.checkIsMovingCallCount, 1);
    EXPECT_EQ(stubMode.lastDirection, Direction::RIGHT);
    EXPECT_EQ(stubMode.lastMotorDriver, controller.motorDriver);
}

// TC-21
// 목적: direction 포인터가 null이면 Stub currentMode에도 위임하지 않는지 확인
// 상황: currentMode는 StubOperatingMode, MotorDriver는 전진 중, direction=nullptr
// 실행: Obstacle Sensor Driver가 null obstacleDetected 입력 전달
// 기대값: Stub의 checkIsMoving 호출 횟수는 0
TEST(ControllerObstacleDetectedTest, StubModeIsNotCalledWhenDirectionPointerIsNull)
{
    Controller controller;
    StubOperatingMode stubMode;
    attachStubMode(controller, stubMode);
    controller.motorDriver->moveForward();

    controller.obstacleDetected(nullptr);

    EXPECT_EQ(stubMode.checkIsMovingCallCount, 0);
    EXPECT_EQ(stubMode.lastMotorDriver, nullptr);
}
