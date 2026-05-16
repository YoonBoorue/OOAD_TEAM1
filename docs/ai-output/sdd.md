# RVC Control SW SDD/SDS Draft

본 문서는 RVC Control SW의 Software Design Description / Software Design Specification 초안이다.
주요 source는 `docs/ai-input/ood_summary.md`, `docs/ai-input/ooi_summary.md`, `docs/ai-output/srs.md`, `docs/ai-output/srs_diagrams.puml`, 현재 `include/rvc`, `src/rvc`, unit tests, system tests이다.

문서의 class 이름, method 이름, diagram 이름, file 이름은 repository의 English identifier를 유지한다. 설명은 한국어로 작성한다. 본 문서의 diagram은 project artifact summary와 현재 repository evidence를 기반으로 재구성한 것이며, 원본 PDF diagram의 직접 복사본이 아니다.

## 1. Design Overview

RVC Control SW는 `Controller`가 true incoming System Operation을 수신하고, mode object, processor, driver로 behavior를 위임하는 구조이다. SRS 최종 검토 결과에 따라 `powerButtonPressed()`, `startButtonPressed()`, `chargeBattery()`, `stopCharging()`, `lowBatteryDetected()`, `lowBatteryCleared()`, `dustDetected()`, `obstacleDetected(direction)`, `timerExpired()`, `chargingTick()`는 system boundary로 들어오는 operation으로 다룬다. 반면 `moveForward()`, `startCleaning()`, `stopMoving()`, `stopCleaning()`, `turnLeft()`, `turnRight()`, `moveBackward()`는 system 내부 decision 결과가 actuator driver로 나가는 Output Command / Actuator Command이다.

현재 구현은 OOD artifact의 핵심 방향과 대체로 일치한다.

| Design concern | 현재 설계 |
|---|---|
| System operation receiver | `Controller`가 외부 event를 받는다. |
| Mode-specific behavior | `OperatingMode`와 `StandbyMode`, `NormalMode`, `BoostMode`, `LowBatteryMode`가 State Pattern 역할을 수행한다. |
| Hardware abstraction | `BatteryDriver`, `CleanerDriver`, `MotorDriver`, `DustSensorDriver`, `ObstacleSensorDriver`가 simulated hardware state와 command를 캡슐화한다. |
| Decision logic | `DustProcessor`, `ObstacleProcessor`가 dust/obstacle decision을 캡슐화한다. |
| Validation interface | Google Test unit tests와 `rvc_simulator` script system tests가 design behavior를 검증한다. |

중요한 설계 방향은 `Controller`가 God Object가 되지 않도록 mode policy와 decision logic을 별도 class에 두는 것이다. 현재 구현은 이 방향을 일부 만족하지만, charging guard, power on/off 초기화, raw pointer ownership, timer thread control은 여전히 `Controller`에 남아 있다.

## 2. System Architecture

전체 구조는 아래 계층으로 볼 수 있다.

| Layer / Boundary | Component | Responsibility |
|---|---|---|
| External interface | `PowerButton`, `StartButton`, sensor event, battery event, simulator CLI | true incoming System Operation을 발생시킨다. |
| Control facade | `Controller` | system operation entry point, mode/driver/processor coordination, power/charging state 관리 |
| Mode layer | `OperatingMode`, concrete modes | mode별 button, dust, low-battery, timer, movement policy 수행 |
| Processor layer | `DustProcessor`, `ObstacleProcessor` | dust response와 obstacle avoidance direction 결정 |
| Driver layer | `BatteryDriver`, `CleanerDriver`, `MotorDriver`, sensor drivers | simulated hardware state와 actuator command 수행 |
| Validation layer | `tests/*.cpp`, `system_tests/tc/*.rvc`, `cmake/SystemTests.cmake`, GitHub Actions | design behavior를 unit/system level로 검증 |

