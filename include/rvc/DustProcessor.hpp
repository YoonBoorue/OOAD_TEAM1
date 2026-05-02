#pragma once
#include "rvc/OperatingMode.hpp"

namespace rvc {

// 먼지 감지 및 처리 결정
class DustProcessor {
public:
    bool decideIsDusted(const OperatingMode& currentMode);
};

} // namespace rvc
