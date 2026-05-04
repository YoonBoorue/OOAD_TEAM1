#include "rvc/Modes.hpp"

namespace rvc {

// --- StandbyMode ---
void StandbyMode::checkIsMoving() const {}
OperatingMode& StandbyMode::startButtonPressed() { return *this; }
OperatingMode& StandbyMode::lowBatteryDetected() { return *this; }
OperatingMode& StandbyMode::lowBatteryCleared() { return *this; }
OperatingMode& StandbyMode::dustDetected() { return *this; }
bool StandbyMode::canCharge() const { return true; }
void StandbyMode::timerExpired() {}

// --- NormalMode ---
void NormalMode::checkIsMoving() const {}
OperatingMode& NormalMode::startButtonPressed() { return *this; }
OperatingMode& NormalMode::lowBatteryDetected() { return *this; }
OperatingMode& NormalMode::lowBatteryCleared() { return *this; }
OperatingMode& NormalMode::dustDetected() { return *this; }
bool NormalMode::canCharge() const { return false; }
void NormalMode::timerExpired() {}

// --- BoostMode ---
void BoostMode::checkIsMoving() const {}
OperatingMode& BoostMode::startButtonPressed() { return *this; }
OperatingMode& BoostMode::lowBatteryDetected() { return *this; }
OperatingMode& BoostMode::lowBatteryCleared() { return *this; }
OperatingMode& BoostMode::dustDetected() { return *this; }
bool BoostMode::canCharge() const { return false; }
void BoostMode::timerExpired() {}

// --- LowBatteryMode ---
void LowBatteryMode::checkIsMoving() const {}
OperatingMode& LowBatteryMode::startButtonPressed() { return *this; }
OperatingMode& LowBatteryMode::lowBatteryDetected() { return *this; }
OperatingMode& LowBatteryMode::lowBatteryCleared() { return *this; }
OperatingMode& LowBatteryMode::dustDetected() { return *this; }
bool LowBatteryMode::canCharge() const { return false; }
void LowBatteryMode::timerExpired() {}

} // namespace rvc
