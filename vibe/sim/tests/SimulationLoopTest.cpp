#include "sim/MapLoader.hpp"
#include "sim/Renderer.hpp"
#include "sim/SimulationLoop.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace
{

sim::SimulationLoop::KeyPoller noKeys()
{
    return []() -> std::optional<char> {
        return std::nullopt;
    };
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

TEST(SimulationLoopTest, ScriptedPowerAndStartKeysAllowCleaning)
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

    EXPECT_GE(simulation.cleanedCells(), 1);
    EXPECT_NE(simulation.modeName(), "StandbyMode");
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
