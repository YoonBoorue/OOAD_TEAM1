#include "rvc/DustProcessor.hpp"

namespace rvc
{

    bool DustProcessor::decideIsDusted(bool dust, const OperatingMode & /*currentMode*/, CleanerDriver & /*cleaner*/) 
    {
    return dust;
    }

} // namespace rvc
