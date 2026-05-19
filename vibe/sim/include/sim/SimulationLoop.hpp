#pragma once

#include "sim/Renderer.hpp"
#include "sim/RvcAdapter.hpp"
#include "sim/Scenario.hpp"

#include <functional>
#include <optional>

namespace sim
{

class SimulationLoop
{
public:
    using KeyPoller = std::function<std::optional<char>()>;

    SimulationLoop(Scenario scenario, Renderer &renderer, KeyPoller keyPoller, int tickDelayMs, bool autoStart = true);

    void start();
    bool tick();
    void run();
    void runForTicks(int maxTicks);

    int tickCount() const;
    int cleanedCells() const;
    int remainingDust() const;
    int batteryLevel() const;
    std::string modeName() const;
    bool isStoppedByLowBattery() const;

private:
    void dispatchKey(char key);
    RenderFrame makeFrame(const ActuatorSnapshot &actuators) const;

    Scenario scenario_;
    RvcAdapter rvc_;
    Renderer &renderer_;
    KeyPoller keyPoller_;
    int tickDelayMs_;
    bool autoStart_;
    int tickCount_;
    bool started_;
    bool chargingActive_;
    bool quitRequested_;
};

} // namespace sim
