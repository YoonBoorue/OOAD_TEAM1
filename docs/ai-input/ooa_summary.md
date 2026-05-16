# OOA Summary - RVC Control SW Project Source

> Purpose: This markdown file is a readable AI-input version of `[T1] OOA_V2.pdf`.
> It summarizes the OOA-stage use cases, refined use case descriptions, SSD overview, and analysis-level domain concepts.
> Source type: project artifact summary.

---

## 1. Document Scope

The OOA artifact refines the RVC Control SW requirements from Week 1.

Main contents observed:

1. Use Case Overview
2. Refined Use Case Descriptions for UC1 through UC16
3. System Sequence Diagram Overview
4. SSD grouping patterns

A full domain model diagram was not clearly visible in the provided slide images. Domain concepts below are summarized from use cases, actors, and system operations.

---

## 2. Use Case Overview

| Group | Use Cases |
|---|---|
| Power / Charging | UC1. Turn On System, UC10. Charge Battery, UC11. Turn Off System, UC16. Stop Charging |
| Cleaning | UC4. Start Cleaning, UC6. Adjust Boost Mode, UC9. Stop Cleaning |
| Movement / Avoidance | UC3. Move Forward, UC5. Avoid Obstacle, UC8. Stop Moving, UC12. Turn Left, UC13. Turn Right, UC14. Move Backward |
| Mode Transition | UC2. Set Cleaning Mode, UC7. Set Stand-by Mode, UC15. Enter Low Battery Mode |

---

## 3. Refined Use Case Descriptions

### UC1. Turn On System

| Field | Content |
|---|---|
| Actor | User (`PowerButton`) |
| Purpose | 시스템 전원을 켜고 초기 상태를 설정한다. |
| Overview | 사용자가 전원이 꺼진 상태에서 `PowerButton`을 누르면 시스템이 하드웨어 구성요소를 초기화하고 현재 작동 상태를 `StandbyMode`로 전환한다. |
| Type | Essential |
| Cross Reference | FR-UC1-01, FR-UC1-02, FR-UC1-03, FR-UC1-04 |
| Precondition | 시스템 전원이 꺼져 있어야 한다. |
| Typical Course | 1. User가 `PowerButton`을 누른다. 2. System이 각 driver/센서를 초기화한다. 3. H/W components가 idle 상태가 된다. 4. Operating mode가 `StandbyMode`로 설정된다. |
| Alternative | N/A |
| Exception | N/A |

### UC2. Set Cleaning Mode

| Field | Content |
|---|---|
| Actor | User (`StartButton`) |
| Purpose | 시스템을 청소 모드인 `NormalMode`로 전환한다. |
| Overview | `StandbyMode` 상태에서 사용자가 `StartButton`을 누르면 시스템이 `NormalMode`로 전환되고 모터와 클리너가 주기적으로 작동한다. |
| Type | Essential |
| Cross Reference | FR-UC2-01, FR-UC2-02 |
| Precondition | 시스템이 `StandbyMode` 상태여야 한다. |
| Typical Course | 1. User가 `StartButton`을 누른다. 2. System operating mode가 `NormalMode`로 전환된다. |
| Alternative | N/A |
| Exception | N/A |

### UC3. Move Forward

| Field | Content |
|---|---|
| Actor | Motor |
| Purpose | 로봇을 전진 이동시킨다. |
| Overview | 시스템이 `NormalMode` 또는 `BoostMode`일 때 motor는 주기적으로 전진 동작을 수행한다. |
| Type | Essential |
| Cross Reference | FR-UC3-01 |
| Precondition | 시스템이 `NormalMode` 또는 `BoostMode`여야 한다. |
| Typical Course | 1. Controller가 motor에 전진 방향을 전달한다. 2. Motor가 전진한다. |
| Alternative | N/A |
| Exception | N/A |

### UC4. Start Cleaning

| Field | Content |
|---|---|
| Actor | Cleaner |
| Purpose | 청소를 수행한다. |
| Overview | 시스템이 `NormalMode` 또는 `BoostMode`일 때 cleaner가 동작하여 공간을 청소한다. |
| Type | Essential |
| Cross Reference | FR-UC4-01 |
| Precondition | 시스템이 `NormalMode` 또는 `BoostMode`여야 한다. |
| Typical Course | 1. Controller가 cleaner에 청소 시작 명령을 전달한다. 2. Cleaner가 청소를 시작한다. |
| Alternative | N/A |
| Exception | N/A |

