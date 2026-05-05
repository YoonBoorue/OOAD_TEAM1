#include "rvc/RobotController.hpp"

#include <cstddef>
#include <iostream>
#include <vector>

namespace
{

    const char *toString(rvc::MotionCommand command)
    {
        switch (command)
        {
        case rvc::MotionCommand::MoveForward:
            return "MoveForward";
        case rvc::MotionCommand::TurnLeft:
            return "TurnLeft";
        case rvc::MotionCommand::TurnRight:
            return "TurnRight";
        case rvc::MotionCommand::MoveBackward:
            return "MoveBackward";
        }

        return "Unknown";
    }

} // namespace

int main()
{
    rvc::RobotController controller;

    const std::vector<rvc::SensorSnapshot> scenario = {
        rvc::SensorSnapshot{false, false, false, false},
        rvc::SensorSnapshot{false, false, false, true},
        rvc::SensorSnapshot{true, false, false, false},
        rvc::SensorSnapshot{true, true, false, false},
        rvc::SensorSnapshot{true, true, true, false}};

    for (std::size_t tick = 0; tick < scenario.size(); ++tick)
    {
        const auto command = controller.decide(scenario[tick]);
        std::cout << "[tick " << tick << "] "
                  << "motion=" << toString(command.motion)
                  << ", cleaningPowerUp=" << command.cleaningPowerUp
                  << '\n';
    }

    return 0;
}