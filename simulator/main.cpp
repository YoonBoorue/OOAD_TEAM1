#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "rvc/Controller.hpp"

namespace
{

    std::string toLower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
                       { return static_cast<char>(std::tolower(ch)); });
        return value;
    }

    std::string boolText(bool value)
    {
        return value ? "true" : "false";
    }

    std::string onOff(bool value)
    {
        return value ? "on" : "off";
    }

    std::string activeInactive(bool value)
    {
        return value ? "active" : "inactive";
    }

    bool parseBoolToken(const std::string &token, bool &out)
    {
        const std::string value = toLower(token);

        if (value == "1" || value == "true" || value == "on" || value == "yes" ||
            value == "y" || value == "active" || value == "blocked")
        {
            out = true;
            return true;
        }

        if (value == "0" || value == "false" || value == "off" || value == "no" ||
            value == "n" || value == "inactive" || value == "clear")
        {
            out = false;
            return true;
        }

        return false;
    }

    bool parseIntToken(const std::string &token, int &out)
    {
        try
        {
            std::size_t consumed = 0;
            const int value = std::stoi(token, &consumed);
            if (consumed != token.size())
            {
                return false;
            }
            out = value;
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    void printHelp()
    {
        std::cout
            << "Commands:\n"
            << "  help                              Show this help message\n"
            << "  status                            Print current controller state\n"
            << "  power                             Press power button\n"
            << "  start                             Press start button\n"
            << "  battery <0-100>                   Set battery level for simulation setup\n"
            << "  charge                            Start battery charging\n"
            << "  charge-tick [count]               Run charging tick one or more times\n"
            << "  stop-charge                       Stop charging\n"
            << "  low-battery                       Trigger low battery detection\n"
            << "  clear-low-battery                 Clear low battery mode manually\n"
            << "  dust                              Trigger dust detection\n"
            << "  obstacle <front> <left> <right>   Trigger obstacle detection. 1/blocked, 0/clear\n"
            << "  expect <field> <value>            Assert state in CLI script\n"
            << "  exit                              Exit simulator\n"
            << "\n"
            << "Expect fields:\n"
            << "  power, charging, battery, hasMode, mode, dustSensor, obstacleSensor,\n"
            << "  cleaner, cleanerMode, motor, motorForward\n"
            << "\n"
            << "Examples:\n"
            << "  rvc_simulator\n"
            << "  rvc_simulator system_tests/tc/TC01_PowerButtonPressed1.rvc\n";
    }

    void printStatus(const rvc::Controller &controller)
    {
        std::cout
            << "STATE"
            << " power=" << onOff(controller.isPowerOn())
            << " charging=" << onOff(controller.isCharging())
            << " battery=" << controller.batteryLevel()
            << " hasMode=" << boolText(controller.hasCurrentMode())
            << " mode=" << controller.currentModeName()
            << " dustSensor=" << activeInactive(controller.isDustSensorActive())
            << " obstacleSensor=" << activeInactive(controller.isObstacleSensorActive())
            << " cleaner=" << onOff(controller.isCleanerCleaning())
            << " cleanerMode=" << controller.cleanerMode()
            << " motor=" << (controller.isMotorMoving() ? "moving" : "stopped")
            << " motorForward=" << boolText(controller.isMotorForward())
            << '\n';
    }

    std::string fieldValue(const rvc::Controller &controller, const std::string &rawField, bool &knownField)
    {
        knownField = true;
        const std::string field = toLower(rawField);

        if (field == "power")
        {
            return onOff(controller.isPowerOn());
        }
        if (field == "charging")
        {
            return onOff(controller.isCharging());
        }
        if (field == "battery")
        {
            return std::to_string(controller.batteryLevel());
        }
        if (field == "hasmode" || field == "modepresent")
        {
            return boolText(controller.hasCurrentMode());
        }
        if (field == "mode")
        {
            return controller.currentModeName();
        }
        if (field == "dustsensor")
        {
            return activeInactive(controller.isDustSensorActive());
        }
        if (field == "obstaclesensor")
        {
            return activeInactive(controller.isObstacleSensorActive());
        }
        if (field == "cleaner")
        {
            return onOff(controller.isCleanerCleaning());
        }
        if (field == "cleanermode")
        {
            return controller.cleanerMode();
        }
        if (field == "motor")
        {
            return controller.isMotorMoving() ? "moving" : "stopped";
        }
        if (field == "motorforward")
        {
            return boolText(controller.isMotorForward());
        }

        knownField = false;
        return {};
    }

    bool valueMatches(const std::string &actual, const std::string &expected)
    {
        return toLower(actual) == toLower(expected);
    }

    std::vector<std::string> split(const std::string &line)
    {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token)
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    std::string stripComment(std::string line)
    {
        const std::size_t comment = line.find('#');
        if (comment != std::string::npos)
        {
            line.erase(comment);
        }
        return line;
    }

} // namespace

int main(int argc, char *argv[])
{
    std::ifstream scriptFile;
    std::istream *input = &std::cin;

    if (argc >= 2)
    {
        scriptFile.open(argv[1]);
        if (!scriptFile.is_open())
        {
            std::cerr << "[ERROR] Cannot open script file: " << argv[1] << '\n';
            return EXIT_FAILURE;
        }
        input = &scriptFile;
        std::cout << "RVC Simulator CLI script=" << argv[1] << '\n';
    }
    else
    {
        std::cout << "RVC Simulator CLI interactive mode\n";
        std::cout << "Type 'help' for commands. Lines beginning with # are ignored.\n";
    }

    rvc::Controller controller;
    bool failed = false;
    printStatus(controller);

    std::string line;
    int lineNo = 0;

    while (std::getline(*input, line))
    {
        ++lineNo;
        const std::string originalLine = line;
        line = stripComment(line);
        const auto tokens = split(line);

        if (tokens.empty())
        {
            continue;
        }

        if (argc >= 2)
        {
            std::cout << "COMMAND line=" << lineNo << " input=\"" << originalLine << "\"\n";
        }

        const std::string command = toLower(tokens[0]);

        if (command == "help")
        {
            printHelp();
            continue;
        }

        if (command == "exit" || command == "quit")
        {
            break;
        }

        if (command == "status")
        {
            printStatus(controller);
            continue;
        }

        if (command == "power")
        {
            controller.powerButtonPressed();
            std::cout << "OK power\n";
            printStatus(controller);
            continue;
        }

        if (command == "start")
        {
            controller.startButtonPressed();
            std::cout << "OK start\n";
            printStatus(controller);
            continue;
        }

        if (command == "battery" || command == "set-battery")
        {
            if (tokens.size() != 2)
            {
                std::cout << "ERROR line=" << lineNo << " battery command requires one integer level\n";
                failed = true;
                continue;
            }

            int level = 0;
            if (!parseIntToken(tokens[1], level))
            {
                std::cout << "ERROR line=" << lineNo << " invalid battery level: " << tokens[1] << '\n';
                failed = true;
                continue;
            }

            controller.setBatteryLevel(level);
            std::cout << "OK battery " << controller.batteryLevel() << '\n';
            printStatus(controller);
            continue;
        }

        if (command == "charge")
        {
            controller.chargeBattery();
            std::cout << "OK charge\n";
            printStatus(controller);
            continue;
        }

        if (command == "charge-tick" || command == "charging-tick")
        {
            int count = 1;
            if (tokens.size() >= 2 && !parseIntToken(tokens[1], count))
            {
                std::cout << "ERROR line=" << lineNo << " invalid tick count: " << tokens[1] << '\n';
                failed = true;
                continue;
            }
            if (count < 1)
            {
                std::cout << "ERROR line=" << lineNo << " tick count must be at least 1\n";
                failed = true;
                continue;
            }

            for (int i = 0; i < count; ++i)
            {
                controller.chargingTick();
            }
            std::cout << "OK charge-tick " << count << '\n';
            printStatus(controller);
            continue;
        }

        if (command == "stop-charge" || command == "stop-charging")
        {
            controller.stopCharging();
            std::cout << "OK stop-charge\n";
            printStatus(controller);
            continue;
        }

        if (command == "low-battery" || command == "low")
        {
            controller.lowBatteryDetected();
            std::cout << "OK low-battery\n";
            printStatus(controller);
            continue;
        }

        if (command == "clear-low-battery" || command == "clear-low")
        {
            controller.lowBatteryCleared();
            std::cout << "OK clear-low-battery\n";
            printStatus(controller);
            continue;
        }

        if (command == "dust")
        {
            controller.dustDetected();
            std::cout << "OK dust\n";
            printStatus(controller);
            continue;
        }

        if (command == "obstacle")
        {
            if (tokens.size() != 4)
            {
                std::cout << "ERROR line=" << lineNo << " obstacle command requires front left right values\n";
                failed = true;
                continue;
            }

            bool blocked[3] = {false, false, false};
            bool parsedAll = true;
            for (int i = 0; i < 3; ++i)
            {
                if (!parseBoolToken(tokens[static_cast<std::size_t>(i + 1)], blocked[i]))
                {
                    std::cout << "ERROR line=" << lineNo << " invalid obstacle value: "
                              << tokens[static_cast<std::size_t>(i + 1)] << '\n';
                    failed = true;
                    parsedAll = false;
                    break;
                }
            }
            if (!parsedAll)
            {
                continue;
            }

            controller.obstacleDetected(blocked);
            std::cout << "OK obstacle front=" << boolText(blocked[0])
                      << " left=" << boolText(blocked[1])
                      << " right=" << boolText(blocked[2]) << '\n';
            printStatus(controller);
            continue;
        }

        if (command == "expect")
        {
            if (tokens.size() != 3)
            {
                std::cout << "ERROR line=" << lineNo << " expect command requires field and value\n";
                failed = true;
                continue;
            }

            bool knownField = false;
            const std::string actual = fieldValue(controller, tokens[1], knownField);
            if (!knownField)
            {
                std::cout << "ERROR line=" << lineNo << " unknown expect field: " << tokens[1] << '\n';
                failed = true;
                continue;
            }

            if (valueMatches(actual, tokens[2]))
            {
                std::cout << "EXPECT_OK " << tokens[1] << '=' << actual << '\n';
            }
            else
            {
                std::cout << "EXPECT_FAIL line=" << lineNo
                          << " field=" << tokens[1]
                          << " expected=" << tokens[2]
                          << " actual=" << actual << '\n';
                failed = true;
            }
            continue;
        }

        std::cout << "ERROR line=" << lineNo << " unknown command: " << tokens[0] << '\n';
        failed = true;
    }

    if (failed)
    {
        std::cout << "RESULT FAIL\n";
        return EXIT_FAILURE;
    }

    std::cout << "RESULT PASS\n";
    return EXIT_SUCCESS;
}
