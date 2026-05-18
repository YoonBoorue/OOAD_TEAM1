#ifndef VIBE_RVC_OPERATING_MODE_HPP
#define VIBE_RVC_OPERATING_MODE_HPP

namespace rvc
{

class Controller;

class OperatingMode
{
public:
    virtual ~OperatingMode() = default;

    virtual OperatingMode* startButtonPressed(Controller& controller) = 0;
    virtual OperatingMode* powerButtonPressed(Controller& controller) = 0;
    virtual OperatingMode* dustDetected(Controller& controller) = 0;
    virtual OperatingMode* lowBatteryDetected(Controller& controller) = 0;
    virtual OperatingMode* timerExpired(Controller& controller) = 0;
    virtual OperatingMode* obstacleDetected(Controller& controller) = 0;
};

class StandbyMode final : public OperatingMode
{
public:
    OperatingMode* startButtonPressed(Controller& controller) override;
    OperatingMode* powerButtonPressed(Controller& controller) override;
    OperatingMode* dustDetected(Controller& controller) override;
    OperatingMode* lowBatteryDetected(Controller& controller) override;
    OperatingMode* timerExpired(Controller& controller) override;
    OperatingMode* obstacleDetected(Controller& controller) override;
};

class NormalMode final : public OperatingMode
{
public:
    OperatingMode* startButtonPressed(Controller& controller) override;
    OperatingMode* powerButtonPressed(Controller& controller) override;
    OperatingMode* dustDetected(Controller& controller) override;
    OperatingMode* lowBatteryDetected(Controller& controller) override;
    OperatingMode* timerExpired(Controller& controller) override;
    OperatingMode* obstacleDetected(Controller& controller) override;
};

class BoostMode final : public OperatingMode
{
public:
    OperatingMode* startButtonPressed(Controller& controller) override;
    OperatingMode* powerButtonPressed(Controller& controller) override;
    OperatingMode* dustDetected(Controller& controller) override;
    OperatingMode* lowBatteryDetected(Controller& controller) override;
    OperatingMode* timerExpired(Controller& controller) override;
    OperatingMode* obstacleDetected(Controller& controller) override;
};

class LowBatteryMode final : public OperatingMode
{
public:
    OperatingMode* startButtonPressed(Controller& controller) override;
    OperatingMode* powerButtonPressed(Controller& controller) override;
    OperatingMode* dustDetected(Controller& controller) override;
    OperatingMode* lowBatteryDetected(Controller& controller) override;
    OperatingMode* timerExpired(Controller& controller) override;
    OperatingMode* obstacleDetected(Controller& controller) override;
};

} // namespace rvc

#endif // VIBE_RVC_OPERATING_MODE_HPP
