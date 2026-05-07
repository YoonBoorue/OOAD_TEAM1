#pragma once
#include "rvc/OperatingMode.hpp"
#include "rvc/CleanerDriver.hpp"

namespace rvc
{
    // 먼지 감지 및 처리 결정
    class DustProcessor
    {
    public:
        bool decideIsDusted(bool dust, const OperatingMode &currentMode, CleanerDriver &cleaner);
    };

} // namespace rvc