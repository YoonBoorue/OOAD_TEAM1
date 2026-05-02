#include "rvc/Modes.hpp"

namespace rvc {

// --- StandbyMode ---
bool StandbyMode::checkIsMoving() const { return false; }
OperatingMode& StandbyMode::startButtonPressed() { return *this; }
OperatingMode& StandbyMode::lowBatteryDetected() { return *this; }
OperatingMode& StandbyMode::lowBatteryCleared() { return *this; }
OperatingMode& StandbyMode::dustDetected() { return *this; }
bool StandbyMode::canCharge() const { return true; }
void StandbyMode::timerExpired() {}

// --- NormalMode ---
bool NormalMode::checkIsMoving() const { return true; }
OperatingMode& NormalMode::startButtonPressed() { return *this; }
OperatingMode& NormalMode::lowBatteryDetected() { return *this; }
OperatingMode& NormalMode::lowBatteryCleared() { return *this; }
OperatingMode& NormalMode::dustDetected() { return *this; }
bool NormalMode::canCharge() const { return false; }
void NormalMode::timerExpired() {}

// --- BoostMode ---
bool BoostMode::checkIsMoving() const { return true; }
OperatingMode& BoostMode::startButtonPressed() { return *this; }
OperatingMode& BoostMode::lowBatteryDetected() { return *this; }
OperatingMode& BoostMode::lowBatteryCleared() { return *this; }
OperatingMode& BoostMode::dustDetected() { return *this; }
bool BoostMode::canCharge() const { return false; }
void BoostMode::timerExpired() {}

// --- LowBatteryMode ---
bool LowBatteryMode::checkIsMoving() const { return true; }
OperatingMode& LowBatteryMode::startButtonPressed() { return *this; }
OperatingMode& LowBatteryMode::lowBatteryDetected() { return *this; }
OperatingMode& LowBatteryMode::lowBatteryCleared() { return *this; }
OperatingMode& LowBatteryMode::dustDetected() { return *this; }
bool LowBatteryMode::canCharge() const { return false; }
void LowBatteryMode::timerExpired() {}

} // namespace rvc
