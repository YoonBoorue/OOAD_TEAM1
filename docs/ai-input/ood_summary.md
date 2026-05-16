# OOD Summary - RVC Control SW Project Source

> Purpose: This markdown file is a readable AI-input version of `[T1] OOD_V2.pdf` plus design information visible in later OOI slides.
> It summarizes the OOD-stage design basis: use cases, design sequence diagrams, class responsibilities, and implementation-oriented design rules.
> Source type: project artifact summary.

---

## 1. Document Scope

The OOD artifact refines the OOA use cases into design-level behavior.

Main contents observed:

1. Use Case Overview
2. Refined Use Case Descriptions
3. Design Sequence Diagrams for selected use cases
4. Design basis for Controller, OperatingMode, Drivers, and Processors

The visible OOD slide images include detailed use case descriptions and the beginning of sequence diagrams. Additional sequence diagrams and class diagram information are summarized from the OOI artifact because it contains revised OOD diagrams and code mapping.

---

## 2. Design Overview

The RVC Control SW is designed around a central `Controller` that receives system operations and coordinates domain objects.

The design separates responsibilities into:

| Design Element | Responsibility |
|---|---|
| `Controller` | Receives system operations and delegates work to modes, drivers, and processors. |
| `OperatingMode` | Represents current mode and defines mode-specific behavior. |
| Concrete Modes | `StandbyMode`, `NormalMode`, `BoostMode`, `LowBatteryMode`. |
| Drivers | Abstract hardware-like operations and state: battery, cleaner, motor, dust sensor, obstacle sensor. |
| Processors | Perform decision logic for dust and obstacle handling. |
| Simulator | Provides external testing interface and scripts; it is separated from OOAD core design. |

---

## 3. Important Design Rule

The design should avoid making `Controller` a God Object.

Recommended responsibility distribution:

- `Controller` receives system operations.
- `Controller` keeps references to drivers, processors, and the current operating mode.
- `OperatingMode` handles mode-specific transition and action policy.
- Drivers perform actual state changes or simulated hardware commands.
- Processors encapsulate decision logic such as obstacle avoidance and dust handling.

---

## 4. Main Classes

| Class / Interface | Role |
|---|---|
| `Controller` | Main façade for system operations. Owns or references drivers/processors and current mode. |
| `OperatingMode` | Abstract interface/base class for mode-specific actions and transitions. |
| `StandbyMode` | Idle mode. Motor and cleaner are stopped. Start button enters `NormalMode`. |
| `NormalMode` | Regular cleaning mode. Movement and cleaning are active. Dust detection may enter `BoostMode`. |
| `BoostMode` | Temporary high-power cleaning mode. Timer expiration returns to `NormalMode`. |
| `LowBatteryMode` | Safety mode. Movement and cleaning are stopped. Start cleaning is blocked. |
| `BatteryDriver` | Maintains battery state, low-battery status, and charging state. |
| `CleanerDriver` | Starts/stops cleaner and boost cleaning behavior. |
| `MotorDriver` | Starts/stops movement and sets movement direction. |
| `DustSensorDriver` | Provides dust detection signal. |
| `ObstacleSensorDriver` | Provides obstacle state for front/left/right directions. |
| `DustProcessor` | Decides response to dust detection, usually by delegating mode transition. |
| `ObstacleProcessor` | Decides obstacle avoidance direction based on sensor state. |
| `Direction` | Enumeration or value object representing `Forward`, `Left`, `Right`, `Backward`, `Stop`. |

---

## 5. System Operations and Design Sequence Mapping

