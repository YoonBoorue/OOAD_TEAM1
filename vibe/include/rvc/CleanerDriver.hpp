#ifndef VIBE_RVC_CLEANER_DRIVER_HPP
#define VIBE_RVC_CLEANER_DRIVER_HPP

namespace rvc
{

class CleanerDriver
{
public:
    bool isRunning;
    bool isBoosting;

    CleanerDriver() = default;

    void start();
    void stop();
    void boost();
    void normal();
};

} // namespace rvc

#endif // VIBE_RVC_CLEANER_DRIVER_HPP
