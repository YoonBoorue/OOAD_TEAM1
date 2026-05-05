#pragma once
#include "rvc/OperatingMode.hpp"
#include "rvc/CleanerDriver.hpp"

namespace rvc
{

    // 먼지 감지 및 처리 결정
    class DustProcessor
    {
    private:
        int dustCount_ = 0;
        static constexpr int BOOST_THRESHOLD = 3;

    public:
        bool decideIsDusted(const OperatingMode &currentMode, CleanerDriver &cleaner);
    };

} // namespace rvc
