#include "rvc/RobotController.hpp"

#include <iostream>

int main()
{
    rvc::RobotController controller;

    rvc::SensorSnapshot sensors{};
    sensors.frontObstacle = false;
    sensors.leftObstacle = false;
    sensors.rightObstacle = false;
    sensors.dustDetected = true;

    const auto command = controller.decide(sensors);

    std::cout << "OOAD_TEAM1 RVC controller started\n";
    std::cout << "cleaningPowerUp=" << command.cleaningPowerUp << "\n";

    return 0;
}