활성 build 기준으로 `CMakeLists.txt`는 `ooad_core` library를 만들고 `OOAD_TEAM1`, `rvc_simulator`, `ooad_tests`가 이를 link한다. system tests는 `rvc_simulator`와 `.rvc` script를 통해 실행된다.

## 3. Package / Module Structure

| Module | Files | Main classes / functions | Design role |
|---|---|---|---|
| Control | `include/rvc/Controller.hpp`, `src/rvc/Controller.cpp` | `Controller` | incoming operation receiver, coordinator |
| Mode | `include/rvc/OperatingMode.hpp`, `include/rvc/Modes.hpp`, `src/rvc/Modes.cpp` | `OperatingMode`, `StandbyMode`, `NormalMode`, `BoostMode`, `LowBatteryMode`, `standbyMode()`, `normalMode()`, `boostMode()`, `lowBatteryMode()` | State Pattern implementation |
| Driver | `BatteryDriver`, `CleanerDriver`, `MotorDriver`, `DustSensorDriver`, `ObstacleSensorDriver` headers and sources | driver classes | simulated hardware and actuator state abstraction |
| Processor | `include/rvc/DustProcessor.hpp`, `src/rvc/DustProcessor.cpp`, `include/rvc/ObstacleProcessor.hpp`, `src/rvc/ObstacleProcessor.cpp` | `DustProcessor`, `ObstacleProcessor` | dust and obstacle decision delegation |
| Supporting type | `include/rvc/Direction.hpp` | `Direction` | movement/avoidance direction enum |
| Simulator | `simulator/main.cpp` | `rvc_simulator` CLI | interactive/script test interface |
| Tests | `tests/*.cpp`, `system_tests/tc/*.rvc` | Google Test suites, `.rvc` scripts | unit and system validation evidence |
| CI/CD | `CMakeLists.txt`, `cmake/SystemTests.cmake`, `.github/workflows/*.yml` | CMake targets, GitHub Actions jobs | build, test, static analysis, coverage, deployment pipeline |

참고: `simulator/rvc_simulator_main.cpp`도 존재하지만 현재 `CMakeLists.txt`의 `rvc_simulator` target은 `simulator/main.cpp`를 사용한다. 따라서 본 SDD의 active simulator design evidence는 `simulator/main.cpp` 기준이다.

## 4. Class Diagram in PlantUML

Class diagram은 `docs/ai-output/sdd_class_diagram.puml`의 `ClassDiagram_RVCDesign`에 작성했다.

핵심 관계는 다음과 같다.

| Relationship | Description |
|---|---|
| `Controller` owns driver/processor objects | 현재 최종 code는 `new`/`delete` 기반 raw pointer ownership을 사용한다. |
| `Controller` references `OperatingMode` | `currentMode`는 mode singleton을 가리키는 non-owning pointer이다. |
| `OperatingMode` is extended by concrete modes | State Pattern으로 mode별 behavior를 polymorphic하게 수행한다. |
| `DustProcessor` delegates to `OperatingMode::dustDetected()` | dust decision은 processor와 mode policy의 조합으로 처리된다. |
| `ObstacleProcessor` delegates to `OperatingMode::checkIsMoving()` | obstacle direction 선택 후 mode가 motor output command를 수행한다. |
| Drivers hold simulated hardware state | motor status/direction, cleaner mode, battery level/charging 가능 상태, sensor active state를 저장한다. |

## 5. Object-level Sequence Diagrams in PlantUML

Object-level sequence diagrams는 SSD가 아니라 내부 object collaboration을 보여주는 design sequence diagram이다. 각 diagram은 PlantUML rendering tool 호환성을 위해 별도 `.puml` file에 분리되어 있으며, 모두 재구성된 design evidence이다.

