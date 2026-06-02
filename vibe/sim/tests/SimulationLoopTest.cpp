#include "sim/MapLoader.hpp"
#include "sim/Renderer.hpp"
#include "sim/RvcAdapter.hpp"
#include "sim/SimulationLoop.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace
{

sim::SimulationLoop::KeyPoller noKeys()
{
    return []() -> std::optional<char> {
        return std::nullopt;
    };
}

class RecordingRenderer final : public sim::Renderer
{
public:
    void render(const sim::RenderFrame &frame) override
    {
        lastFrame = frame;
    }

    sim::RenderFrame lastFrame;
};

sim::Scenario makeScenarioWithBattery(int batteryLevel)
{
    sim::Room room(3, 3);
    room.setDust(sim::Position{2, 2});
    return sim::Scenario{"battery-test", sim::Environment(std::move(room), sim::Pose{{1, 1}}, batteryLevel)};
}

} // namespace

TEST(SimulationLoopTest, FiftyTickScenarioCleansAtLeastOneCell)
{
    std::istringstream map(
        "5 4\n"
        ".*...\n"
        ".R...\n"
        "..#..\n"
        ".....\n");

    sim::NullRenderer renderer;
    sim::SimulationLoop simulation(sim::loadScenarioFromMap(map, "test-map"), renderer, noKeys(), 0);
    simulation.runForTicks(50);

    EXPECT_GE(simulation.cleanedCells(), 1);
}

TEST(SimulationLoopTest, ScriptedPowerAndStartKeysKeepSimulatorActive)
{
    std::istringstream map(
        "5 4\n"
        ".*...\n"
        ".R...\n"
        "..#..\n"
        ".....\n");

    std::vector<std::optional<char>> keys = {'p', 's'};
    std::size_t index = 0;
    auto poller = [&keys, &index]() -> std::optional<char> {
        if (index >= keys.size())
        {
            return std::nullopt;
        }

        return keys[index++];
    };

    sim::NullRenderer renderer;
    sim::SimulationLoop simulation(sim::loadScenarioFromMap(map, "scripted-map"), renderer, poller, 0, false);
    simulation.runForTicks(50);

    // [변경] scripted key path verifies the simulator remains active after manual power/start.
    EXPECT_GT(simulation.tickCount(), 1);
    EXPECT_NE(simulation.modeName(), "StandbyMode");
}

TEST(SimulationLoopTest, ChargingKeyInStandbyUpdatesEnvironmentBattery)
{
    std::vector<std::optional<char>> keys = {'p', 'c', std::nullopt};
    std::size_t index = 0;
    auto poller = [&keys, &index]() -> std::optional<char> {
        if (index >= keys.size())
        {
            return std::nullopt;
        }

        return keys[index++];
    };

    RecordingRenderer renderer;
    sim::SimulationLoop simulation(makeScenarioWithBattery(50), renderer, poller, 0, false);

    simulation.runForTicks(2);

    EXPECT_EQ(simulation.batteryLevel(), 60);
    EXPECT_TRUE(renderer.lastFrame.chargingActive);

    simulation.runForTicks(1);

    EXPECT_EQ(simulation.batteryLevel(), 70);
    EXPECT_TRUE(renderer.lastFrame.chargingActive);
}

TEST(SimulationLoopTest, ChargingKeyWhileOffUsesEnvironmentBattery)
{
    std::vector<std::optional<char>> keys = {'c'};
    std::size_t index = 0;
    auto poller = [&keys, &index]() -> std::optional<char> {
        if (index >= keys.size())
        {
            return std::nullopt;
        }

        return keys[index++];
    };

    RecordingRenderer renderer;
    sim::SimulationLoop simulation(makeScenarioWithBattery(50), renderer, poller, 0, false);

    simulation.runForTicks(1);

    EXPECT_EQ(simulation.batteryLevel(), 60);
    EXPECT_TRUE(renderer.lastFrame.chargingActive);
    EXPECT_EQ(renderer.lastFrame.modeName, "Off");
}

