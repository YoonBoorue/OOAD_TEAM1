#include "sim/SimulationLoop.hpp"

#include "sim/DirectionUtil.hpp"

#include <algorithm>
#include <chrono>
#include <thread>
#include <utility>

namespace sim
{

SimulationLoop::SimulationLoop(Scenario scenario, Renderer &renderer, KeyPoller keyPoller, int tickDelayMs, bool autoStart)
    : scenario_(std::move(scenario)),
      rvc_(),
      renderer_(renderer),
      keyPoller_(std::move(keyPoller)),
      tickDelayMs_(std::max(0, tickDelayMs)),
      autoStart_(autoStart),
      tickCount_(0),
      started_(false),
      chargingActive_(false),
      quitRequested_(false)
{
}

void SimulationLoop::start()
{
    if (!started_)
    {
        if (autoStart_)
        {
            rvc_.powerOnAndStart();
        }
        started_ = true;
    }
}

bool SimulationLoop::tick()
{
    start();

    if (keyPoller_)
    {
        const std::optional<char> key = keyPoller_();
        if (key.has_value())
        {
            dispatchKey(*key);
        }
    }

    if (quitRequested_)
    {
        return false;
    }

    if (chargingActive_)
    {
        rvc_.chargingTick();
    }

    const SensorSnapshot sensors = scenario_.environment.sense(rvc_.heading());
    rvc_.feedSensors(sensors);

    const ActuatorSnapshot actuators = rvc_.actuators();
    scenario_.environment.applyActuators(actuators);
    ++tickCount_;

    renderer_.render(makeFrame(actuators));

    if (tickDelayMs_ > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(tickDelayMs_));
    }

    return !quitRequested_ && remainingDust() > 0;
}

void SimulationLoop::run()
{
    while (tick())
    {
    }
}

void SimulationLoop::runForTicks(int maxTicks)
{
    for (int i = 0; i < maxTicks; ++i)
    {
        if (!tick())
        {
            break;
        }
    }
}

int SimulationLoop::tickCount() const
{
    return tickCount_;
}

int SimulationLoop::cleanedCells() const
{
    return scenario_.environment.cleanedCells();
}

int SimulationLoop::remainingDust() const
{
    return scenario_.environment.room().remainingDust();
}

int SimulationLoop::batteryLevel() const
{
    return scenario_.environment.batteryLevel();
}

std::string SimulationLoop::modeName() const
{
    return rvc_.modeName();
}

bool SimulationLoop::isStoppedByLowBattery() const
{
    return rvc_.isLowBatteryMode();
}

void SimulationLoop::dispatchKey(char key)
{
    switch (key)
    {
    case 'p':
    case 'P':
        rvc_.powerButtonPressed();
        break;
    case 's':
    case 'S':
        rvc_.startButtonPressed();
        break;
    case 'c':
    case 'C':
        if (chargingActive_)
        {
            rvc_.stopCharging();
            chargingActive_ = false;
        }
        else
        {
            rvc_.chargeBattery();
            chargingActive_ = true;
        }
        break;
    case 'q':
    case 'Q':
        quitRequested_ = true;
        break;
    default:
        break;
    }
}

RenderFrame SimulationLoop::makeFrame(const ActuatorSnapshot &actuators) const
{
    const Pose &pose = scenario_.environment.pose();
    const Room &room = scenario_.environment.room();

    RenderFrame frame;
    frame.tickIndex = tickCount_;
    frame.roomWidth = room.width();
    frame.roomHeight = room.height();
    frame.dustCells = room.dustCells();
    frame.obstacleCells = room.obstacleCells();
    frame.robotPose = pose;
    frame.modeName = rvc_.modeName();
    frame.batteryLevel = scenario_.environment.batteryLevel();
    frame.motorMoving = actuators.motorMoving;
    frame.motorForward = actuators.motorForward;
    frame.motorDirection = directionName(actuators.motorDirection);
    frame.cleanerRunning = actuators.cleanerRunning;
    frame.cleanerBoost = actuators.cleanerBoost;
    frame.chargingActive = chargingActive_;
    frame.cleanedCells = scenario_.environment.cleanedCells();
    frame.totalDustCells = frame.cleanedCells + room.remainingDust();
    return frame;
}

} // namespace sim