| Diagram name | PlantUML file | Related UC | Main incoming operation / command | Purpose |
|---|---|---|---|---|
| `SD_PowerButton_ObjectSequence` | `docs/ai-output/sdd_sd_power_button.puml` | UC1, UC11 | `powerButtonPressed()` | power on/off와 driver initialization/shutdown |
| `SD_StartButton_ModeSequence` | `docs/ai-output/sdd_sd_start_button.puml` | UC2, UC3, UC4, UC7, UC8, UC9 | `startButtonPressed()`, output commands | `OperatingMode`가 movement/cleaning start/stop을 수행하는 흐름 |
| `SD_DustAndTimer_ObjectSequence` | `docs/ai-output/sdd_sd_dust_timer.puml` | UC6 | `dustDetected()`, `timerExpired()` / `timerExpiredNow()` | dust boost mode 진입과 timer 만료 복귀 |
| `SD_ObstacleAvoidance_ObjectSequence` | `docs/ai-output/sdd_sd_obstacle_avoidance.puml` | UC5, UC12, UC13, UC14 | `obstacleDetected(direction)`, output commands | obstacle decision과 motor command mapping |
| `SD_Charging_ObjectSequence` | `docs/ai-output/sdd_sd_charging.puml` | UC10, UC16, UC15 recovery | `chargeBattery()`, `chargingTick()`, `stopCharging()` | charging guard, tick, full-charge stop, low-battery recovery |
| `SD_LowBattery_ObjectSequence` | `docs/ai-output/sdd_sd_low_battery.puml` | UC15 | `lowBatteryDetected()`, `lowBatteryCleared()` | low-battery mode 진입과 recovery policy |
| `StateModeTransitionDesign` | `docs/ai-output/sdd_state_mode_transition.puml` | 전체 mode behavior | mode transition events | mode state와 charging flag의 설계 관계 |

## 6. State / Mode Transition Design

현재 mode 설계는 `OperatingMode` polymorphism과 function-local singleton mode objects로 구현된다.

| Current state | Incoming operation | Next state / effect | Implemented by |
|---|---|---|---|
| Off | `powerButtonPressed()` | `StandbyMode`, sensors/drivers initialized | `Controller::powerButtonPressed()`, `standbyMode()` |
| Off | `chargeBattery()` | battery not full이면 charging on, mode 없음 | `Controller::chargeBattery()`, `BatteryDriver::startCharging()` |
| `StandbyMode` | `startButtonPressed()` | `NormalMode`, motor forward, cleaner normal | `StandbyMode::startButtonPressed()` |
| `NormalMode` | `startButtonPressed()` | `StandbyMode`, motor/cleaner stop | `NormalMode::startButtonPressed()` |
| `BoostMode` | `startButtonPressed()` | `StandbyMode`, motor/cleaner stop | `BoostMode::startButtonPressed()` |
| `LowBatteryMode` | `startButtonPressed()` | `LowBatteryMode`, motor/cleaner remain stopped | `LowBatteryMode::startButtonPressed()` |
| `NormalMode` | `dustDetected()` | `BoostMode`, cleaner boost | `DustProcessor::decideIsDusted()`, `NormalMode::dustDetected()` |
| `BoostMode` | `timerExpired()` | `NormalMode`, cleaner normal | `Controller::timerExpiredNow()`, `BoostMode::timerExpired()` |
| `StandbyMode`, `NormalMode`, `BoostMode`, `LowBatteryMode` | `lowBatteryDetected()` | `LowBatteryMode`, motor/cleaner stop | mode `lowBatteryDetected()`, local `enterLowBatteryMode()` helper |
| `LowBatteryMode` | `lowBatteryCleared()` or `chargingTick()` over threshold | `StandbyMode` | `LowBatteryMode::lowBatteryCleared()` |
| Any on mode | `powerButtonPressed()` | Off, sensors inactive, motor/cleaner stopped, charging off | `Controller::powerButtonPressed()` |

Charging은 별도 `OperatingMode` subclass가 아니라 `Controller::isNowCharging` flag로 구현된다. `StandbyMode`와 `LowBatteryMode`는 `canCharge() == true`, `NormalMode`와 `BoostMode`는 `canCharge() == false`이다. power-off state에서는 `Controller::canStartCharging()`이 charging을 허용한다.