TEST(SimulationLoopTest, ChargingKeyWithAutoStartRejectsChargingDuringCleaning)
{
    std::vector<std::optional<char>> keys = {'c'};
    std::size_t index = 0;
    auto poller = [&keys, &index]() -> std::optional<char> {
        if (index >= keys.size())
        {
            return std::nullopt;
        }

        return keys[index++];
    };

    RecordingRenderer renderer;
    sim::SimulationLoop simulation(makeScenarioWithBattery(50), renderer, poller, 0);

    simulation.runForTicks(1);

    // [변경] autoStart cleaning 상태에서 charging key는 충전을 시작하지 않고 청소 tick의 배터리 소모만 반영한다.
    EXPECT_FALSE(renderer.lastFrame.chargingActive);
    EXPECT_EQ(simulation.batteryLevel(), 48);
    EXPECT_EQ(simulation.modeName(), "NormalMode");
}

TEST(SimulationLoopTest, BackwardActuatorMovesRobotBackward)
{
    sim::Room room(3, 3);
    sim::Environment environment(std::move(room), sim::Pose{{1, 1}}, 100);
    sim::ActuatorSnapshot actuators;
    actuators.motorMoving = true;
    actuators.motorForward = false;
    actuators.motorDirection = rvc::Direction::Backward;

    environment.applyActuators(actuators);

    EXPECT_EQ(environment.pose().position.x, 1);
    EXPECT_EQ(environment.pose().position.y, 2);
}

TEST(SimulationLoopTest, RvcAdapterMarksBackwardCommandAsNotForward)
{
    sim::RvcAdapter adapter;
    adapter.powerOnAndStart();

    sim::SensorSnapshot sensors;
    // [변경] all-blocked scenario uses front/left input plus front recheck states.
    sensors.obstacleBlocked = {true, true};
    sensors.frontAfterRightTurnBlocked = true;
    sensors.leftAfterBackwardBlocked = true;
    sensors.frontAfterBackwardRightCheckBlocked = true;
    adapter.feedSensors(sensors);

    const sim::ActuatorSnapshot actuators = adapter.actuators();
    EXPECT_TRUE(actuators.motorMoving);
    EXPECT_FALSE(actuators.motorForward);
    EXPECT_EQ(actuators.motorDirection, rvc::Direction::Backward);
}

TEST(SimulationLoopTest, RvcAdapterMapsRightRecheckClearToRightMovement)
{
    sim::RvcAdapter adapter;
    adapter.powerOnAndStart();

    sim::SensorSnapshot sensors;
    // [추가] front/left blocked 후 right path clear는 map 좌표상 오른쪽 이동으로 표현한다.
    sensors.obstacleBlocked = {true, true};
    sensors.frontAfterRightTurnBlocked = false;
    sensors.leftAfterBackwardBlocked = true;
    sensors.frontAfterBackwardRightCheckBlocked = true;
    adapter.feedSensors(sensors);

    const sim::ActuatorSnapshot actuators = adapter.actuators();
    EXPECT_TRUE(actuators.motorMoving);
    EXPECT_TRUE(actuators.motorForward);
    EXPECT_EQ(actuators.motorDirection, rvc::Direction::Right);
    EXPECT_EQ(adapter.heading(), rvc::Direction::Right);
}

TEST(MapLoaderTest, MalformedMapReportsParseError)
{
    std::istringstream map(
        "3 2\n"
        ".R.\n"
        ".X.\n");

    try
    {
        (void)sim::loadScenarioFromMap(map, "bad-map");
        FAIL() << "Expected MapParseError";
    }
    catch (const sim::MapParseError &error)
    {
        EXPECT_EQ(error.line(), 3);
        EXPECT_NE(std::string(error.what()).find("invalid glyph"), std::string::npos);
    }
}
