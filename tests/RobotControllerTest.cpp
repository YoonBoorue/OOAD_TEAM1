#include "rvc/RobotController.hpp"

#include <gtest/gtest.h>

using namespace rvc;

TEST(RobotControllerTest, MovesForwardWhenNoObstacleExists)
{
    RobotController controller;

    SensorSnapshot sensors{};
    sensors.frontObstacle = false;
    sensors.leftObstacle = false;
    sensors.rightObstacle = false;
    sensors.dustDetected = false;

    const auto command = controller.decide(sensors);

    EXPECT_EQ(command.motion, MotionCommand::MoveForward);
    EXPECT_FALSE(command.cleaningPowerUp);
}

TEST(RobotControllerTest, TurnsLeftWhenFrontObstacleExistsAndLeftSideIsOpen)
{
    RobotController controller(Direction::LEFT);

    SensorSnapshot sensors{};
    sensors.frontObstacle = true;
    sensors.leftObstacle = false;
    sensors.rightObstacle = false;
    sensors.dustDetected = false;

    const auto command = controller.decide(sensors);

    EXPECT_EQ(command.motion, MotionCommand::TurnLeft);
}

TEST(RobotControllerTest, TurnsRightWhenFrontAndLeftAreBlocked)
{
    RobotController controller(Direction::LEFT);

    SensorSnapshot sensors{};
    sensors.frontObstacle = true;
    sensors.leftObstacle = true;
    sensors.rightObstacle = false;
    sensors.dustDetected = false;

    const auto command = controller.decide(sensors);

    EXPECT_EQ(command.motion, MotionCommand::TurnRight);
}

TEST(RobotControllerTest, MovesBackwardWhenFrontLeftRightAreBlocked)
{
    RobotController controller;

    SensorSnapshot sensors{};
    sensors.frontObstacle = true;
    sensors.leftObstacle = true;
    sensors.rightObstacle = true;
    sensors.dustDetected = false;

    const auto command = controller.decide(sensors);

    EXPECT_EQ(command.motion, MotionCommand::MoveBackward);
}

TEST(RobotControllerTest, PowersUpCleaningWhenDustIsDetected)
{
    RobotController controller;

    SensorSnapshot sensors{};
    sensors.frontObstacle = false;
    sensors.leftObstacle = false;
    sensors.rightObstacle = false;
    sensors.dustDetected = true;

    const auto command = controller.decide(sensors);

    EXPECT_EQ(command.motion, MotionCommand::MoveForward);
    EXPECT_TRUE(command.cleaningPowerUp);
}