## 7. System Operation to Class/Method Mapping

| True incoming System Operation | Source status | Primary method | Delegated methods / collaborators | Notes |
|---|---|---|---|---|
| `powerButtonPressed()` | source: project artifact, implementation/test | `Controller::powerButtonPressed()` | `BatteryDriver::initialize()`, `turnOffBattery()`, sensor initialize/deactivate, `MotorDriver::stopMoving()`, `CleanerDriver::stopCleaning()`, `standbyMode()` | on/off branch를 모두 처리한다. |
| `startButtonPressed()` | source: project artifact, implementation/test | `Controller::startButtonPressed()` | `OperatingMode::startButtonPressed()`, concrete mode methods, `MotorDriver`, `CleanerDriver` | mode-specific start/standby transition은 mode에 위임된다. |
| `chargeBattery()` | source: project artifact, implementation/test | `Controller::chargeBattery()` | `Controller::canStartCharging()`, `OperatingMode::canCharge()`, `BatteryDriver::startCharging()`, `Controller::chargingTick()` | 현재 구현은 호출 즉시 charging tick 1회를 수행한다. |
| `stopCharging()` | source: project artifact, implementation/test | `Controller::stopCharging()` | `BatteryDriver::stopCharging()` | charging flag를 false로 만든다. |
| `lowBatteryDetected()` | source: project artifact, implementation/test | `Controller::lowBatteryDetected()` | `OperatingMode::lowBatteryDetected()`, `MotorDriver::stopMoving()`, `CleanerDriver::stopCleaning()` | mode-specific stop policy가 mode layer에 있다. |
| `lowBatteryCleared()` | source: project artifact, implementation/test | `Controller::lowBatteryCleared()` | `OperatingMode::lowBatteryCleared()` | current implementation policy는 `LowBatteryMode`에서 `StandbyMode` 복귀이다. |
| `dustDetected()` | source: project artifact, implementation/test | `Controller::dustDetected()` | `DustProcessor::decideIsDusted()`, `OperatingMode::dustDetected()`, `CleanerDriver::decideSetting()` | charging 중이거나 mode가 없으면 무시된다. |
| `obstacleDetected(direction)` | source: project artifact, implementation/test | `Controller::obstacleDetected(const bool direction[3])` | `ObstacleProcessor::decideDirection()`, `OperatingMode::checkIsMoving()`, `MotorDriver` commands | direction 배열은 front/left/right blocked 상태를 의미한다. |
| `timerExpired()` | source: project artifact | `Controller::timerExpiredNow()` for repo/test helper, `Controller::startTimer()` for async path | `OperatingMode::timerExpired()`, `CleanerDriver::decideSetting(false)` | requirement name은 `timerExpired()`이고 repo public helper는 `timerExpiredNow()`이다. |
| `chargingTick()` | source: implementation/test | `Controller::chargingTick()` | `BatteryDriver::inclineLV()`, `BatteryDriver::isFull()`, `Controller::lowBatteryCleared()` | artifact-level `clockTick()` 중 charging behavior만 부분 대체한다. |
| `clockTick()` | source: project artifact | 없음 | 없음 | artifact-only / not implemented. 일반 periodic operation은 현재 code에 없다. |

## 8. Output Command to Driver/Mode Mapping

