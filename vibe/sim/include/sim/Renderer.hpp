#pragma once

#include "sim/Pose.hpp"

#include <string>
#include <vector>

namespace sim
{

struct RenderFrame
{
    int tickIndex = 0;
    int roomWidth = 0;
    int roomHeight = 0;

    std::vector<Position> dustCells;
    std::vector<Position> obstacleCells;
    Pose robotPose;

    std::string modeName;
    int batteryLevel = 0;
    bool motorMoving = false;
    bool motorForward = true;
    std::string motorDirection;
    bool cleanerRunning = false;
    bool cleanerBoost = false;
    bool chargingActive = false;
    int cleanedCells = 0;
    int totalDustCells = 0;
};

class Renderer
{
public:
    virtual ~Renderer() = default;

    virtual void render(const RenderFrame &frame) = 0;
};

class NullRenderer final : public Renderer
{
public:
    void render(const RenderFrame &frame) override;
};

} // namespace sim
