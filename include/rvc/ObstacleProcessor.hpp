#pragma once
#include "rvc/OperatingMode.hpp"
#include "rvc/Direction.hpp"
#include "rvc/MotorDriver.hpp"
#include <array>

namespace rvc
{

    // 장애물 감지 및 이동 경로 결정
    class ObstacleProcessor
    {
    private:
        std::array<bool, 3> direction_;

    public:
        void decideDirection(const std::array<bool, 3> &dir, const OperatingMode &currentMode, MotorDriver &motor);
    };

} // namespace rvc