| Output Command / Actuator Command | Triggering incoming operation | Design owner | Driver method | Behavior |
|---|---|---|---|---|
| `moveForward()` | `startButtonPressed()`, `obstacleDetected(direction)` | `StandbyMode::startButtonPressed()`, `NormalMode::checkIsMoving()`, `BoostMode::checkIsMoving()` | `MotorDriver::moveForward()` | motor moving true, forward true |
| `startCleaning()` | `startButtonPressed()` | `StandbyMode::startButtonPressed()` | `CleanerDriver::startCleaning()` | cleaner mode `normal` |
| `stopMoving()` | `startButtonPressed()`, `powerButtonPressed()`, `lowBatteryDetected()` | `NormalMode`, `BoostMode`, `LowBatteryMode`, `Controller` power-off branch | `MotorDriver::stopMoving()` | motor status false |
| `stopCleaning()` | `startButtonPressed()`, `powerButtonPressed()`, `lowBatteryDetected()` | `NormalMode`, `BoostMode`, `LowBatteryMode`, `Controller` power-off branch | `CleanerDriver::stopCleaning()` | cleaner mode `off` |
| `turnLeft()` | `obstacleDetected(direction)` | `ObstacleProcessor` chooses `Direction::LEFT`, mode executes | `MotorDriver::turnLeft()` then often `moveForward()` | direction rotates left relative to current direction |
| `turnRight()` | `obstacleDetected(direction)` | `ObstacleProcessor` chooses `Direction::RIGHT`, mode executes | `MotorDriver::turnRight()` then often `moveForward()` | direction rotates right relative to current direction |
| `moveBackward()` | `obstacleDetected(direction)` | `ObstacleProcessor` chooses `Direction::BACK`, mode executes | `MotorDriver::moveBackward()` | forward flag false; moving status remains as previous active movement state |
| boost cleaner setting | `dustDetected()`, `timerExpired()` | `NormalMode::dustDetected()`, `BoostMode::timerExpired()` | `CleanerDriver::decideSetting(true/false)` | cleaner mode `boost` or `normal` |

## 9. SOLID Analysis

| Principle | Current evidence | Assessment | Refinement candidate |
|---|---|---|---|
| SRP | `Controller` receives system operations, modes handle mode policy, drivers store hardware state, processors decide dust/obstacle behavior. | Mostly satisfied, but `Controller` still owns construction, charging guard, timer thread, and power lifecycle. | Introduce constructor injection and move timer/charging policy into small collaborators if project scope grows. |
| OCP | New mode can extend `OperatingMode`; current mode methods are virtual. | Satisfied for mode behavior. Adding a new driver type would still require `Controller` changes. | Depend on driver interfaces if hardware variants become real. |
| LSP | `StandbyMode`, `NormalMode`, `BoostMode`, `LowBatteryMode` all implement the same `OperatingMode` contract. Unsupported actions safely no-op or remain in the same mode. | Mostly satisfied. | Document no-op semantics for every mode operation. |
| ISP | Battery, cleaner, motor, dust sensor, obstacle sensor responsibilities are separated. | Satisfied at class level. | Avoid adding unrelated simulator accessors to production-facing interfaces if hardware integration begins. |
| DIP | `Controller` currently depends directly on concrete drivers/processors via raw pointers. | Partially satisfied by stable classes, but not by abstractions. | Use dependency injection with `std::unique_ptr` or references to abstract interfaces for testability. |

God Object risk is reduced by the State Pattern and processors, but not eliminated. The largest remaining concentration is `Controller`, which owns all collaborators and contains power, charging, timer, and operation guard logic.

## 10. AI-assisted Design Refinement

AI inspection refined the SDD basis in the following ways.

| Refinement | Reason | SDD impact |
|---|---|---|
| True System Operations separated from Output Commands | SRS final review confirmed SSD boundary correctness. | `System Operation to Class/Method Mapping` and `Output Command to Driver/Mode Mapping` are separate sections. |
| `SSD_ControlOperations` concept removed from SDD as SSD | It showed `:RVCSystem -> MotorDriver/CleanerDriver`, so it was outbound behavior. | SDD uses object-level sequence diagrams for internal design instead. |
| `timerExpired()` mapped to `Controller::timerExpiredNow()` | Artifact name differs from repo helper method. | Sequence and traceability tables keep requirement name and repo mapping. |
| `chargingTick()` added as implementation/test operation | It is required for current charging test behavior. | Charging design sequence and mapping explicitly include it. |
| `clockTick()` retained as artifact-only gap | General periodic operation is not implemented. | Known risks include the gap; no unsupported design is invented. |
| UC12/UC13/UC14 routed through `obstacleDetected(direction)` | Turn left/right/back are not independent incoming system operations. | Obstacle sequence shows these as output commands selected by `ObstacleProcessor` and mode. |

