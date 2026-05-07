#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "rvc/Controller.hpp"
#include "rvc/Modes.hpp"

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
            << "  timer-expired                     Fire timer expiry synchronously\n"
            << "  obstacle <front> <left> <right>   Trigger obstacle detection. 1/blocked, 0/clear\n"
            << "  expect <field> <value>            Assert state in CLI script\n"
            << "  exit                              Exit simulator\n"
            << "\n"
            << "Expect fields:\n"
            << "  power, charging, battery, hasMode, mode, dustSensor, obstacleSensor,\n"
            << "  cleaner, cleanerMode, motor, motorForward, motorDirection\n"
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
        if (field == "motordirection")
        {
            switch (controller.motorDirection())
            {
            case rvc::Direction::FRONT: return "FRONT";
            case rvc::Direction::LEFT:  return "LEFT";
            case rvc::Direction::RIGHT: return "RIGHT";
            case rvc::Direction::BACK:  return "BACK";
            default:                    return "UNKNOWN";
            }
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

    // ─────────────────────────────────────────
    // Interactive menu helpers
    // ─────────────────────────────────────────

    void printMenuStatus(const rvc::Controller &controller)
    {
        std::cout << "-----------------------------------\n";
        std::cout << "  Mode    : " << controller.currentModeName() << "\n";
        std::cout << "  Battery : " << controller.batteryLevel() << "%";
        if (controller.isCharging()) { std::cout << " (Charging)"; }
        std::cout << "\n";
        if (controller.isMotorMoving())
        {
            const std::string dirStr = [&]() -> std::string {
                switch (controller.motorDirection()) {
                    case rvc::Direction::FRONT: return "FRONT";
                    case rvc::Direction::LEFT:  return "LEFT";
                    case rvc::Direction::RIGHT: return "RIGHT";
                    case rvc::Direction::BACK:  return "BACK";
                    default:                    return "UNKNOWN";
                }
            }();
            std::cout << "  Motor   : " << dirStr
                      << " / " << (controller.isMotorForward() ? "Forward" : "Backward") << "\n";
        }
        else
        {
            std::cout << "  Motor   : Stopped\n";
        }
        std::cout << "  Cleaner : " << controller.cleanerMode() << "\n";
        std::cout << "-----------------------------------\n";
    }

    void handlePower(rvc::Controller &controller)
    {
        bool wasPowerOn = controller.isPowerOn();
        controller.powerButtonPressed();

        if (!wasPowerOn)
        {
            std::cout << "[System] Power On → " << controller.currentModeName() << "\n";
        }
        else
        {
            std::cout << "[System] Power Off\n";
        }
    }

    void handleStart(rvc::Controller &controller)
    {
        if (!controller.isPowerOn())
        {
            std::cout << "[System] Cannot start: Power is off\n";
            return;
        }
        if (controller.isCharging())
        {
            std::cout << "[System] Cannot start: Charging in progress\n";
            return;
        }

        bool wasCleaning = controller.isCleanerCleaning();
        controller.startButtonPressed();

        std::cout << "[System] Start Button → " << controller.currentModeName() << "\n";
        if (!wasCleaning && controller.isCleanerCleaning())
        {
            std::cout << "[Cleaner] Start Cleaning...\n";
            std::cout << "[Motor] Move Forward\n";
        }
        else if (wasCleaning && !controller.isCleanerCleaning())
        {
            std::cout << "[Cleaner] Stop Cleaning\n";
            std::cout << "[Motor] Stopped\n";
        }
        else
        {
            std::cout << "[System] No change in current mode\n";
        }
    }

    void handleDust(rvc::Controller &controller)
    {
        if (!controller.isPowerOn())
        {
            std::cout << "[Dust] No effect: Power is off\n";
            return;
        }

        bool wasBoost = (controller.currentModeKind() == rvc::ModeKind::Boost);
        controller.dustDetected();

        if (controller.currentModeKind() == rvc::ModeKind::Boost && !wasBoost)
        {
            std::cout << "[Dust Detected] → BoostMode\n";
            std::cout << "[Cleaner] Boost Cleaning for 5 sec...\n";
        }
        else if (controller.currentModeKind() == rvc::ModeKind::Boost)
        {
            std::cout << "[Dust Detected] Already in BoostMode\n";
        }
        else
        {
            std::cout << "[Dust] No effect in current mode (" << controller.currentModeName() << ")\n";
        }
    }

    void handleLowBattery(rvc::Controller &controller)
    {
        if (!controller.isPowerOn())
        {
            std::cout << "[Battery] No effect: Power is off\n";
            return;
        }

        bool wasCleaning = controller.isCleanerCleaning();
        bool wasMoving   = controller.isMotorMoving();
        controller.lowBatteryDetected();

        std::cout << "[Battery] Low Battery Detected! → " << controller.currentModeName() << "\n";
        if (wasCleaning && !controller.isCleanerCleaning())
        {
            std::cout << "[Cleaner] Stop Cleaning\n";
        }
        if (wasMoving && !controller.isMotorMoving())
        {
            std::cout << "[Motor] Stopped\n";
        }
    }

    void handleObstacle(rvc::Controller &controller)
    {
        if (!controller.isPowerOn())
        {
            std::cout << "[Obstacle] No effect: Power is off\n";
            return;
        }

        std::cout << "  Input [front,left,right] (e.g. [1,0,0]): ";
        std::string token;
        if (!(std::cin >> token)) { return; }

        // strip brackets
        for (char &c : token) { if (c == '[' || c == ']') { c = ' '; } }
        std::replace(token.begin(), token.end(), ',', ' ');

        std::istringstream iss(token);
        int f = 0, l = 0, r = 0;
        if (!(iss >> f >> l >> r))
        {
            std::cout << "[Error] Invalid format. Use [front,left,right] e.g. [1,0,0]\n";
            return;
        }

        bool dir[3] = { f != 0, l != 0, r != 0 };

        std::cout << "\n[Obstacle Detected]"
                  << " front=" << (dir[0] ? "blocked" : "clear")
                  << " left="  << (dir[1] ? "blocked" : "clear")
                  << " right=" << (dir[2] ? "blocked" : "clear") << "\n";

        controller.obstacleDetected(dir);

        const std::string dirStr = [&]() -> std::string {
            switch (controller.motorDirection()) {
                case rvc::Direction::FRONT: return "FRONT";
                case rvc::Direction::LEFT:  return "LEFT";
                case rvc::Direction::RIGHT: return "RIGHT";
                case rvc::Direction::BACK:  return "BACK";
                default:                    return "UNKNOWN";
            }
        }();

        std::cout << "[Motor] Direction: " << dirStr
                  << ", Forward: " << (controller.isMotorForward() ? "true" : "false") << "\n";
    }

    void handleCharge(rvc::Controller &controller)
    {
        int prevLevel = controller.batteryLevel();
        controller.chargeBattery();

        if (controller.batteryLevel() > prevLevel)
        {
            std::cout << "[Battery] Charging... " << prevLevel << "% → "
                      << controller.batteryLevel() << "%\n";
            if (controller.batteryLevel() == 100)
            {
                std::cout << "[Battery] Fully Charged!\n";
            }
        }
        else
        {
            std::cout << "[Battery] Cannot charge in current mode ("
                      << controller.currentModeName() << ")\n";
        }
    }

    void runInteractiveMenu(rvc::Controller &controller)
    {
        std::cout << "\n======= RVC Simulator =======\n";

        while (true)
        {
            std::cout << "\n";
            printMenuStatus(controller);
            std::cout << "\n";
            std::cout << "1. Power " << (controller.isPowerOn() ? "Off" : "On") << "\n";
            std::cout << "2. " << (controller.isCleanerCleaning() ? "Stop" : "Start") << " Cleaning\n";
            std::cout << "3. Dust Detected\n";
            std::cout << "4. Low Battery\n";
            std::cout << "5. Charge Battery\n";
            std::cout << "6. Obstacle Detected\n";
            std::cout << "0. Exit\n";
            std::cout << "Select: ";

            int choice;
            if (!(std::cin >> choice)) { break; }

            std::cout << "\n";

            switch (choice)
            {
            case 0: return;
            case 1: handlePower(controller);      break;
            case 2: handleStart(controller);      break;
            case 3: handleDust(controller);       break;
            case 4: handleLowBattery(controller); break;
            case 5: handleCharge(controller);     break;
            case 6: handleObstacle(controller);   break;
            default:
                std::cout << "[Error] Please enter 0-6.\n";
                break;
            }
        }
    }

} // namespace

int main(int argc, char *argv[])
{
    // ── Interactive menu mode (no script file) ──────────────────────────
    if (argc < 2)
    {
        rvc::Controller controller;
        runInteractiveMenu(controller);
        return EXIT_SUCCESS;
    }

    // ── Script file mode (system tests) ─────────────────────────────────
    std::ifstream scriptFile(argv[1]);
    if (!scriptFile.is_open())
    {
        std::cerr << "[ERROR] Cannot open script file: " << argv[1] << '\n';
        return EXIT_FAILURE;
    }
    std::istream *input = &scriptFile;
    std::cout << "RVC Simulator CLI script=" << argv[1] << '\n';

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

        std::cout << "COMMAND line=" << lineNo << " input=\"" << originalLine << "\"\n";

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

        if (command == "timer-expired" || command == "timer_expired")
        {
            controller.timerExpiredNow();
            std::cout << "OK timer-expired\n";
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
