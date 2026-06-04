#include "sim/AnsiRenderer.hpp"
#include "sim/KeyInput.hpp"
#include "sim/MapLoader.hpp"
#include "sim/SimulationLoop.hpp"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{

constexpr const char *DefaultMapPath = "vibe/sim/maps/default.map";
constexpr int TickDelayMs = 200;

struct Options
{
    std::string mapPath = DefaultMapPath;
};

void restoreTerminalAndExit(int)
{
    std::cout << "\x1b[0m\x1b[?25h" << std::flush;
    std::_Exit(130);
}

Options parseOptions(int argc, char *argv[])
{
    Options options;
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "--map")
        {
            if (++i >= argc)
            {
                throw std::invalid_argument("--map requires a path");
            }
            options.mapPath = argv[i];
        }
        else
        {
            throw std::invalid_argument("unknown option: " + arg);
        }
    }

    return options;
}

void printUsage(const char *program)
{
    std::cerr
        << "Usage: " << program << " [--map <path>]\n";
}

} // namespace

int main(int argc, char *argv[])
{
    std::signal(SIGINT, restoreTerminalAndExit);

    try
    {
        const Options options = parseOptions(argc, argv);
        sim::Scenario scenario = sim::loadScenarioFromMapFile(options.mapPath);
        sim::AnsiRenderer renderer(std::cout);
        sim::KeyInput keyInput;
        sim::SimulationLoop simulation(
            std::move(scenario),
            renderer,
            [&keyInput]() { return keyInput.poll(); },
            TickDelayMs);
        simulation.run();

        std::cout << "Cleaned " << simulation.cleanedCells()
                  << '/' << (simulation.cleanedCells() + simulation.remainingDust())
                  << " cells in " << simulation.tickCount() << " ticks\n";
        return EXIT_SUCCESS;
    }
    catch (const std::exception &error)
    {
        std::cout << "\x1b[0m\x1b[?25h" << std::flush;
        std::cerr << "ERROR: " << error.what() << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }
}