## 11. Design Decisions and Rationale

| Decision | Rationale | Current evidence |
|---|---|---|
| Use `Controller` as facade for incoming operations | External actor/device events need a single system boundary entry point. | `Controller.hpp` exposes system operation methods. |
| Use State Pattern for operating modes | Mode-specific behavior changes by state and should not all be hard-coded in `Controller`. | `OperatingMode` virtual interface and concrete mode classes. |
| Use driver classes for simulated hardware | Movement, cleaning, battery, and sensor state should be isolated from use-case coordination. | `MotorDriver`, `CleanerDriver`, `BatteryDriver`, sensor drivers. |
| Use processors for decision logic | Dust and obstacle decisions should not fully live in `Controller`. | `DustProcessor::decideIsDusted()`, `ObstacleProcessor::decideDirection()`. |
| Represent charging as controller flag, not mode | Current implementation treats charging as cross-cutting state that can coexist with Off, Standby, or LowBattery. | `Controller::isNowCharging`, `chargingTick()`. |
| Use simulator scripts for system validation | Use cases can be validated end-to-end without Google Test internals. | `simulator/main.cpp`, `system_tests/tc/*.rvc`, `cmake/SystemTests.cmake`. |
| Keep mode objects as function-local singletons | Modes are currently stateless, so singleton references avoid allocation. | `standbyMode()`, `normalMode()`, `boostMode()`, `lowBatteryMode()` in `Modes.cpp`. |

## 12. Use Case - System Operation - Sequence Diagram - Class/Method - Unit Test - System Test Traceability Table

