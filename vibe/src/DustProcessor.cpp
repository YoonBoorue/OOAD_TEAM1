#include "../include/rvc/DustProcessor.hpp"

#include "../include/rvc/CleanerDriver.hpp"

namespace rvc
{

bool DustProcessor::shouldBoost(const CleanerDriver& cleanerDriver) const
{
    return cleanerDriver.isRunning && !cleanerDriver.isBoosting;
}

} // namespace rvc
