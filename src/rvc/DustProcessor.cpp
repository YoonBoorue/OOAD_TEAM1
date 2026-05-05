#include "rvc/DustProcessor.hpp"

namespace rvc
{

    bool DustProcessor::decideIsDusted(const OperatingMode & /*currentMode*/, CleanerDriver &cleaner)
    {
      return true;
    }

} // namespace rvc