| UC | Incoming System Operation / Output Command | SDD sequence diagram | Class / method evidence | Unit test evidence | System test evidence | Support |
|---|---|---|---|---|---|---|
| UC1 Turn On System | `powerButtonPressed()` | `SD_PowerButton_ObjectSequence` | `Controller::powerButtonPressed()`, driver `initialize()`, `standbyMode()` | `ControllerTurnOnSystemTest.cpp`, `ButtonTest.cpp` | `TC01_PowerButtonPressed1.rvc` | Supported |
| UC2 Set Cleaning Mode | `startButtonPressed()` | `SD_StartButton_ModeSequence` | `Controller::startButtonPressed()`, `StandbyMode::startButtonPressed()` | `ButtonTest.cpp`, `ControllerTurnOnSystemTest.cpp` | `TC03_StartButtonPressed1.rvc` | Supported |
| UC3 Move Forward | Trigger: `startButtonPressed()` or `obstacleDetected(direction)`; output `moveForward()` | `SD_StartButton_ModeSequence`, `SD_ObstacleAvoidance_ObjectSequence` | `MotorDriver::moveForward()`, `StandbyMode::startButtonPressed()`, mode `checkIsMoving()` | `ButtonTest.cpp`, `ControllerObstacleDetectedTest.cpp` | `TC03`, `TC24-TC43` | Supported |
| UC4 Start Cleaning | Trigger: `startButtonPressed()` or `dustDetected()`; output `startCleaning()` / boost setting | `SD_StartButton_ModeSequence`, `SD_DustAndTimer_ObjectSequence` | `CleanerDriver::startCleaning()`, `CleanerDriver::decideSetting()` | `ButtonTest.cpp`, `DustDetectedTest.cpp` | `TC03`, `TC19-TC23`, `TC24-TC43` | Supported |
| UC5 Avoid Obstacle | `obstacleDetected(direction)` plus motor output commands | `SD_ObstacleAvoidance_ObjectSequence` | `Controller::obstacleDetected()`, `ObstacleProcessor::decideDirection()`, mode `checkIsMoving()` | `ControllerObstacleDetectedTest.cpp` | `TC24-TC43` | Partial due cleaner-stop mismatch |
| UC6 Adjust Boost Mode | `dustDetected()`, `timerExpired()` mapped to `timerExpiredNow()` | `SD_DustAndTimer_ObjectSequence` | `DustProcessor::decideIsDusted()`, `NormalMode::dustDetected()`, `BoostMode::timerExpired()` | `DustDetectedTest.cpp`, `ControllerEnterLowBatteryModeTest.cpp` | `TC19-TC23`, `TC44` | Supported with timer mapping |
| UC7 Set Stand-by Mode | `startButtonPressed()` | `SD_StartButton_ModeSequence` | `NormalMode::startButtonPressed()`, `BoostMode::startButtonPressed()` | `ButtonTest.cpp` | `TC06_StartButtonPressed4.rvc`, `TC07_StartButtonPressed5.rvc` | Supported |
| UC8 Stop Moving | Trigger: `startButtonPressed()`, `powerButtonPressed()`, `lowBatteryDetected()`; output `stopMoving()` | `SD_StartButton_ModeSequence`, `SD_PowerButton_ObjectSequence`, `SD_LowBattery_ObjectSequence` | `MotorDriver::stopMoving()`, mode start/low-battery methods, power-off branch | `ButtonTest.cpp`, `ControllerTurnOffSystemTest.cpp`, `ControllerEnterLowBatteryModeTest.cpp` | `TC02`, `TC06-TC07`, `TC15-TC18`, `TC45` | Supported |
| UC9 Stop Cleaning | Trigger: `startButtonPressed()`, `powerButtonPressed()`, `lowBatteryDetected()`; output `stopCleaning()` | `SD_StartButton_ModeSequence`, `SD_PowerButton_ObjectSequence`, `SD_LowBattery_ObjectSequence` | `CleanerDriver::stopCleaning()`, mode start/low-battery methods, power-off branch | `ButtonTest.cpp`, `ControllerTurnOffSystemTest.cpp`, `ControllerEnterLowBatteryModeTest.cpp` | `TC02`, `TC06-TC07`, `TC15-TC18`, `TC45` | Supported |
| UC10 Charge Battery | `chargeBattery()`, `chargingTick()` | `SD_Charging_ObjectSequence` | `Controller::chargeBattery()`, `Controller::chargingTick()`, `BatteryDriver::startCharging()`, `inclineLV()` | `ControllerChargeBatteryTest.cpp` | `TC08-TC12`, `TC45` | Partial/Inconsistent with artifact safe-state wording |
| UC11 Turn Off System | `powerButtonPressed()` | `SD_PowerButton_ObjectSequence` | `Controller::powerButtonPressed()` off branch, driver stop/deactivate methods | `ControllerTurnOffSystemTest.cpp`, `ButtonTest.cpp` | `TC02_PowerButtonPressed2.rvc` | Supported |
| UC12 Turn Left | Trigger: `obstacleDetected(direction)`; output `turnLeft()` and `moveForward()` | `SD_ObstacleAvoidance_ObjectSequence` | `ObstacleProcessor::decideDirection()`, `MotorDriver::turnLeft()`, `MotorDriver::moveForward()` | `ControllerObstacleDetectedTest.cpp` | `TC24`, `TC28-TC29`, `TC34`, `TC38-TC39` | Supported |
| UC13 Turn Right | Trigger: `obstacleDetected(direction)`; output `turnRight()` and `moveForward()` | `SD_ObstacleAvoidance_ObjectSequence` | `ObstacleProcessor::decideDirection()`, `MotorDriver::turnRight()`, `MotorDriver::moveForward()` | `ControllerObstacleDetectedTest.cpp` | `TC30`, `TC36`, `TC40` | Supported |
| UC14 Move Backward | Trigger: `obstacleDetected(direction)`; output `moveBackward()` | `SD_ObstacleAvoidance_ObjectSequence` | `ObstacleProcessor::decideDirection()`, `MotorDriver::moveBackward()` | `ControllerObstacleDetectedTest.cpp` | `TC26`, `TC28`, `TC32-TC33`, `TC36`, `TC38`, `TC42-TC43` | Supported |
| UC15 Enter Low Battery Mode | `lowBatteryDetected()`, recovery through `lowBatteryCleared()` / `chargingTick()` | `SD_LowBattery_ObjectSequence`, `SD_Charging_ObjectSequence` | `Controller::lowBatteryDetected()`, mode `lowBatteryDetected()`, `enterLowBatteryMode()`, `LowBatteryMode::lowBatteryCleared()` | `ControllerEnterLowBatteryModeTest.cpp`, `ControllerChargeBatteryTest.cpp` | `TC15-TC18`, `TC45` | Supported with Standby recovery policy |
| UC16 Stop Charging | `stopCharging()` direct request; `chargingTick()` auto-stop at full | `SD_Charging_ObjectSequence` | `Controller::stopCharging()`, `BatteryDriver::stopCharging()`, `Controller::chargingTick()` | `ControllerStopChargingTest.cpp` | `TC45` direct `stop-charge`; `TC13` automatic full-charge stop via `charge-tick` | Supported / Partial for post-stop mode policy |

