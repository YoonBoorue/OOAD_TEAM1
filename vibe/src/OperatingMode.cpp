#include "../include/rvc/OperatingMode.hpp"

namespace rvc
{

OperatingMode* StandbyMode::startButtonPressed(Controller&)
{
    return new NormalMode();
}

OperatingMode* StandbyMode::powerButtonPressed(Controller&)
{
    return this;
}

OperatingMode* StandbyMode::dustDetected(Controller&)
{
    return this;
}

OperatingMode* StandbyMode::lowBatteryDetected(Controller&)
{
    return this;
}

OperatingMode* StandbyMode::timerExpired(Controller&)
{
    return this;
}

OperatingMode* StandbyMode::obstacleDetected(Controller&)
{
    return this;
}

OperatingMode* NormalMode::startButtonPressed(Controller&)
{
    return new StandbyMode();
}

OperatingMode* NormalMode::powerButtonPressed(Controller&)
{
    return this;
}

OperatingMode* NormalMode::dustDetected(Controller&)
{
    return new BoostMode();
}

OperatingMode* NormalMode::lowBatteryDetected(Controller&)
{
    return new LowBatteryMode();
}

OperatingMode* NormalMode::timerExpired(Controller&)
{
    return this;
}

OperatingMode* NormalMode::obstacleDetected(Controller&)
{
    return this;
}

OperatingMode* BoostMode::startButtonPressed(Controller&)
{
    return new StandbyMode();
}

OperatingMode* BoostMode::powerButtonPressed(Controller&)
{
    return this;
}

OperatingMode* BoostMode::dustDetected(Controller&)
{
    return this;
}

OperatingMode* BoostMode::lowBatteryDetected(Controller&)
{
    return new LowBatteryMode();
}

OperatingMode* BoostMode::timerExpired(Controller&)
{
    return new NormalMode();
}

OperatingMode* BoostMode::obstacleDetected(Controller&)
{
    return this;
}

OperatingMode* LowBatteryMode::startButtonPressed(Controller&)
{
    return this;
}

OperatingMode* LowBatteryMode::powerButtonPressed(Controller&)
{
    return this;
}

OperatingMode* LowBatteryMode::dustDetected(Controller&)
{
    return this;
}

OperatingMode* LowBatteryMode::lowBatteryDetected(Controller&)
{
    return this;
}

OperatingMode* LowBatteryMode::timerExpired(Controller&)
{
    return this;
}

OperatingMode* LowBatteryMode::obstacleDetected(Controller&)
{
    return this;
}

} // namespace rvc