| System Operation | Related SD | Related Use Case | Main Collaborators |
|---|---|---|---|
| `powerButtonPressed()` | SD-01 Turn On System / SD-11 Turn Off System | UC1, UC11 | `Controller`, drivers, `OperatingMode` |
| `startButtonPressed()` | SD-02 Set Cleaning Mode / SD-07 Set Stand-by Mode | UC2, UC7 | `Controller`, `OperatingMode`, `MotorDriver`, `CleanerDriver` |
| `chargeBattery()` | SD-10 Charge Battery | UC10 | `Controller`, `BatteryDriver`, `StandbyMode` |
| `stopCharging()` | SD-16 Stop Charging | UC16 | `Controller`, `BatteryDriver` |
| `lowBatteryDetected()` | SD-15 Enter Low Battery Mode | UC15 | `Controller`, `BatteryDriver`, `LowBatteryMode`, `MotorDriver`, `CleanerDriver` |
| `dustDetected()` | SD-06 Adjust Boost Mode | UC6 | `Controller`, `DustProcessor`, `OperatingMode`, `CleanerDriver` |
| `obstacleDetected()` | SD-05 Avoid Obstacle | UC5 | `Controller`, `ObstacleProcessor`, `ObstacleSensorDriver`, `MotorDriver`, `CleanerDriver` |
| `timerExpired()` | SD-06 Adjust Boost Mode | UC6 | `Controller`, `BoostMode`, `NormalMode` |
| `turnLeft()` | SD-12 Turn Left | UC12 | `OperatingMode`, `MotorDriver` |
| `turnRight()` | SD-13 Turn Right | UC13 | `OperatingMode`, `MotorDriver` |
| `moveBackward()` | SD-14 Move Backward | UC14 | `OperatingMode`, `MotorDriver` |

---

## 6. Sequence Design Summaries

### SD-01 Turn On System

1. `PowerButton` sends `powerButtonPressed()` to `Controller`.
2. If the system is off, `Controller` turns on power state.
3. `Controller` initializes hardware-related components.
4. `Controller` sets current direction to `Forward`.
5. `Controller` enters `StandbyMode`.
6. Drivers are initialized or set to idle state.

### SD-02 Set Cleaning Mode

1. `StartButton` sends `startButtonPressed()` to `Controller`.
2. If current mode is `StandbyMode`, `Controller` delegates transition to `OperatingMode`.
3. `StandbyMode.startButtonPressed()` returns or selects `NormalMode`.
4. `Controller` enters `NormalMode`.
5. `NormalMode` activates motor and cleaner as needed.

### SD-05 Avoid Obstacle

1. `ObstacleSensorDriver` sends obstacle information to `Controller`.
2. `Controller` asks `ObstacleProcessor` to decide avoidance direction.
3. `Controller` or current mode stops cleaner during avoidance.
4. Based on the processor result, system performs one of:
   - `turnLeft()`
   - `turnRight()`
   - `moveBackward()`
5. If cleaning mode remains valid, movement and cleaning resume.

### SD-06 Adjust Boost Mode

1. `DustSensorDriver` sends dust signal to `Controller`.
2. `Controller` uses `DustProcessor` or current `OperatingMode` to decide response.
3. If current mode allows boost, system enters `BoostMode`.
4. `BoostMode` activates stronger cleaner behavior.
5. When timer expires, system returns to `NormalMode` unless a higher-priority mode applies.

### SD-07 Set Stand-by Mode

1. `StartButton` sends `startButtonPressed()` to `Controller` while cleaning is active.
2. Current `OperatingMode` handles the event.
3. System enters `StandbyMode`.
4. Motor and cleaner are stopped.

### SD-10 Charge Battery

1. User starts charging.
2. `Controller` checks if charging is allowed.
3. If allowed, system enters safe state such as `StandbyMode`.
4. `BatteryDriver` starts charging.
5. If charging is not allowed, the operation has no effect.

### SD-11 Turn Off System

1. `PowerButton` sends `powerButtonPressed()` to `Controller` while system is on.
2. `Controller` stops cleaner and motor.
3. `Controller` deactivates sensor/driver state as needed.
4. System power state becomes off.
5. Current mode is cleared or no active mode is assigned.

### SD-12 Turn Left

1. Obstacle avoidance selects `Left`.
2. Mode or controller requests `MotorDriver` to turn left.
3. Motor direction changes to left and later returns to forward movement.

### SD-13 Turn Right

1. Obstacle avoidance selects `Right`.
2. Mode or controller requests `MotorDriver` to turn right.
3. Motor direction changes to right and later returns to forward movement.

