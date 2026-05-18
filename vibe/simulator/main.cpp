#include "../include/rvc/Controller.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace
{

const char* modeName(const rvc::Controller& controller)
{
    if (controller.currentMode == nullptr)
    {
        return "Off";
    }

    if (dynamic_cast<rvc::StandbyMode*>(controller.currentMode) != nullptr)
    {
        return "StandbyMode";
    }

    if (dynamic_cast<rvc::NormalMode*>(controller.currentMode) != nullptr)
    {
        return "NormalMode";
    }

    if (dynamic_cast<rvc::BoostMode*>(controller.currentMode) != nullptr)
    {
        return "BoostMode";
    }

    if (dynamic_cast<rvc::LowBatteryMode*>(controller.currentMode) != nullptr)
    {
        return "LowBatteryMode";
    }

    return "UnknownMode";
}

const char* onOff(bool value)
{
    return value ? "on" : "off";
}

const char* directionName(rvc::Direction direction)
{
    switch (direction)
    {
    case rvc::Direction::Forward:
        return "Forward";
    case rvc::Direction::Left:
        return "Left";
    case rvc::Direction::Right:
        return "Right";
    case rvc::Direction::Backward:
        return "Backward";
    }

    return "UnknownDirection";
}

void printState(const rvc::Controller& controller)
{
    std::cout << "[Power: " << (controller.currentMode == nullptr ? "OFF" : "ON") << "] "
              << "[Mode: " << modeName(controller) << "] "
              << "[Motor: " << onOff(controller.motorDriver.isRunning) << "] "
              << "[Cleaner: " << onOff(controller.cleanerDriver.isRunning) << "] "
              << "[Boost: " << onOff(controller.cleanerDriver.isBoosting) << "] "
              << "[Charging: " << onOff(controller.batteryDriver.isCharging) << "] "
              << "[Direction: " << directionName(controller.motorDriver.direction) << "]\n";
}

void printMenu()
{
    std::cout << "\n"
              << "1) Power Button  2) Start Button  3) Dust Detected\n"
              << "4) Obstacle (front)  5) Low Battery  6) Charge Battery\n"
              << "7) Stop Charging  8) Timer Expired  9) Clock Tick  0) Exit\n"
              << "> ";
}

void runCommand(rvc::Controller& controller, const std::string& command)
{
    if (command == "power")
    {
        controller.powerButtonPressed();
    }
    else if (command == "start")
    {
        controller.startButtonPressed();
    }
    else if (command == "dust")
    {
        controller.dustSensorDriver.dustDetected = true;
        controller.dustDetected();
    }
    else if (command == "obstacle_front")
    {
        const bool direction[3] = {true, false, false};
        controller.obstacleDetected(direction);
        controller.obstacleSensorDriver.clear();
    }
    else if (command == "obstacle_left")
    {
        const bool direction[3] = {false, true, false};
        controller.obstacleDetected(direction);
        controller.obstacleSensorDriver.clear();
    }
    else if (command == "obstacle_right")
    {
        const bool direction[3] = {false, false, true};
        controller.obstacleDetected(direction);
        controller.obstacleSensorDriver.clear();
    }
    else if (command == "obstacle_all")
    {
        const bool direction[3] = {true, true, true};
        controller.obstacleDetected(direction);
        controller.obstacleSensorDriver.clear();
    }
    else if (command == "low_battery")
    {
        controller.lowBatteryDetected();
    }
    else if (command == "charge")
    {
        controller.chargeBattery();
    }
    else if (command == "stop_charge")
    {
        controller.stopCharging();
    }
    else if (command == "timer")
    {
        controller.timerExpired();
    }
    else if (command == "charge_tick")
    {
        controller.chargingTick();
    }
    else if (command == "clock_tick")
    {
        controller.clockTick();
    }
    else if (command == "low_battery_cleared")
    {
        controller.lowBatteryCleared();
    }
    else
    {
        std::cout << "UNKNOWN " << command << "\n";
    }
}

bool parseOnOff(const std::string& value, bool& result)
{
    if (value == "on")
    {
        result = true;
        return true;
    }

    if (value == "off")
    {
        result = false;
        return true;
    }

    return false;
}

void printExpectation(const std::string& name, bool passed)
{
    std::cout << (passed ? "PASS " : "FAIL ") << name << "\n";
}

int runScript(const std::string& path)
{
    std::ifstream input(path);
    if (!input)
    {
        std::cerr << "Could not open script: " << path << "\n";
        return 1;
    }

    rvc::Controller controller;
    std::string line;
    int failures = 0;

    while (std::getline(input, line))
    {
        std::istringstream stream(line);
        std::string command;
        stream >> command;

        if (command.empty() || command[0] == '#')
        {
            continue;
        }

        if (command == "expect_mode")
        {
            std::string expected;
            stream >> expected;
            const bool passed = expected == modeName(controller);
            failures += passed ? 0 : 1;
            printExpectation("expect_mode " + expected, passed);
        }
        else if (command == "expect_motor")
        {
            std::string expected;
            stream >> expected;
            bool expectedOn = false;
            if (!parseOnOff(expected, expectedOn))
            {
                std::cout << "FAIL expect_motor " << expected << "\n";
                ++failures;
                continue;
            }

            const bool passed = controller.motorDriver.isRunning == expectedOn;
            failures += passed ? 0 : 1;
            printExpectation("expect_motor " + expected, passed);
        }
        else if (command == "expect_cleaner")
        {
            std::string expected;
            stream >> expected;
            bool expectedOn = false;
            if (!parseOnOff(expected, expectedOn))
            {
                std::cout << "FAIL expect_cleaner " << expected << "\n";
                ++failures;
                continue;
            }

            const bool passed = controller.cleanerDriver.isRunning == expectedOn;
            failures += passed ? 0 : 1;
            printExpectation("expect_cleaner " + expected, passed);
        }
        else if (command == "expect_boost")
        {
            std::string expected;
            stream >> expected;
            bool expectedOn = false;
            if (!parseOnOff(expected, expectedOn))
            {
                std::cout << "FAIL expect_boost " << expected << "\n";
                ++failures;
                continue;
            }

            const bool passed = controller.cleanerDriver.isBoosting == expectedOn;
            failures += passed ? 0 : 1;
            printExpectation("expect_boost " + expected, passed);
        }
        else if (command == "expect_charging")
        {
            std::string expected;
            stream >> expected;
            bool expectedOn = false;
            if (!parseOnOff(expected, expectedOn))
            {
                std::cout << "FAIL expect_charging " << expected << "\n";
                ++failures;
                continue;
            }

            const bool passed = controller.batteryDriver.isCharging == expectedOn;
            failures += passed ? 0 : 1;
            printExpectation("expect_charging " + expected, passed);
        }
        else if (command == "expect_direction")
        {
            std::string expected;
            stream >> expected;
            const bool passed = expected == directionName(controller.motorDriver.direction);
            failures += passed ? 0 : 1;
            printExpectation("expect_direction " + expected, passed);
        }
        else
        {
            runCommand(controller, command);
        }
    }

    return failures == 0 ? 0 : 1;
}

int runInteractive()
{
    rvc::Controller controller;

    while (true)
    {
        printMenu();

        int choice = -1;
        if (!(std::cin >> choice))
        {
            return 1;
        }

        switch (choice)
        {
        case 0:
            return 0;
        case 1:
            controller.powerButtonPressed();
            break;
        case 2:
            controller.startButtonPressed();
            break;
        case 3:
            controller.dustSensorDriver.dustDetected = true;
            controller.dustDetected();
            break;
        case 4:
        {
            const bool direction[3] = {true, false, false};
            controller.obstacleDetected(direction);
            controller.obstacleSensorDriver.clear();
            break;
        }
        case 5:
            controller.lowBatteryDetected();
            break;
        case 6:
            controller.chargeBattery();
            break;
        case 7:
            controller.stopCharging();
            break;
        case 8:
            controller.timerExpired();
            break;
        case 9:
            controller.clockTick();
            break;
        default:
            std::cout << "Invalid selection\n";
            break;
        }

        printState(controller);
    }
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc == 3 && std::string(argv[1]) == "--script")
    {
        return runScript(argv[2]);
    }

    if (argc != 1)
    {
        std::cerr << "Usage: " << argv[0] << " [--script <file>]\n";
        return 1;
    }

    return runInteractive();
}
