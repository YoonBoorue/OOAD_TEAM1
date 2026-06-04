#ifndef VIBE_RVC_DUST_PROCESSOR_HPP
#define VIBE_RVC_DUST_PROCESSOR_HPP

namespace rvc
{

class CleanerDriver;

class DustProcessor
{
public:
    DustProcessor() = default;

    bool shouldBoost(const CleanerDriver& cleanerDriver) const;
};

} // namespace rvc

#endif // VIBE_RVC_DUST_PROCESSOR_HPP
