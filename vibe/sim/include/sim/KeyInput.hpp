#pragma once

#include <optional>

namespace sim
{

class KeyInput
{
public:
    KeyInput();
    ~KeyInput();

    KeyInput(const KeyInput &) = delete;
    KeyInput &operator=(const KeyInput &) = delete;

    std::optional<char> poll();

private:
#ifndef _WIN32
    bool configured_;
    struct TermiosState;
    TermiosState *state_;
#endif
};

} // namespace sim
