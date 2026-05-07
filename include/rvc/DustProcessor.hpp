#pragma once
#include "rvc/OperatingMode.hpp"
#include "rvc/CleanerDriver.hpp"

namespace rvc
{
    // 먼지 감지 및 처리 결정
    class DustProcessor
    {
    public:
        OperatingMode &decideIsDusted(CleanerDriver &cleaner, OperatingMode &currentMode);
    };

} // namespace rvc