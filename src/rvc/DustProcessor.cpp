#include "rvc/DustProcessor.hpp"

namespace rvc
{
    OperatingMode &DustProcessor::decideIsDusted(CleanerDriver &cleaner, OperatingMode &currentMode)
    {
        return currentMode.dustDetected(cleaner);
    }
}
