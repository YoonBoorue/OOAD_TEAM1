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

    void initialize();
    void start();
    void startCleaning();
    void stop();
    void stopCleaning();
    void boost();
    void normal();
    void decideSetting(bool boostEnabled);
};

} // namespace rvc

#endif // VIBE_RVC_CLEANER_DRIVER_HPP
