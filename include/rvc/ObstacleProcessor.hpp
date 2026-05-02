#pragma once
#include "rvc/OperatingMode.hpp"
#include <array>

namespace rvc {

// 방향 열거형
enum class Direction {
    FRONT,
    LEFT,
    RIGHT
};

// 장애물 감지 및 이동 경로 결정
class ObstacleProcessor {
private:
    std::array<bool, 3> direction_;

public:
    Direction decideDirection(const std::array<bool, 3>& dir, const OperatingMode& currentMode, bool fw);
    Direction decideExit(const std::array<bool, 2>& dir);
};

} // namespace rvc