### UC5. Avoid Obstacle

| Field | Content |
|---|---|
| Actor | Obstacle Sensors |
| Purpose | 장애물 감지 상황에서 충돌을 회피한다. |
| Overview | `ObstacleSensor`가 장애물 정보를 감지하면 signal을 System에 전달하고, System은 회피 방향을 결정한다. 회피 중에는 cleaner를 정지하고, 회피 후 조건에 따라 이동과 청소를 재개한다. |
| Type | Essential |
| Cross Reference | FR-UC5-01, FR-UC5-02, FR-UC5-03 |
| Precondition | 시스템이 `NormalMode` 또는 `BoostMode`여야 한다. |
| Typical Course | 1. `ObstacleSensor`가 장애물을 감지한다. 2. `ObstacleSensor`가 obstacle signal을 System에 전달한다. 3. System이 회피 방향을 결정한다. 4. System이 회피 방향으로 이동을 명령한다. |
| Alternative | If front is blocked and left is free, turn left. If front is blocked and right is free, turn right. If front/left/right are all blocked, move backward and then turn to an available direction. |
| Exception | N/A |

### UC6. Adjust Boost Mode

| Field | Content |
|---|---|
| Actor | Dust Sensors |
| Purpose | 먼지 감지 시 `BoostMode`로 전환하여 청소 출력을 강화하고, 일정 시간 후 `NormalMode`로 복귀한다. |
| Overview | `DustSensor`가 먼지를 감지하면 dust signal을 System에 전달한다. System은 `BoostMode`로 전환하여 5초 동안 청소를 수행한 후 `NormalMode`로 복귀한다. |
| Type | Essential |
| Cross Reference | FR-UC6-01, FR-UC6-02, FR-UC6-03, FR-UC6-04 |
| Precondition | 시스템이 `NormalMode` 상태여야 한다. |
| Typical Course | 1. `DustSensor`가 먼지를 감지한다. 2. `DustSensor`가 dust signal을 System에 전송한다. 3. System mode가 `BoostMode`로 전환된다. |
| Alternative | N/A |
| Exception | If dust signal is received in `BoostMode`, the boost timer may be reset/extended depending on implementation policy. |

### UC7. Set Stand-by Mode

| Field | Content |
|---|---|
| Actor | StartButton |
| Purpose | 대기 모드로 전환한다. |
| Overview | 시스템이 `NormalMode` 또는 `BoostMode`일 때 사용자가 `StartButton`을 누르면 `StandbyMode`로 전환한다. |
| Type | Essential |
| Cross Reference | FR-UC7-01 |
| Precondition | 시스템이 `NormalMode` 또는 `BoostMode`여야 한다. |
| Typical Course | 1. `StartButton`이 눌린다. 2. System이 현재 모드를 확인한다. 3. System이 `StandbyMode`로 전환된다. |
| Alternative | N/A |
| Exception | N/A |

### UC8. Stop Moving

| Field | Content |
|---|---|
| Actor | Motor |
| Purpose | 이동을 정지한다. |
| Overview | System이 `StandbyMode`, `LowBatteryMode`, 또는 off state에 들어가면 모터 이동이 정지된다. |
| Type | Essential |
| Cross Reference | FR-UC8-01, FR-UC8-02, FR-UC8-03 |
| Precondition | 시스템이 `StandbyMode`로 전환되거나 전원이 꺼져야 한다. |
| Typical Course | 1. Motor에 정지 신호를 보낸다. 2. Motor가 정지한다. |
| Alternative | N/A |
| Exception | N/A |

### UC9. Stop Cleaning

| Field | Content |
|---|---|
| Actor | Cleaner |
| Purpose | 청소를 정지한다. |
| Overview | System이 `StandbyMode`, `LowBatteryMode`, 또는 off state에 들어가면 cleaner가 정지된다. |
| Type | Essential |
| Cross Reference | FR-UC9-01, FR-UC9-02, FR-UC9-03 |
| Precondition | 시스템이 `StandbyMode`로 전환되거나 전원이 꺼져야 한다. |
| Typical Course | 1. Cleaner에 정지 신호를 보낸다. 2. Cleaner가 정지한다. |
| Alternative | N/A |
| Exception | N/A |

### UC10. Charge Battery