### SD-14 Move Backward

1. Obstacle avoidance selects `Backward` because front/left/right are blocked.
2. Mode or controller requests `MotorDriver` to move backward.
3. After moving backward, obstacle direction is checked again.
4. The system turns left or right if an available direction exists.

### SD-15 Enter Low Battery Mode

1. `BatteryDriver` sends low-battery signal to `Controller`.
2. `Controller` enters `LowBatteryMode`.
3. `LowBatteryMode` stops motor and cleaner.
4. Start cleaning is blocked while low battery remains active.

### SD-16 Stop Charging

1. User requests stop charging.
2. `Controller` sends stop charging command to `BatteryDriver`.
3. `BatteryDriver` stops charging.
4. System remains in or returns to a non-cleaning safe state.

---

## 7. State / Mode Transition Design

| Current State | Event | Next State | Notes |
|---|---|---|---|
| Off | `powerButtonPressed()` | `StandbyMode` | Initialize drivers and direction. |
| StandbyMode | `startButtonPressed()` | `NormalMode` | Start cleaning behavior. |
| NormalMode | `startButtonPressed()` | `StandbyMode` | Stop cleaning. |
| BoostMode | `startButtonPressed()` | `StandbyMode` | Stop cleaning from boost. |
| NormalMode | `dustDetected()` | `BoostMode` | Enter temporary high-power cleaning. |
| BoostMode | `timerExpired()` | `NormalMode` | Return after boost duration. |
| NormalMode / BoostMode / StandbyMode | `lowBatteryDetected()` | `LowBatteryMode` | Safety mode has high priority. |
| LowBatteryMode | `lowBatteryCleared()` | Context-dependent | Return to allowed mode according to implementation policy. |
| Any On State | `powerButtonPressed()` | Off | Stop motor/cleaner and deactivate. |

---

## 8. SOLID Analysis Basis

### SRP - Single Responsibility Principle

- `Controller` should receive and coordinate system operations only.
- Mode-specific behavior should move to `OperatingMode` subclasses.
- Hardware state manipulation should remain in drivers.
- Decision algorithms should remain in processors.

### OCP - Open/Closed Principle

- New operating modes should be added by extending `OperatingMode` rather than rewriting controller logic.
- New sensor processing strategies should be added by extending processors or strategy objects.

### LSP - Liskov Substitution Principle

- All concrete modes should be substitutable as `OperatingMode`.
- Mode methods should preserve expected semantics: unsupported operations should no-op or return the same mode safely.

### ISP - Interface Segregation Principle

- Driver interfaces should not force unrelated hardware operations.
- Motor, cleaner, battery, dust sensor, and obstacle sensor responsibilities should remain separated.

### DIP - Dependency Inversion Principle

- `Controller` should depend on abstractions or stable interfaces where possible.
- Direct construction of concrete drivers inside `Controller` may reduce testability; dependency injection can improve design.

---

## 9. Design Risks / Refinement Points

| Risk | Explanation | Suggested Refinement |
|---|---|---|
| God Object Controller | Too much decision logic in `Controller` makes maintenance difficult. | Delegate mode policy to `OperatingMode`, obstacle decision to `ObstacleProcessor`, and dust decision to `DustProcessor`. |
| Getter-heavy design | Exposing internal driver state through many getters weakens encapsulation. | Prefer behavior methods such as `canStartCharging()`, `enterLowBatteryMode()`, `stopForSafety()`. |
| Mode transition ambiguity | LowBattery, Boost timer, and charging events can conflict. | Define priority: Off > LowBattery > Charging/Safe > Boost > Normal > Standby. |
| Testability | Direct ownership of concrete drivers makes stubbing harder. | Use constructor injection or test-only setters where appropriate. |

---

## 10. Notes for AI Generation

- The SDD should include PlantUML class and sequence diagrams.
- Keep `Controller` as the system-operation receiver but do not place all behavior there.
- Represent `OperatingMode` using State Pattern.
- Mark diagram content as reconstructed from OOD/OOI artifacts and implementation evidence if exact original diagram text is unavailable.
