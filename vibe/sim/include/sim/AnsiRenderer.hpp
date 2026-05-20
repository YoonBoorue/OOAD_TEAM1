#pragma once

#include "sim/Renderer.hpp"

#include <iosfwd>

namespace sim
{

class AnsiRenderer final : public Renderer
{
public:
    explicit AnsiRenderer(std::ostream &output);
    ~AnsiRenderer() override;

    AnsiRenderer(const AnsiRenderer &) = delete;
    AnsiRenderer &operator=(const AnsiRenderer &) = delete;

    void render(const RenderFrame &frame) override;

private:
    void enableVirtualTerminalProcessing();
    void restoreTerminal();
    void drawFrame(const RenderFrame &frame);

    std::ostream &output_;
    bool firstFrame_;
};

} // namespace sim
