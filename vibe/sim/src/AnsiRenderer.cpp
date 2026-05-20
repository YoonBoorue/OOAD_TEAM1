#include "sim/AnsiRenderer.hpp"

#include <algorithm>
#include <ostream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace sim
{
namespace
{

constexpr const char *Reset = "\x1b[0m";
constexpr const char *Yellow = "\x1b[33m";
constexpr const char *Gray = "\x1b[90m";
constexpr const char *BrightCyan = "\x1b[96m";
constexpr const char *BrightMagenta = "\x1b[95m";
constexpr const char *Red = "\x1b[31m";

bool containsPosition(const std::vector<Position> &positions, Position target)
{
    return std::find(positions.begin(), positions.end(), target) != positions.end();
}

char robotGlyph(const RenderFrame &frame)
{
    if (frame.motorDirection == "Left")
    {
        return '<';
    }
    if (frame.motorDirection == "Right")
    {
        return '>';
    }
    if (frame.motorDirection == "Backward")
    {
        return 'v';
    }

    return '^';
}

const char *robotColor(const RenderFrame &frame)
{
    if (frame.modeName == "LowBatteryMode")
    {
        return Red;
    }
    if (frame.modeName == "BoostMode")
    {
        return BrightMagenta;
    }

    return BrightCyan;
}

} // namespace

AnsiRenderer::AnsiRenderer(std::ostream &output)
    : output_(output),
      firstFrame_(true)
{
    enableVirtualTerminalProcessing();
    output_ << "\x1b[?25l";
}

AnsiRenderer::~AnsiRenderer()
{
    restoreTerminal();
}

void AnsiRenderer::render(const RenderFrame &frame)
{
    drawFrame(frame);
}

void AnsiRenderer::enableVirtualTerminalProcessing()
{
#ifdef _WIN32
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(handle, &mode))
    {
        return;
    }

    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(handle, mode);
#endif
}

void AnsiRenderer::restoreTerminal()
{
    output_ << Reset << "\x1b[?25h" << std::flush;
}

void AnsiRenderer::drawFrame(const RenderFrame &frame)
{
    if (firstFrame_)
    {
        output_ << "\x1b[2J";
        firstFrame_ = false;
    }

    output_ << "\x1b[H";
    for (int y = 0; y < frame.roomHeight; ++y)
    {
        for (int x = 0; x < frame.roomWidth; ++x)
        {
            const Position position{x, y};
            if (position == frame.robotPose.position)
            {
                output_ << robotColor(frame) << robotGlyph(frame) << Reset;
            }
            else if (containsPosition(frame.obstacleCells, position))
            {
                output_ << Gray << '#' << Reset;
            }
            else if (containsPosition(frame.dustCells, position))
            {
                output_ << Yellow << '*' << Reset;
            }
            else
            {
                output_ << '.';
            }
        }
        output_ << "   \n";
    }

    output_ << "tick " << frame.tickIndex
            << " | mode " << frame.modeName
            << (frame.chargingActive ? "  CHARGING" : "")
            << " | battery " << frame.batteryLevel << '%'
            << " | motor " << (frame.motorMoving ? "moving" : "stopped")
            << ' ' << frame.motorDirection
            << ' ' << (frame.motorForward ? "forward" : "backward")
            << " | cleaner " << (frame.cleanerRunning ? "on" : "off")
            << (frame.cleanerBoost ? " boost" : "")
            << " | cleaned " << frame.cleanedCells << '/' << frame.totalDustCells
            << "        \n";
    output_ << "[p] power  [s] start  [c] charge  [q] quit        \n";

    output_ << std::flush;
}

} // namespace sim