| Field | Content |
|---|---|
| Actor | User |
| Purpose | 배터리를 충전하여 시스템이 다시 정상적으로 동작할 수 있도록 한다. |
| Overview | 사용자가 충전을 시작하면 시스템이 배터리를 충전한다. 충전 중에는 청소 동작을 수행하지 않는 안전한 상태를 유지한다. |
| Type | Essential |
| Cross Reference | FR-UC10-01 |
| Precondition | 시스템이 `NormalMode` 또는 `BoostMode`가 아니어야 하며, 충전 가능한 환경이어야 한다. |
| Typical Course | 1. User가 충전을 시작한다. 2. System이 `StandbyMode`로 전환된다. 3. System이 battery charging을 시작한다. 4. 배터리가 일정 수준 이상 충전되면 `StandbyMode` 상태를 유지한다. |
| Alternative | N/A |
| Exception | N/A |

### UC11. Turn Off System

| Field | Content |
|---|---|
| Actor | User (`PowerButton`) |
| Purpose | 시스템을 종료하고 모든 하드웨어 전원을 끈다. |
| Overview | 사용자가 전원이 켜진 상태에서 `PowerButton`을 누르면 시스템이 동작을 종료하고 off state로 전환된다. |
| Type | Essential |
| Cross Reference | FR-UC11-01, FR-UC11-02 |
| Precondition | 시스템이 켜져 있어야 한다. |
| Typical Course | 1. User가 `PowerButton`을 누른다. 2. System이 모든 H/W 객체에 off 또는 idle 신호를 보낸다. 3. H/W components가 off 상태로 전환된다. 4. System이 off state로 전환된다. |
| Alternative | N/A |
| Exception | N/A |

### UC12. Turn Left

| Field | Content |
|---|---|
| Actor | Motor |
| Purpose | 로봇을 좌회전 이동시킨다. |
| Overview | 장애물 회피 판단 결과 왼쪽 방향으로 이동 가능할 경우, System이 Motor에 좌회전 명령을 전달한다. |
| Type | Essential |
| Cross Reference | FR-UC13-01 or related obstacle avoidance FRs in original artifact |
| Precondition | 시스템이 `NormalMode` 또는 `BoostMode`이며, 왼쪽 방향이 이동 가능해야 한다. |
| Typical Course | 1. System이 Motor에 좌회전 명령을 전달한다. 2. Motor가 좌회전한다. |
| Alternative | N/A |
| Exception | N/A |

### UC13. Turn Right

| Field | Content |
|---|---|
| Actor | Motor |
| Purpose | 로봇을 우회전 이동시킨다. |
| Overview | 장애물 회피 판단 결과 오른쪽 방향으로 이동 가능할 경우, System이 Motor에 우회전 명령을 전달한다. |
| Type | Essential |
| Cross Reference | FR-UC14-01 or related obstacle avoidance FRs in original artifact |
| Precondition | 시스템이 `NormalMode` 또는 `BoostMode`이며, 오른쪽 방향이 이동 가능해야 한다. |
| Typical Course | 1. System이 Motor에 우회전 명령을 전달한다. 2. Motor가 우회전한다. |
| Alternative | N/A |
| Exception | N/A |

### UC14. Move Backward

| Field | Content |
|---|---|
| Actor | Motor |
| Purpose | 로봇을 후진 이동시킨다. |
| Overview | 전방, 좌측, 우측 방향이 모두 장애물로 막힌 경우, System이 Motor에 후진 명령을 전달한다. 이후 가능한 방향으로 회전한다. |
| Type | Essential |
| Cross Reference | FR-UC12-01, FR-UC12-02 |
| Precondition | 시스템이 `NormalMode` 또는 `BoostMode`이며, 전방/좌측/우측 방향이 모두 막혀 있어야 한다. |
| Typical Course | 1. System이 Motor에 후진 명령을 전달한다. 2. Motor가 후진한다. |
| Alternative | N/A |
| Exception | N/A |

### UC15. Enter Low Battery Mode

| Field | Content |
|---|---|
| Actor | Battery |
| Purpose | 배터리 잔량 부족 시 `LowBatteryMode`로 전환한다. |
| Overview | 배터리 잔량이 임계값 이하로 떨어지면 System에 신호를 보내고, System은 전체 동작을 안전하게 제한하는 `LowBatteryMode`로 전환한다. |
| Type | Essential |
| Cross Reference | FR-UC10-01, FR-UC10-02 in the original artifact context |
| Precondition | 시스템이 켜져 있어야 한다. |
| Typical Course | 1. Battery가 low-battery signal을 전송한다. 2. System이 `LowBatteryMode`로 전환된다. 3. System이 강제 동작 정지 명령을 내린다. |
| Alternative | N/A |
| Exception | N/A |

