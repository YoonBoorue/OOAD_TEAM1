#include "sim/KeyInput.hpp"

#ifdef _WIN32
#include <conio.h>
#else
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace sim
{

#ifndef _WIN32
struct KeyInput::TermiosState
{
    termios original{};
};
#endif

KeyInput::KeyInput()
#ifndef _WIN32
    : configured_(false),
      state_(new TermiosState())
#endif
{
#ifndef _WIN32
    if (tcgetattr(STDIN_FILENO, &state_->original) == 0)
    {
        termios raw = state_->original;
        raw.c_lflag &= static_cast<unsigned int>(~(ICANON | ECHO));
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        configured_ = tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0;
    }
#endif
}

KeyInput::~KeyInput()
{
#ifndef _WIN32
    if (configured_)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &state_->original);
    }
    delete state_;
#endif
}

std::optional<char> KeyInput::poll()
{
#ifdef _WIN32
    if (_kbhit() == 0)
    {
        return std::nullopt;
    }

    return static_cast<char>(_getch());
#else
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(STDIN_FILENO, &readSet);

    timeval timeout{};
    const int ready = select(STDIN_FILENO + 1, &readSet, nullptr, nullptr, &timeout);
    if (ready <= 0 || !FD_ISSET(STDIN_FILENO, &readSet))
    {
        return std::nullopt;
    }

    char value = '\0';
    if (read(STDIN_FILENO, &value, 1) != 1)
    {
        return std::nullopt;
    }

    return value;
#endif
}

} // namespace sim