## 13. Known Design Inconsistencies and Risks

| Risk / inconsistency | Evidence | Impact / handling |
|---|---|---|
| `Controller` raw pointer ownership | `Controller` constructs collaborators with `new` and deletes them manually. | Works with copy disabled, but `std::unique_ptr` would reduce ownership risk and clarify lifetime. |
| Detached timer thread captures `this` | `Controller::startTimer()` creates a detached thread that later calls mode behavior. | Potential lifetime race if `Controller` is destroyed before timer callback. Tests use `timerExpiredNow()` to avoid waiting. |
| General `clockTick()` missing | SRS marks `clockTick()` artifact-only / not implemented. | General periodic sensor/battery behavior remains a gap; `chargingTick()` only covers charging. |
| Sensor signal abstraction mismatch | Artifacts describe drivers sending signals; current tests/simulator call `Controller` operations directly. | Acceptable for simulator, but real hardware integration would need event adapter layer. |
| Obstacle avoidance cleaner stop mismatch | Artifact expects cleaner stop during avoidance; current code/tests keep cleaner active in Normal/Boost. | SRS marks FR-UC5-03 Gap/Inconsistent; SDD diagrams document current implementation. |
| Charging safe-state ambiguity | Artifact suggests safe non-cleaning state for charging; current code rejects charging in `NormalMode`/`BoostMode` and allows Off charging. | Traceability marks UC10 partial/inconsistent; requirement should be clarified before code change. |
| Low-battery recovery policy fixed to Standby | OOD says context-dependent; current `LowBatteryMode::lowBatteryCleared()` returns `StandbyMode`. | Documented as implementation policy. |
| `Direction::Stop` absent | OOD summary mentions possible `Stop`; code enum only has `FRONT`, `LEFT`, `RIGHT`, `BACK`. | Stop is represented by `MotorDriver::status == false`, not direction value. |
| `MotorDriver::moveBackward()` does not set `status` true | Method only sets `forward = false`. | It works for active obstacle recovery because status is already true; direct use from stopped state would be ambiguous. |
| Active simulator source selection | `CMakeLists.txt` builds `simulator/main.cpp`; `simulator/rvc_simulator_main.cpp` also exists. | Possible stale duplicate. Keep active design evidence tied to CMake target. |
| `BatteryDriver::status` naming | `status == true` means full/not chargeable. | Semantically confusing name; future refactor could rename to `full` or `notChargeable`. |
| Branch coverage lower than function coverage | OOI summary reports approximate branch coverage around 19%. | Additional negative and edge-case tests would strengthen validation. |

## Validation Notes

이 SDD는 문서 생성 작업이며 implementation code를 수정하지 않았다. Design evidence는 repository의 현재 class/method names와 SRS final boundary decision을 기준으로 정리했다.
