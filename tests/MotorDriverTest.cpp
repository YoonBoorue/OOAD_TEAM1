#include <gtest/gtest.h>

#define private public
#include "rvc/MotorDriver.hpp"
#undef private

#include "rvc/Modes.hpp"

using namespace rvc;

/* moveForward(), stopMoving() Unit Test(7개))
 * TC 목록:
 * - TC-UC3-01: moveForward() 호출 시 전진 이동 상태가 되는지 확인
 * - TC-UC8-01: 이동 중 stopMoving() 호출 시 정지 상태가 되는지 확인
 * - TC-UC8-02: 이미 정지한 상태에서 stopMoving() 호출 시 계속 정지 상태인지 확인
 * - TC-SD05-NORMAL-LEFT: NormalMode에서 LEFT 입력 시 좌회전 후 전진하는지 확인
 * - TC-SD05-NORMAL-RIGHT: NormalMode에서 RIGHT 입력 시 우회전 후 전진하는지 확인
 * - TC-SD05-BOOST-LEFT: BoostMode에서 LEFT 입력 시 좌회전 후 전진하는지 확인
 * - TC-SD05-BOOST-RIGHT: BoostMode에서 RIGHT 입력 시 우회전 후 전진하는지 확인
 *
 * Driver:
 * - 각 TEST 함수가 테스트 Driver 역할을 한다.
 * - TEST 함수가 객체를 생성하고 테스트 대상 함수를 직접 호출한다.
 *
 * Stub:
 * - 별도의 Stub은 사용하지 않는다.
 * - 실제 MotorDriver 객체를 사용해서 상태 변화를 확인한다.
 */

TEST(MotorDriverTest, MoveForwardSetsMotorRunningForward)
{
    // TC-UC3-01: moveForward() 호출 시 전진 이동 상태가 되는지 확인
    MotorDriver motor;

    motor.moveForward();

    EXPECT_TRUE(motor.status);
    EXPECT_TRUE(motor.forward);
    EXPECT_EQ(Direction::FRONT, motor.moveDirection);
}

TEST(MotorDriverTest, StopMovingClearsRunningStatus)
{
    // TC-UC8-01: 이동 중 stopMoving() 호출 시 정지 상태가 되는지 확인
    MotorDriver motor;
    motor.moveForward();

    motor.stopMoving();

    EXPECT_FALSE(motor.status);
}

TEST(MotorDriverTest, StopMovingKeepsMotorStoppedWhenAlreadyStopped)
{
    // TC-UC8-02: 이미 정지한 상태에서 stopMoving() 호출 시 계속 정지 상태인지 확인
    MotorDriver motor;

    motor.stopMoving();

    EXPECT_FALSE(motor.status);
}

TEST(NormalModeMotorDriverTest, LeftDirectionTurnsLeftAndMovesForward)
{
    // TC-SD05-NORMAL-LEFT: NormalMode에서 LEFT 입력 시 좌회전 후 전진하는지 확인
    NormalMode mode;
    MotorDriver motor;

    mode.checkIsMoving(Direction::LEFT, motor);

    EXPECT_TRUE(motor.status);
    EXPECT_TRUE(motor.forward);
    EXPECT_EQ(Direction::LEFT, motor.moveDirection);
}

TEST(NormalModeMotorDriverTest, RightDirectionTurnsRightAndMovesForward)
{
    // TC-SD05-NORMAL-RIGHT: NormalMode에서 RIGHT 입력 시 우회전 후 전진하는지 확인
    NormalMode mode;
    MotorDriver motor;

    mode.checkIsMoving(Direction::RIGHT, motor);

    EXPECT_TRUE(motor.status);
    EXPECT_TRUE(motor.forward);
    EXPECT_EQ(Direction::RIGHT, motor.moveDirection);
}

TEST(BoostModeMotorDriverTest, LeftDirectionTurnsLeftAndMovesForward)
{
    // TC-SD05-BOOST-LEFT: BoostMode에서 LEFT 입력 시 좌회전 후 전진하는지 확인
    BoostMode mode;
    MotorDriver motor;

    mode.checkIsMoving(Direction::LEFT, motor);

    EXPECT_TRUE(motor.status);
    EXPECT_TRUE(motor.forward);
    EXPECT_EQ(Direction::LEFT, motor.moveDirection);
}

TEST(BoostModeMotorDriverTest, RightDirectionTurnsRightAndMovesForward)
{
    // TC-SD05-BOOST-RIGHT: BoostMode에서 RIGHT 입력 시 우회전 후 전진하는지 확인
    BoostMode mode;
    MotorDriver motor;

    mode.checkIsMoving(Direction::RIGHT, motor);

    EXPECT_TRUE(motor.status);
    EXPECT_TRUE(motor.forward);
    EXPECT_EQ(Direction::RIGHT, motor.moveDirection);
}
