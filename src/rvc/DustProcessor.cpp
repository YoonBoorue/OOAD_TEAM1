#include "rvc/DustProcessor.hpp"

namespace rvc
{

    bool DustProcessor::decideIsDusted(const OperatingMode & /*currentMode*/, CleanerDriver &cleaner)
    {
      ++dustCount_;
      if (dustCount_ >= BOOST_THRESHOLD) {
        dustCount_ = 0;
        return true;
      }
      return false;
    }

} // namespace rvc