### UC16. Stop Charging

| Field | Content |
|---|---|
| Actor | User |
| Purpose | 배터리 충전을 끝낸다. |
| Overview | 사용자가 충전을 종료하면 System이 battery charging을 종료한다. |
| Type | Essential |
| Cross Reference | FR-UC16-01 |
| Precondition | 충전 중이어야 한다. |
| Typical Course | 1. User가 충전 중지 동작을 한다. 2. System이 충전을 멈춘다. |
| Alternative | N/A |
| Exception | N/A |

---

## 4. SSD Overview

The OOA artifact groups SSDs into patterns rather than showing every diagram in full detail in the overview pages.

| SSD Pattern | Related Use Cases | Meaning |
|---|---|---|
| User Control Pattern | UC1, UC2, UC7, UC10, UC11, UC16 | User or button input enters the system boundary and triggers system operations. |
| Sensor Data Pattern | UC5, UC6, UC15 | Sensor/driver input enters the system boundary and triggers mode or movement decisions. |
| RVC Control Pattern | UC3, UC4, UC8, UC9 | System sends control commands to motor or cleaner. |
| Direction Change Pattern | UC12, UC13, UC14 | System changes movement direction based on obstacle avoidance decision. |

---

## 5. System Operations Derived from SSDs

| System Operation | Related Use Case | Trigger |
|---|---|---|
| `powerButtonPressed()` | UC1, UC11 | User presses power button. |
| `startButtonPressed()` | UC2, UC7 | User presses start button. |
| `chargeBattery()` | UC10 | User starts charging. |
| `stopCharging()` | UC16 | User stops charging. |
| `lowBatteryDetected()` | UC15 | Battery driver sends low-battery signal. |
| `lowBatteryCleared()` | UC15 / recovery | Battery state recovers above low threshold. |
| `dustDetected()` | UC6 | Dust sensor driver sends dust signal. |
| `obstacleDetected()` | UC5 | Obstacle sensor driver sends obstacle information. |
| `timerExpired()` | UC6 | Boost timer expires. |
| `clockTick()` | Common periodic behavior | Digital clock tick occurs. |
| `moveForward()` | UC3 | System continues normal movement. |
| `startCleaning()` | UC4 | System activates cleaner. |
| `stopMoving()` | UC8 | Mode/off transition requires motor stop. |
| `stopCleaning()` | UC9 | Mode/off transition requires cleaner stop. |
| `turnLeft()` | UC12 | Obstacle avoidance selects left. |
| `turnRight()` | UC13 | Obstacle avoidance selects right. |
| `moveBackward()` | UC14 | Obstacle avoidance selects backward movement. |

---

## 6. Analysis-Level Domain Concepts

| Concept | Meaning |
|---|---|
| `RVC System` | System under discussion. Receives user/sensor events and controls cleaning/movement. |
| `OperatingMode` | Abstract concept representing system operating state such as Standby, Normal, Boost, and LowBattery. |
| `StandbyMode` | Waiting state after power-on or cleaning stop. Motor and cleaner are stopped. |
| `NormalMode` | Regular cleaning mode. Motor and cleaner operate. |
| `BoostMode` | Temporary high-power cleaning mode entered after dust detection. |
| `LowBatteryMode` | Safety mode entered when battery is low. Motor and cleaner are stopped. |
| `Battery` / `BatteryDriver` | Provides battery status and charging/low-battery events. |
| `DustSensor` / `DustSensorDriver` | Detects dust and generates dust signal. |
| `ObstacleSensor` / `ObstacleSensorDriver` | Detects obstacle state in front, left, and right directions. |
| `Motor` / `MotorDriver` | Executes movement commands. |
| `Cleaner` / `CleanerDriver` | Executes cleaning commands. |
| `DigitalClockTick` | Periodic event used for repeated sensor checks and mode behavior. |

---

## 7. Notes for AI Generation

- Use cases should be written in essential, black-box style in SRS.
- SSDs should show system-level interactions only: actor/device -> `:RVCSystem`.
- Design-level object collaboration belongs in SDD, not SRS.
- If exact SSD diagram details are unavailable, generate PlantUML SSDs from the system operations above and mark them as reconstructed from OOA summary and implementation evidence.
