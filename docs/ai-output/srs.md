# RVC Control SW SRS Draft

문서 버전: draft-1  
작성 기준일: 2026-05-16  
대상 시스템: RVC Control SW  
관련 diagram 파일: `docs/ai-output/srs_ssd_user_control.puml`, `docs/ai-output/srs_ssd_sensor_events.puml`, `docs/ai-output/srs_diagrams.puml`

## 1. Introduction

### 1.1 목적

이 문서는 RVC Control SW 프로젝트의 Software Requirements Specification(SRS) 초안이다. 본 SRS는 RVC가 전원, 청소 모드, 이동, 장애물 회피, 먼지 감지, 배터리 충전, 저전력 상태를 어떻게 처리해야 하는지를 black-box 관점에서 정의한다.

### 1.2 Source of Truth

본 문서의 주요 요구사항 source는 다음과 같다.

| Source | 사용 목적 |
|---|---|
| `docs/ai-input/week1_summary.md` | 기능 요구사항, 비기능 요구사항, use case 목록의 primary source |
| `docs/ai-input/ooa_summary.md` | refined use case, SSD system operation, domain concept의 primary source |
| `docs/ai-input/ooi_summary.md` | 구현, 테스트, CI/CD 검증 evidence |
| `include/rvc/*.hpp`, `src/rvc/*.cpp` | 현재 구현 evidence |
| `tests/*.cpp`, `system_tests/tc/*.rvc` | unit/system test evidence |

요구사항은 project artifact에서 온 경우 `source: project artifact`로 표시한다. 구현 또는 테스트에서만 확인되는 내용은 `source: implementation/test`로 표시한다.

### 1.3 문서 원칙

- 요구사항은 project artifact summary를 우선한다.
- 구현이 artifact와 다를 경우 요구사항을 현재 구현에 맞게 조용히 바꾸지 않는다.
- 구현 차이는 `15. AI Inspection Findings`에 별도로 기록한다.
- SRS는 black-box 요구사항 문서이므로 내부 class 설계 상세는 최소화한다.

## 2. System Overview

RVC Control SW는 로봇 청소기의 제어 소프트웨어이다. 시스템은 사용자 입력과 센서/배터리 이벤트를 받아 현재 동작 모드를 결정하고, cleaner와 motor에 명령을 전달한다.

주요 동작은 다음과 같다.

- 사용자가 `PowerButton`을 누르면 system을 켜거나 끈다.
- system은 power-on 이후 `StandbyMode`에서 대기한다.
- 사용자가 `StartButton`을 누르면 `NormalMode`로 전환하여 이동과 청소를 시작한다.
- 청소 중 먼지가 감지되면 `BoostMode`로 전환하고, boost 시간이 끝나면 `NormalMode`로 복귀한다.
- 장애물이 감지되면 front/left/right 상태를 기준으로 회피 방향을 결정한다.
- 배터리가 low threshold 이하가 되면 `LowBatteryMode`로 전환하고 motor/cleaner를 정지한다.
- 충전이 허용되는 상태에서는 battery charging state를 관리한다.

## 3. System Scope and Boundary

### 3.1 포함 범위

| 범위 | 설명 |
|---|---|
| Power control | `PowerButton` 입력에 따른 on/off 전환 |
| Cleaning mode control | `StandbyMode`, `NormalMode`, `BoostMode`, `LowBatteryMode` 전환 |
| Movement control | forward, left, right, backward, stop 동작 명령 |
| Obstacle avoidance | front/left/right obstacle 상태에 따른 회피 방향 결정 |
| Dust handling | dust signal에 따른 boost mode 전환 및 timer expiry 처리 |
| Battery handling | charging, stop charging, low battery event, recovery 처리 |
| Test simulator interface | system test script 실행을 위한 simulator command/expect interface |

### 3.2 제외 범위

| 제외 범위 | 설명 |
|---|---|
| 실제 hardware integration | 실제 센서/모터/청소기 장치 드라이버 구현은 포함하지 않는다. 현재 driver는 simulated state 중심이다. |
| UI/UX application | 사용자 화면, mobile app, cloud service는 SRS 범위 밖이다. |
| Physical navigation algorithm | 지도 작성, 위치 추정, 최적 경로 탐색은 포함하지 않는다. |
| Deployment server behavior | CI/CD와 EC2 배포는 project environment evidence이며 RVC runtime 기능 요구사항은 아니다. |

### 3.3 System Boundary

SRS 관점에서 system boundary는 `:RVCSystem`이다. 외부 actor/device는 system operation을 호출하거나 system으로 signal을 보낸다. system 내부 구현 class(`Controller`, driver, mode, processor)는 SDD에서 상세화한다.

## 4. Actors and External Interfaces

| Actor / External Interface | Role | 관련 incoming system operation / output command |
|---|---|---|
| `User` | 전원, 시작/정지, 충전 시작/중지를 요청한다. | `powerButtonPressed()`, `startButtonPressed()`, `chargeBattery()`, `stopCharging()` |
| `PowerButton` | user의 power input을 system에 전달한다. | `powerButtonPressed()` |
| `StartButton` | user의 cleaning start/standby input을 system에 전달한다. | `startButtonPressed()` |
| `BatteryDriver` | battery level, charging 가능 여부, low-battery event를 제공한다. | `lowBatteryDetected()`, `lowBatteryCleared()`, `chargeBattery()`, `stopCharging()` |
| `DustSensorDriver` | dust detection signal을 제공한다. | `dustDetected()` |
| `ObstacleSensorDriver` | front/left/right obstacle 정보를 제공한다. | `obstacleDetected(direction)` |
| `DigitalClockTick` | periodic behavior와 boost timer expiry를 나타내는 외부 event 개념이다. | `timerExpired()`, `clockTick()` |
| `MotorDriver` | system의 movement output command를 실행한다. | Output commands: `moveForward()`, `turnLeft()`, `turnRight()`, `moveBackward()`, `stopMoving()` |
| `CleanerDriver` | system의 cleaner output command를 실행한다. | Output commands: `startCleaning()`, `stopCleaning()`, boost setting |

## 5. Functional Requirements

### 5.1 Common / Hidden Requirements

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-001 | system은 `Off`, `StandbyMode`, `NormalMode`, `BoostMode`, `LowBatteryMode` operating state/mode를 지원해야 한다. | source: project artifact | Supported: `ModeKind`, `currentMode == nullptr` as `Off` |
| FR-002 | system은 active 상태에서 sensor 및 battery 관련 상태를 주기적으로 확인해야 한다. | source: project artifact | Gap: 일반 `DigitalClockTick` operation 없음 |
| FR-003 | system은 low-battery condition이 해제되면 `LowBatteryMode`에서 사용 가능한 정상 operating mode로 회복해야 한다. | source: project artifact | Supported/Policy: 현재 구현은 `StandbyMode`로 회복 |
| FR-004 | system은 charging/non-charging battery state를 관리해야 한다. | source: project artifact | Supported: `isNowCharging`, `chargeBattery()`, `stopCharging()` |

### 5.2 UC1 - Turn On System

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC1-01 | system이 off일 때 user가 `PowerButton`을 누르면 system은 on 상태가 되어야 한다. | source: project artifact | Supported |
| FR-UC1-02 | system이 켜질 때 obstacle sensor driver, dust sensor driver, motor driver, cleaner driver 등 hardware-related components를 initialize해야 한다. | source: project artifact | Supported |
| FR-UC1-03 | system이 켜질 때 default movement direction을 `Forward`로 설정해야 한다. | source: project artifact | Supported |
| FR-UC1-04 | system이 켜질 때 operating mode를 `StandbyMode`로 설정해야 한다. | source: project artifact | Supported |

### 5.3 UC2 - Set Cleaning Mode

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC2-01 | `StandbyMode`에서 user가 `StartButton`을 누르면 system은 `NormalMode`로 전환해야 한다. | source: project artifact | Supported |
| FR-UC2-02 | `NormalMode`에서 system은 `DigitalClockTick`에 따라 cleaner 및 sensor-related drivers를 주기적으로 동작시켜야 한다. | source: project artifact | Gap: 일반 periodic tick 구현 없음 |

### 5.4 UC3 - Move Forward

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC3-01 | `NormalMode` 또는 `BoostMode`에서 system은 motor가 forward movement를 수행하도록 명령해야 한다. | source: project artifact | Supported |

### 5.5 UC4 - Start Cleaning

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC4-01 | `NormalMode` 또는 `BoostMode`에서 system은 cleaning active 동안 cleaner를 계속 동작시켜야 한다. | source: project artifact | Supported |

### 5.6 UC5 - Avoid Obstacle

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC5-01 | `ObstacleSensorDriver`는 front, left, right 방향의 obstacle을 감지해야 한다. | source: project artifact | Partial: 현재 driver는 active state만 보유 |
| FR-UC5-02 | `ObstacleSensorDriver`는 obstacle 정보를 system에 전달해야 한다. | source: project artifact | Partial: simulator/test가 `obstacleDetected(direction)` 직접 호출 |
| FR-UC5-03 | obstacle avoidance 중 system은 `CleanerDriver`에 cleaning stop을 요청해야 한다. | source: project artifact | Gap/Inconsistent: 현재 구현과 테스트는 cleaner 유지 |
| FR-UC5-04 | system은 obstacle sensor input을 기준으로 avoidance direction을 결정해야 한다. | source: project artifact | Supported |
| FR-UC5-05 | current movement direction이 blocked이면 system은 가능한 경우 left 또는 right 등 available direction을 선택해야 한다. | source: project artifact | Supported |
| FR-UC5-06 | obstacle avoidance 이후 current mode가 `NormalMode` 또는 `BoostMode`이면 system은 movement와 cleaning을 재개해야 한다. | source: project artifact | Supported |

### 5.7 UC6 - Adjust Boost Mode

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC6-01 | `DustSensorDriver`는 dust를 감지해야 한다. | source: project artifact | Partial: 현재 driver는 active/dust state accessor만 있음 |
| FR-UC6-02 | `DustSensorDriver`는 dust signal을 system에 전달해야 한다. | source: project artifact | Partial: simulator/test가 `dustDetected()` 직접 호출 |
| FR-UC6-03 | system이 cleaning 중 dust signal을 받으면 `BoostMode`로 전환해야 한다. | source: project artifact | Supported |
| FR-UC6-04 | boost duration이 만료되면 higher-priority mode가 막지 않는 한 system은 `BoostMode`에서 `NormalMode`로 돌아와야 한다. | source: project artifact | Supported |

### 5.8 UC7 - Set Stand-by Mode

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC7-01 | `NormalMode` 또는 `BoostMode`에서 user가 `StartButton`을 누르면 system은 `StandbyMode`로 전환해야 한다. | source: project artifact | Supported |

### 5.9 UC8 - Stop Moving

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC8-01 | system이 `StandbyMode`에 들어가면 `MotorDriver`를 stop해야 한다. | source: project artifact | Supported |
| FR-UC8-02 | system이 `LowBatteryMode`에 들어가면 `MotorDriver`를 stop해야 한다. | source: project artifact | Supported |
| FR-UC8-03 | system이 off state에 들어가면 `MotorDriver`를 stop해야 한다. | source: project artifact | Supported |

### 5.10 UC9 - Stop Cleaning

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC9-01 | system이 `StandbyMode`에 들어가면 `CleanerDriver`를 stop해야 한다. | source: project artifact | Supported |
| FR-UC9-02 | system이 `LowBatteryMode`에 들어가면 `CleanerDriver`를 stop해야 한다. | source: project artifact | Supported |
| FR-UC9-03 | system이 off state에 들어가면 `CleanerDriver`를 stop해야 한다. | source: project artifact | Supported |

### 5.11 UC10 - Charge Battery

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC10-01 | user가 charging을 시작하고 charging이 허용되면 system은 battery state를 charging으로 설정해야 한다. | source: project artifact | Supported |
| FR-UC10-02 | charging이 시작되면 system은 `StandbyMode` 같은 safe non-cleaning mode에 들어가거나 그 상태를 유지해야 한다. | source: project artifact | Partial/Inconsistent: active cleaning 중 charging은 거부되고, off state charging은 허용 |

### 5.12 UC11 - Turn Off System

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC11-01 | system이 on일 때 user가 `PowerButton`을 누르면 system은 off 상태가 되어야 한다. | source: project artifact | Supported |
| FR-UC11-02 | system이 꺼질 때 hardware-related components를 stop 또는 deactivate해야 한다. | source: project artifact | Supported |

### 5.13 UC12 - Turn Left

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC12-01 | left direction이 avoidance direction으로 선택되면 system은 `MotorDriver`에 left turn을 요청해야 한다. | source: project artifact | Supported |
| FR-UC12-02 | left turn 이후 적절한 경우 system은 forward movement로 돌아가야 한다. | source: project artifact | Supported |

### 5.14 UC13 - Turn Right

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC13-01 | right direction이 avoidance direction으로 선택되면 system은 `MotorDriver`에 right turn을 요청해야 한다. | source: project artifact | Supported |
| FR-UC13-02 | right turn 이후 적절한 경우 system은 forward movement로 돌아가야 한다. | source: project artifact | Supported |

### 5.15 UC14 - Move Backward

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC14-01 | current direction이 blocked이면 system은 `MotorDriver`에 backward movement를 요청해야 한다. | source: project artifact | Supported |
| FR-UC14-02 | backward movement 이후 left 또는 right direction이 available하면 system은 available direction으로 turn해야 한다. | source: project artifact | Supported |

### 5.16 UC15 - Enter Low Battery Mode

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC15-01 | battery level이 10% below/threshold가 되면 `BatteryDriver`는 system에 low-battery signal을 보내야 한다. | source: project artifact | Partial: threshold는 있으나 signal은 직접 호출 방식 |
| FR-UC15-02 | system이 low-battery signal을 받으면 `LowBatteryMode`로 전환해야 한다. | source: project artifact | Supported |

### 5.17 UC16 - Stop Charging

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| FR-UC16-01 | user가 charging stop을 요청하면 system은 battery charging을 stop해야 한다. | source: project artifact | Supported |
| FR-UC16-02 | charging stop 이후 system은 `StandbyMode` 같은 appropriate non-charging state로 전환해야 한다. | source: project artifact | Partial: charging state는 해제하나 mode transition은 명시적이지 않음 |

## 6. Non-Functional Requirements

| ID | Requirement | Source | Current evidence |
|---|---|---|---|
| N1.1 | Cleaning start/stop은 2초 이내에 완료되어야 한다. | source: project artifact | Gap: timing assertion 없음 |
| N1.2 | Obstacle detection 이후 direction change는 0.5초 이내에 시작되어야 한다. | source: project artifact | Gap: timing assertion 없음 |
| N1.3 | Dust detection 이후 `BoostMode` transition은 1초 이내에 발생해야 한다. | source: project artifact | Gap: timing assertion 없음 |
| N2.1 | system은 `DigitalClockTick`에 따라 주기적으로 동작해야 한다. | source: project artifact | Gap: 일반 periodic tick 없음 |
| N3.1 | low-battery threshold는 10% 이하이어야 한다. | source: project artifact | Supported: `LowBatteryThreshold = 10` |

## 7. Use Case List

| Group | Use Cases | Source |
|---|---|---|
| Power / Charging | UC1. Turn On System, UC10. Charge Battery, UC11. Turn Off System, UC16. Stop Charging | source: project artifact |
| Cleaning | UC4. Start Cleaning, UC6. Adjust Boost Mode, UC9. Stop Cleaning | source: project artifact |
| Movement / Avoidance | UC3. Move Forward, UC5. Avoid Obstacle, UC8. Stop Moving, UC12. Turn Left, UC13. Turn Right, UC14. Move Backward | source: project artifact |
| Mode Transition | UC2. Set Cleaning Mode, UC7. Set Stand-by Mode, UC15. Enter Low Battery Mode | source: project artifact |

## 8. Refined Use Case Descriptions

### UC1. Turn On System

| Field | Content |
|---|---|
| Actor | `User`, `PowerButton` |
| Purpose | system 전원을 켜고 초기 state를 설정한다. |
| Precondition | system이 off 상태이다. |
| Main Flow | 1. User가 `PowerButton`을 누른다. 2. system이 driver/sensor-related components를 initialize한다. 3. hardware-related components가 idle state가 된다. 4. operating mode가 `StandbyMode`로 설정된다. |
| Alternative / Exception | N/A |
| Related FR | FR-UC1-01 ~ FR-UC1-04 |
| Source | source: project artifact |

### UC2. Set Cleaning Mode

| Field | Content |
|---|---|
| Actor | `User`, `StartButton` |
| Purpose | system을 cleaning mode인 `NormalMode`로 전환한다. |
| Precondition | system이 `StandbyMode` 상태이다. |
| Main Flow | 1. User가 `StartButton`을 누른다. 2. system operating mode가 `NormalMode`로 전환된다. 3. motor와 cleaner가 normal cleaning 동작을 시작한다. |
| Alternative / Exception | charging 중이면 start request는 무시될 수 있다(source: implementation/test). |
| Related FR | FR-UC2-01, FR-UC2-02 |
| Source | source: project artifact; charging exception source: implementation/test |

### UC3. Move Forward

| Field | Content |
|---|---|
| Actor | `MotorDriver` |
| Purpose | RVC가 forward movement를 수행한다. |
| Precondition | system이 `NormalMode` 또는 `BoostMode`이다. |
| Triggering incoming operation | `startButtonPressed()`로 `NormalMode`에 진입하거나, `obstacleDetected(direction)` 처리 후 회피/복귀가 필요한 경우 발생한다. |
| Main Flow | 1. system이 forward direction command를 결정한다. 2. system이 `MotorDriver`에 forward movement를 요청한다. 3. motor가 forward 상태로 동작한다. |
| Alternative / Exception | `StandbyMode`, `LowBatteryMode`, off state에서는 movement가 발생하지 않는다. |
| Related FR | FR-UC3-01 |
| Source | source: project artifact |

### UC4. Start Cleaning

| Field | Content |
|---|---|
| Actor | `CleanerDriver` |
| Purpose | cleaner가 cleaning을 수행한다. |
| Precondition | system이 `NormalMode` 또는 `BoostMode`이다. |
| Triggering incoming operation | `startButtonPressed()`로 `NormalMode`에 진입하거나, `dustDetected()`로 `BoostMode`에 진입하거나, `obstacleDetected(direction)` 처리 후 cleaning을 재개할 때 발생한다. |
| Main Flow | 1. system이 cleaner activation command를 전달한다. 2. cleaner가 normal 또는 boost setting으로 cleaning을 시작한다. |
| Alternative / Exception | `StandbyMode`, `LowBatteryMode`, off state에서는 cleaner가 stopped 상태여야 한다. |
| Related FR | FR-UC4-01 |
| Source | source: project artifact |

### UC5. Avoid Obstacle

| Field | Content |
|---|---|
| Actor | `ObstacleSensorDriver` |
| Purpose | obstacle detection 상황에서 collision을 회피한다. |
| Precondition | system이 `NormalMode` 또는 `BoostMode`이다. |
| Main Flow | 1. `ObstacleSensorDriver`가 front/left/right obstacle signal을 system에 전달한다. 2. system이 avoidance direction을 결정한다. 3. system이 selected direction으로 movement command를 전달한다. 4. 가능한 경우 movement와 cleaning을 재개한다. |
| Alternative / Exception | front blocked and left free -> left turn; front blocked and right free -> right turn; front/left/right blocked -> backward movement 후 available direction 탐색. |
| Related FR | FR-UC5-01 ~ FR-UC5-06 |
| Source | source: project artifact |

### UC6. Adjust Boost Mode

| Field | Content |
|---|---|
| Actor | `DustSensorDriver`, `DigitalClockTick` |
| Purpose | dust 감지 시 cleaning output을 강화하고 일정 시간 후 normal cleaning으로 복귀한다. |
| Precondition | system이 `NormalMode` 상태이다. |
| Main Flow | 1. `DustSensorDriver`가 dust signal을 system에 전달한다. 2. system mode가 `BoostMode`로 전환된다. 3. cleaner가 boost setting으로 동작한다. 4. boost duration이 만료되면 system은 `NormalMode`로 복귀한다. |
| Alternative / Exception | `BoostMode`에서 dust signal이 반복되면 implementation policy에 따라 boost 유지 또는 timer reset/extension 가능성이 있다. 현재 구현은 즉시 `BoostMode`를 유지한다. |
| Related FR | FR-UC6-01 ~ FR-UC6-04 |
| Source | source: project artifact; repeated dust policy source: implementation/test |

### UC7. Set Stand-by Mode

| Field | Content |
|---|---|
| Actor | `User`, `StartButton` |
| Purpose | active cleaning mode에서 standby state로 전환한다. |
| Precondition | system이 `NormalMode` 또는 `BoostMode`이다. |
| Main Flow | 1. User가 `StartButton`을 누른다. 2. system이 current mode를 확인한다. 3. system이 `StandbyMode`로 전환한다. 4. motor와 cleaner를 stop한다. |
| Alternative / Exception | `LowBatteryMode`에서는 start request가 ignored될 수 있다(source: implementation/test). |
| Related FR | FR-UC7-01 |
| Source | source: project artifact; low-battery exception source: implementation/test |

### UC8. Stop Moving

| Field | Content |
|---|---|
| Actor | `MotorDriver` |
| Purpose | movement를 stop한다. |
| Precondition | system이 `StandbyMode`, `LowBatteryMode`, 또는 off state로 전환된다. |
| Triggering incoming operation | `startButtonPressed()`, `powerButtonPressed()`, `lowBatteryDetected()` 중 하나로 stop-moving 조건이 발생한다. |
| Main Flow | 1. system이 motor stop command를 전달한다. 2. motor가 stopped state가 된다. |
| Alternative / Exception | N/A |
| Related FR | FR-UC8-01 ~ FR-UC8-03 |
| Source | source: project artifact |

### UC9. Stop Cleaning

| Field | Content |
|---|---|
| Actor | `CleanerDriver` |
| Purpose | cleaning을 stop한다. |
| Precondition | system이 `StandbyMode`, `LowBatteryMode`, 또는 off state로 전환된다. |
| Triggering incoming operation | `startButtonPressed()`, `powerButtonPressed()`, `lowBatteryDetected()` 중 하나로 stop-cleaning 조건이 발생한다. |
| Main Flow | 1. system이 cleaner stop command를 전달한다. 2. cleaner가 off state가 된다. |
| Alternative / Exception | N/A |
| Related FR | FR-UC9-01 ~ FR-UC9-03 |
| Source | source: project artifact |

### UC10. Charge Battery

| Field | Content |
|---|---|
| Actor | `User`, `BatteryDriver` |
| Purpose | battery를 charging하여 system이 정상 동작 가능하도록 한다. |
| Precondition | system이 `NormalMode` 또는 `BoostMode`가 아니며, charging 가능한 상태이다. |
| Main Flow | 1. User가 charging start를 요청한다. 2. system이 charging allowed condition을 확인한다. 3. system이 safe non-cleaning state를 유지하거나 진입한다. 4. battery charging이 시작된다. |
| Alternative / Exception | charging이 허용되지 않으면 operation은 no-op이다. 현재 구현은 off state charging도 허용한다. |
| Related FR | FR-UC10-01, FR-UC10-02 |
| Source | source: project artifact; off charging behavior source: implementation/test |

### UC11. Turn Off System

| Field | Content |
|---|---|
| Actor | `User`, `PowerButton` |
| Purpose | system 동작을 종료하고 hardware-related components를 off/idle state로 전환한다. |
| Precondition | system이 on 상태이다. |
| Main Flow | 1. User가 `PowerButton`을 누른다. 2. system이 motor와 cleaner를 stop한다. 3. sensor/driver state를 deactivate 또는 idle 처리한다. 4. system이 off state가 된다. |
| Alternative / Exception | charging 중이면 charging state도 off로 전환된다(source: implementation/test). |
| Related FR | FR-UC11-01, FR-UC11-02 |
| Source | source: project artifact; charging exception source: implementation/test |

### UC12. Turn Left

| Field | Content |
|---|---|
| Actor | `MotorDriver` |
| Purpose | obstacle avoidance 결과에 따라 left direction으로 이동한다. |
| Precondition | system이 `NormalMode` 또는 `BoostMode`이고 left direction이 available하다. |
| Triggering incoming operation | `obstacleDetected(direction)` 처리 중 avoidance direction이 `LEFT`로 결정될 때 발생한다. |
| Main Flow | 1. system이 avoidance direction으로 `LEFT`를 선택한다. 2. system이 `MotorDriver`에 `turnLeft()`를 요청한다. 3. motor direction이 left로 변경되고 forward movement를 재개한다. |
| Alternative / Exception | N/A |
| Related FR | FR-UC12-01, FR-UC12-02 |
| Source | source: project artifact |

### UC13. Turn Right

| Field | Content |
|---|---|
| Actor | `MotorDriver` |
| Purpose | obstacle avoidance 결과에 따라 right direction으로 이동한다. |
| Precondition | system이 `NormalMode` 또는 `BoostMode`이고 right direction이 available하다. |
| Triggering incoming operation | `obstacleDetected(direction)` 처리 중 avoidance direction이 `RIGHT`로 결정될 때 발생한다. |
| Main Flow | 1. system이 avoidance direction으로 `RIGHT`를 선택한다. 2. system이 `MotorDriver`에 `turnRight()`를 요청한다. 3. motor direction이 right로 변경되고 forward movement를 재개한다. |
| Alternative / Exception | N/A |
| Related FR | FR-UC13-01, FR-UC13-02 |
| Source | source: project artifact |

### UC14. Move Backward

| Field | Content |
|---|---|
| Actor | `MotorDriver` |
| Purpose | front/left/right가 모두 blocked된 경우 backward movement로 회피한다. |
| Precondition | system이 `NormalMode` 또는 `BoostMode`이고 front/left/right directions가 blocked 상태이다. |
| Triggering incoming operation | `obstacleDetected(direction)` 처리 중 avoidance direction이 `BACK`으로 결정될 때 발생한다. |
| Main Flow | 1. system이 avoidance direction으로 `BACK`을 선택한다. 2. system이 `MotorDriver`에 `moveBackward()`를 요청한다. 3. 이후 available direction이 생기면 left/right turn을 수행한다. |
| Alternative / Exception | available side direction이 없으면 backward state를 유지할 수 있다(source: implementation/test). |
| Related FR | FR-UC14-01, FR-UC14-02 |
| Source | source: project artifact; no-side-clear behavior source: implementation/test |

### UC15. Enter Low Battery Mode

| Field | Content |
|---|---|
| Actor | `BatteryDriver` |
| Purpose | battery 부족 시 system을 safety mode로 전환한다. |
| Precondition | system이 on 상태이다. |
| Main Flow | 1. battery가 low-battery signal을 system에 전달한다. 2. system이 `LowBatteryMode`로 전환한다. 3. system이 motor와 cleaner를 stop한다. 4. low battery 상태에서는 start cleaning이 blocked된다. |
| Alternative / Exception | charging으로 battery level이 threshold를 초과하면 현재 구현은 `StandbyMode`로 recovery한다. |
| Related FR | FR-UC15-01, FR-UC15-02 |
| Source | source: project artifact; recovery policy source: implementation/test |

### UC16. Stop Charging

| Field | Content |
|---|---|
| Actor | `User`, `BatteryDriver` |
| Purpose | battery charging을 종료한다. |
| Precondition | charging 중이다. |
| Main Flow | 1. User가 charging stop을 요청한다. 2. system이 `BatteryDriver`에 stop charging command를 전달한다. 3. battery charging state가 off가 된다. |
| Alternative / Exception | charging 중이 아니면 no-op로 처리될 수 있다(source: implementation/test). |
| Related FR | FR-UC16-01, FR-UC16-02 |
| Source | source: project artifact; no-op behavior source: implementation/test |

## 9. System Sequence Diagrams in PlantUML

System Sequence Diagram(SSD)은 system boundary를 `:RVCSystem`으로 두고 actor/device에서 system으로 들어오는 incoming event만 표현한다. 따라서 `moveForward()`, `startCleaning()`, `stopMoving()`, `stopCleaning()`, `turnLeft()`, `turnRight()`, `moveBackward()`는 SSD의 system operation이 아니라 system에서 actuator로 나가는 output command이다.

PlantUML diagram은 rendering tool에서 diagram type이 섞이지 않도록 별도 `.puml` file로 분리되어 있다. `docs/ai-output/srs_diagrams.puml`은 `OutputCommandOverview`만 포함한다.

| Diagram name | PlantUML file | 구분 | 포함 operation / command |
|---|---|---|---|
| `SSD_UserControl` | `docs/ai-output/srs_ssd_user_control.puml` | true SSD | `powerButtonPressed()`, `startButtonPressed()`, `chargeBattery()`, `stopCharging()` |
| `SSD_SensorEvents` | `docs/ai-output/srs_ssd_sensor_events.puml` | true SSD | `dustDetected()`, `obstacleDetected(direction)`, `lowBatteryDetected()`, `lowBatteryCleared()`, `timerExpired()`, `chargingTick()` |
| `OutputCommandOverview` | `docs/ai-output/srs_diagrams.puml` | outbound command overview, not a true SSD | `moveForward()`, `startCleaning()`, `stopMoving()`, `stopCleaning()`, `turnLeft()`, `turnRight()`, `moveBackward()` |

`OutputCommandOverview`는 요구사항 추적을 돕기 위한 command overview이다. 이 diagram은 `:RVCSystem -> MotorDriver/CleanerDriver` 방향의 outbound interaction을 보여주므로 true SSD로 분류하지 않는다.

## 10. System Operations

System Operation은 actor/device/event가 `:RVCSystem` boundary로 보내는 incoming event이다. 아래 표는 SRS에서 사용하는 true system operation만 포함한다.

| System Operation | Related UC | Trigger | Source | Implementation note |
|---|---|---|---|---|
| `powerButtonPressed()` | UC1, UC11 | User presses `PowerButton` | source: project artifact | Repo method: `Controller::powerButtonPressed()` |
| `startButtonPressed()` | UC2, UC7 | User presses `StartButton` | source: project artifact | Repo method: `Controller::startButtonPressed()` |
| `chargeBattery()` | UC10 | User starts charging | source: project artifact | Repo method: `Controller::chargeBattery()` |
| `stopCharging()` | UC16 | User stops charging | source: project artifact | Repo method: `Controller::stopCharging()` |
| `lowBatteryDetected()` | UC15 | Battery low-battery signal | source: project artifact | Repo method: `Controller::lowBatteryDetected()` |
| `lowBatteryCleared()` | UC15 recovery | Battery state recovers above low threshold | source: project artifact | Repo method: `Controller::lowBatteryCleared()` |
| `dustDetected()` | UC6 | Dust signal | source: project artifact | Repo method: `Controller::dustDetected()` |
| `obstacleDetected(direction)` | UC5, UC12, UC13, UC14 | Obstacle information | source: project artifact | Repo method: `Controller::obstacleDetected(const bool direction[3])` |
| `timerExpired()` | UC6 | Boost timer expires | source: project artifact | Requirement/system operation name; repo-level helper is `Controller::timerExpiredNow()` and simulator command is `timer-expired` |
| `clockTick()` | Common periodic behavior, UC2 | `DigitalClockTick` | source: project artifact | Artifact-only / not implemented as a general repo operation |
| `chargingTick()` | UC10, UC15 recovery, UC16 support | Charging loop/tick event | source: implementation/test | Repo method: `Controller::chargingTick()`; partially substitutes artifact-level `clockTick()` for charging behavior only |

## 11. Output Commands / Actuator Commands

Output command는 `:RVCSystem` 내부 decision 결과로 actuator/driver에 전달되는 outbound command이다. 이 command들은 use case behavior를 실현하지만, SSD 관점의 incoming system operation은 아니다.

| Output Command | Target | Related UC | Triggering incoming operation | Source |
|---|---|---|---|---|
| `moveForward()` | `MotorDriver` | UC3, UC5, UC12, UC13 | `startButtonPressed()`, `obstacleDetected(direction)`, mode entry/recovery | source: project artifact; implementation/test |
| `startCleaning()` | `CleanerDriver` | UC4, UC5, UC6 | `startButtonPressed()`, `dustDetected()`, obstacle recovery/mode entry | source: project artifact; implementation/test |
| `stopMoving()` | `MotorDriver` | UC8 | `startButtonPressed()`, `powerButtonPressed()`, `lowBatteryDetected()` | source: project artifact; implementation/test |
| `stopCleaning()` | `CleanerDriver` | UC9 | `startButtonPressed()`, `powerButtonPressed()`, `lowBatteryDetected()` | source: project artifact; implementation/test |
| `turnLeft()` | `MotorDriver` | UC12 | `obstacleDetected(direction)` | source: project artifact; implementation/test |
| `turnRight()` | `MotorDriver` | UC13 | `obstacleDetected(direction)` | source: project artifact; implementation/test |
| `moveBackward()` | `MotorDriver` | UC14 | `obstacleDetected(direction)` | source: project artifact; implementation/test |

## 12. Operation Contracts

### 12.1 `powerButtonPressed()`

| 항목 | 내용 |
|---|---|
| Related UC | UC1, UC11 |
| Precondition | system은 off 또는 on 상태이다. |
| Postcondition | off -> `StandbyMode`; on -> off state. |
| Side effects | power on 시 drivers initialize; power off 시 motor/cleaner stop, sensors deactivate, charging off. |
| Source | source: project artifact; side effect source: implementation/test |

### 12.2 `startButtonPressed()`

| 항목 | 내용 |
|---|---|
| Related UC | UC2, UC7 |
| Precondition | system이 on이고 current mode가 존재한다. |
| Postcondition | `StandbyMode` -> `NormalMode`; `NormalMode`/`BoostMode` -> `StandbyMode`; `LowBatteryMode`는 유지. |
| Output commands | Normal 진입 시 `moveForward()`, `startCleaning()`; Standby 진입 시 `stopMoving()`, `stopCleaning()`. |
| Source | source: project artifact; low-battery/charging behavior source: implementation/test |

### 12.3 `chargeBattery()`

| 항목 | 내용 |
|---|---|
| Related UC | UC10 |
| Precondition | charging이 allowed 상태이다. Artifact 기준으로 `NormalMode`/`BoostMode`가 아니어야 한다. |
| Postcondition | charging state가 on이 되고 battery level이 증가할 수 있다. Full battery이면 charging이 종료된다. |
| Side effects | 현재 구현은 `chargeBattery()` 호출 시 charging loop 1회를 수행한다. `LowBatteryMode`에서 battery level이 threshold를 초과하면 recovery가 발생한다. |
| Source | source: project artifact; charging tick behavior source: implementation/test |

### 12.4 `chargingTick()`

| 항목 | 내용 |
|---|---|
| Related UC | UC10, UC15 recovery, UC16 support |
| Precondition | charging state가 on이다. |
| Postcondition | battery level이 charging step만큼 증가할 수 있으며, full battery이면 charging state가 off가 된다. |
| Artifact relation | artifact-level `clockTick()` 중 charging behavior에 해당하는 구현/test operation이다. 일반 sensor/battery periodic check 전체를 대체하지는 않는다. |
| Source | source: implementation/test |

### 12.5 `stopCharging()`

| 항목 | 내용 |
|---|---|
| Related UC | UC16 |
| Precondition | charging 중이거나 charging stop request가 들어온 상태이다. |
| Postcondition | charging state가 off가 된다. |
| Side effects | battery level은 변경하지 않는다. |
| Source | source: project artifact; idempotent behavior source: implementation/test |

### 12.6 `dustDetected()`

| 항목 | 내용 |
|---|---|
| Related UC | UC6 |
| Precondition | system이 `NormalMode`이고 charging 중이 아니다. |
| Postcondition | system이 `BoostMode`가 되고 cleaner가 boost setting이 된다. |
| Output commands | `CleanerDriver` boost setting command가 발생한다. |
| Source | source: project artifact; charging guard source: implementation/test |

### 12.7 `timerExpired()`

| 항목 | 내용 |
|---|---|
| Related UC | UC6 |
| Precondition | boost duration이 만료된다. |
| Postcondition | `BoostMode`이면 `NormalMode`로 복귀한다. higher-priority mode이면 해당 mode를 유지한다. |
| Implementation mapping | requirement/system operation name은 `timerExpired()`이고, 현재 repo의 public helper method는 `Controller::timerExpiredNow()`이다. |
| Source | source: project artifact; repo mapping source: implementation/test |

### 12.8 `lowBatteryDetected()`

| 항목 | 내용 |
|---|---|
| Related UC | UC15 |
| Precondition | system이 on 상태이다. |
| Postcondition | system이 `LowBatteryMode`로 전환된다. |
| Output commands | `stopMoving()`, `stopCleaning()` |
| Source | source: project artifact |

### 12.9 `lowBatteryCleared()`

| 항목 | 내용 |
|---|---|
| Related UC | UC15 recovery |
| Precondition | system이 `LowBatteryMode`이고 battery condition이 recovery되었다. |
| Postcondition | system은 available normal operating mode로 회복한다. 현재 구현은 `StandbyMode`로 회복한다. |
| Side effects | charging은 계속될 수 있다(source: implementation/test). |
| Source | source: project artifact; concrete policy source: implementation/test |

### 12.10 `obstacleDetected(direction)`

| 항목 | 내용 |
|---|---|
| Related UC | UC5, UC12, UC13, UC14 |
| Precondition | system이 `NormalMode` 또는 `BoostMode`이고 direction input이 valid하다. |
| Postcondition | obstacle state에 따라 forward/left/right/back output command가 선택된다. |
| Output commands | `moveForward()`, `turnLeft()`, `turnRight()`, `moveBackward()` 중 하나 이상 |
| Side effects | artifact 기준으로 avoidance 중 cleaner stop 후 resume이 요구된다. 현재 구현은 cleaner mode를 유지한다. |
| Source | source: project artifact; current behavior source: implementation/test |

### 12.11 `clockTick()`

| 항목 | 내용 |
|---|---|
| Related UC | Common, UC2 |
| Precondition | system이 active 상태이다. |
| Postcondition | system은 sensor/battery state를 주기적으로 확인하고 필요한 operation을 유발한다. |
| Implementation status | artifact-only / not implemented. 현재 repository에는 일반 `clockTick()` operation이 없다. |
| Source | source: project artifact |

## 13. Domain Model / Glossary

| Term | Meaning | Source |
|---|---|---|
| `RVC System` | user/sensor/battery event를 받아 cleaning 및 movement를 제어하는 system boundary | source: project artifact |
| `OperatingMode` | system operating state를 나타내는 abstraction | source: project artifact |
| `Off` | active mode가 없는 power-off state | source: project artifact; implementation/test |
| `StandbyMode` | power-on 이후 대기 state. motor와 cleaner가 stopped 상태이다. | source: project artifact |
| `NormalMode` | regular cleaning mode. motor와 cleaner가 active 상태이다. | source: project artifact |
| `BoostMode` | dust detection 후 high-power cleaning을 수행하는 temporary mode | source: project artifact |
| `LowBatteryMode` | battery 부족 시 cleaning/movement를 제한하는 safety mode | source: project artifact |
| `BatteryDriver` | battery level, charging, low-battery threshold를 관리하는 external/device abstraction | source: project artifact |
| `DustSensorDriver` | dust signal을 제공하는 sensor abstraction | source: project artifact |
| `ObstacleSensorDriver` | front/left/right obstacle 정보를 제공하는 sensor abstraction | source: project artifact |
| `MotorDriver` | forward, left, right, backward, stop movement command를 수행하는 actuator abstraction | source: project artifact |
| `CleanerDriver` | cleaning start/stop 및 boost cleaning behavior를 수행하는 actuator abstraction | source: project artifact |
| `DigitalClockTick` | periodic behavior와 timer expiry를 나타내는 event concept | source: project artifact |
| `Direction` | movement/avoidance direction. 현재 구현 값은 `FRONT`, `LEFT`, `RIGHT`, `BACK`이다. | source: implementation/test |

## 14. FR - Use Case - System Operation - Unit Test - System Test Traceability Table

| FR ID | UC | Incoming System Operation / Output Command Evidence | Unit Test Evidence | System Test Evidence | Support |
|---|---|---|---|---|---|
| FR-001 | Common | mode state accessors | `ButtonTest.cpp`, `DustDetectedTest.cpp`, `ControllerEnterLowBatteryModeTest.cpp` | TC01-TC45 mode expectations | Supported |
| FR-002 | Common | `clockTick()` artifact-only / not implemented | None | None | Gap |
| FR-003 | UC15 recovery | `lowBatteryCleared()`, `chargeBattery()` | `ControllerEnterLowBatteryModeTest.cpp`, `ControllerChargeBatteryTest.cpp` | TC18, TC45 | Supported with Standby policy |
| FR-004 | UC10, UC16 | `chargeBattery()`, `chargingTick()`, `stopCharging()` | `ControllerChargeBatteryTest.cpp`, `ControllerStopChargingTest.cpp` | TC08-TC14, TC45 | Supported |
| FR-UC1-01 | UC1 | `powerButtonPressed()` | `ControllerTurnOnSystemTest.cpp`, `ButtonTest.cpp` | TC01 | Supported |
| FR-UC1-02 | UC1 | `powerButtonPressed()` | `ControllerTurnOnSystemTest.cpp`, `ButtonTest.cpp` | TC01 | Supported |
| FR-UC1-03 | UC1 | `powerButtonPressed()` | `ControllerTurnOnSystemTest.cpp`, `ControllerObstacleDetectedTest.cpp` | TC01, TC03 | Supported |
| FR-UC1-04 | UC1 | `powerButtonPressed()` | `ControllerTurnOnSystemTest.cpp`, `ButtonTest.cpp` | TC01 | Supported |
| FR-UC2-01 | UC2 | `startButtonPressed()` | `ButtonTest.cpp`, `ControllerTurnOnSystemTest.cpp` | TC03 | Supported |
| FR-UC2-02 | UC2 | `clockTick()` artifact-only / not implemented | None | None | Gap |
| FR-UC3-01 | UC3 | Trigger: `startButtonPressed()` or `obstacleDetected(direction)`; output command: `moveForward()` | `ButtonTest.cpp`, `ControllerObstacleDetectedTest.cpp` | TC03, TC24-TC43 | Supported |
| FR-UC4-01 | UC4 | Trigger: `startButtonPressed()` or `dustDetected()`; output command: `startCleaning()` / boost setting | `ButtonTest.cpp`, `DustDetectedTest.cpp` | TC03, TC19, TC24-TC43 | Supported |
| FR-UC5-01 | UC5 | `obstacleDetected(direction)` | `ControllerObstacleDetectedTest.cpp` | TC24-TC43 | Partial |
| FR-UC5-02 | UC5 | `obstacleDetected(direction)` | `ControllerObstacleDetectedTest.cpp` | TC24-TC43 | Partial |
| FR-UC5-03 | UC5 | Trigger: `obstacleDetected(direction)`; expected output command: `stopCleaning()` during avoidance | None matching artifact behavior | None matching artifact behavior | Gap/Inconsistent |
| FR-UC5-04 | UC5 | `obstacleDetected(direction)` | `ControllerObstacleDetectedTest.cpp` | TC24-TC43 | Supported |
| FR-UC5-05 | UC5 | Trigger: `obstacleDetected(direction)`; output commands: `turnLeft()`, `turnRight()`, `moveBackward()` | `ControllerObstacleDetectedTest.cpp` | TC24-TC43 | Supported |
| FR-UC5-06 | UC5 | Trigger: `obstacleDetected(direction)`; output commands: `moveForward()`, `startCleaning()`/cleaner remains active | `ControllerObstacleDetectedTest.cpp` | TC24-TC43 | Supported |
| FR-UC6-01 | UC6 | `dustDetected()` | `DustDetectedTest.cpp` | TC19-TC23, TC44 | Partial |
| FR-UC6-02 | UC6 | `dustDetected()` | `DustDetectedTest.cpp` | TC19-TC23, TC44 | Partial |
| FR-UC6-03 | UC6 | `dustDetected()` | `DustDetectedTest.cpp` | TC19-TC20 | Supported |
| FR-UC6-04 | UC6 | `timerExpired()` | `DustDetectedTest.cpp`, `ControllerEnterLowBatteryModeTest.cpp` | TC23, TC44 | Supported |
| FR-UC7-01 | UC7 | `startButtonPressed()` | `ButtonTest.cpp` | TC06-TC07 | Supported |
| FR-UC8-01 | UC8 | Trigger: `startButtonPressed()`; output command: `stopMoving()` | `ButtonTest.cpp`, `ControllerTurnOffSystemTest.cpp` | TC06-TC07 | Supported |
| FR-UC8-02 | UC8 | Trigger: `lowBatteryDetected()`; output command: `stopMoving()` | `ControllerEnterLowBatteryModeTest.cpp` | TC15-TC18, TC45 | Supported |
| FR-UC8-03 | UC8 | Trigger: `powerButtonPressed()`; output command: `stopMoving()` | `ControllerTurnOffSystemTest.cpp`, `ButtonTest.cpp` | TC02 | Supported |
| FR-UC9-01 | UC9 | Trigger: `startButtonPressed()`; output command: `stopCleaning()` | `ButtonTest.cpp`, `ControllerTurnOffSystemTest.cpp` | TC06-TC07 | Supported |
| FR-UC9-02 | UC9 | Trigger: `lowBatteryDetected()`; output command: `stopCleaning()` | `ControllerEnterLowBatteryModeTest.cpp` | TC15-TC18, TC45 | Supported |
| FR-UC9-03 | UC9 | Trigger: `powerButtonPressed()`; output command: `stopCleaning()` | `ControllerTurnOffSystemTest.cpp`, `ButtonTest.cpp` | TC02 | Supported |
| FR-UC10-01 | UC10 | `chargeBattery()` | `ControllerChargeBatteryTest.cpp` | TC08-TC12 | Supported |
| FR-UC10-02 | UC10 | `chargeBattery()` | `ControllerChargeBatteryTest.cpp` | TC08-TC12, TC45 | Partial/Inconsistent |
| FR-UC11-01 | UC11 | `powerButtonPressed()` | `ControllerTurnOffSystemTest.cpp`, `ButtonTest.cpp` | TC02 | Supported |
| FR-UC11-02 | UC11 | `powerButtonPressed()` | `ControllerTurnOffSystemTest.cpp`, `ButtonTest.cpp` | TC02 | Supported |
| FR-UC12-01 | UC12 | Trigger: `obstacleDetected(direction)`; output command: `turnLeft()` | `ControllerObstacleDetectedTest.cpp` | TC24, TC28-TC29, TC34, TC38-TC39 | Supported |
| FR-UC12-02 | UC12 | Trigger: `obstacleDetected(direction)`; output commands: `turnLeft()`, `moveForward()` | `ControllerObstacleDetectedTest.cpp` | TC24, TC28-TC29, TC34, TC38-TC39 | Supported |
| FR-UC13-01 | UC13 | Trigger: `obstacleDetected(direction)`; output command: `turnRight()` | `ControllerObstacleDetectedTest.cpp` | TC30, TC36, TC40 | Supported |
| FR-UC13-02 | UC13 | Trigger: `obstacleDetected(direction)`; output commands: `turnRight()`, `moveForward()` | `ControllerObstacleDetectedTest.cpp` | TC30, TC36, TC40 | Supported |
| FR-UC14-01 | UC14 | Trigger: `obstacleDetected(direction)`; output command: `moveBackward()` | `ControllerObstacleDetectedTest.cpp` | TC26, TC28, TC32-TC33, TC36, TC38, TC42-TC43 | Supported |
| FR-UC14-02 | UC14 | Trigger: `obstacleDetected(direction)`; output commands: `turnLeft()`/`turnRight()` after backward | `ControllerObstacleDetectedTest.cpp` | TC24, TC29-TC30, TC34, TC39-TC40 | Supported |
| FR-UC15-01 | UC15 | `lowBatteryDetected()` | `ControllerEnterLowBatteryModeTest.cpp` | TC15-TC18, TC45 | Partial |
| FR-UC15-02 | UC15 | `lowBatteryDetected()` | `ControllerEnterLowBatteryModeTest.cpp` | TC15-TC18, TC45 | Supported |
| FR-UC16-01 | UC16 | `stopCharging()` direct request; `chargingTick()` auto-stops at full battery | `ControllerStopChargingTest.cpp` | TC45 direct `stop-charge`; TC13 automatic full-charge stop via `charge-tick` | Supported |
| FR-UC16-02 | UC16 | `stopCharging()` | `ControllerStopChargingTest.cpp` | TC45 | Partial |

## 15. AI Inspection Findings

### 15.1 Supported Areas

- UC1, UC2, UC3, UC4, UC6, UC7, UC8, UC9, UC10, UC11, UC12, UC13, UC14, UC15, UC16의 핵심 behavior는 현재 구현 및 테스트 evidence로 상당 부분 확인된다.
- Unit tests는 system operation 단위로 구성되어 있으며, `ButtonTest`, `ControllerTurnOnSystemTest`, `ControllerTurnOffSystemTest`, `ControllerChargeBatteryTest`, `ControllerStopChargingTest`, `ControllerEnterLowBatteryModeTest`, `ControllerObstacleDetectedTest`, `DustDetectedTest`가 주요 evidence이다.
- System tests는 `system_tests/tc/TC01_*.rvc`부터 `TC45_*.rvc`까지 simulator script로 제공되며, `cmake/SystemTests.cmake`와 `.github/workflows/ooi-system-test.yml`에 의해 CTest/GitHub Actions와 연결된다.

### 15.2 Gaps and Inconsistencies

| Finding | 설명 | Impact |
|---|---|---|
| General `DigitalClockTick` gap | project artifact는 periodic sensor/battery check를 요구하지만 현재 구현에는 일반 `clockTick()` operation이 없다. | FR-002, FR-UC2-02, N2.1은 Gap으로 표시 |
| Sensor signal abstraction mismatch | artifact는 `DustSensorDriver`, `ObstacleSensorDriver`, `BatteryDriver`가 signal을 system에 보내는 흐름을 설명하지만 현재 구현과 tests는 `Controller` operation을 직접 호출한다. | FR-UC5-01/02, FR-UC6-01/02, FR-UC15-01은 Partial |
| Obstacle avoidance cleaner stop mismatch | artifact는 avoidance 중 cleaner stop을 요구하지만 현재 code/tests는 cleaner가 계속 on 상태임을 기대한다. | FR-UC5-03은 Gap/Inconsistent |
| Charging safe state ambiguity | artifact는 charging 시작 시 safe non-cleaning mode 진입/유지를 요구한다. 현재 구현은 `NormalMode`/`BoostMode` charging을 거부하고 off-state charging을 허용한다. | FR-UC10-02는 Partial/Inconsistent |
| Stop charging mode transition ambiguity | artifact는 charging stop 이후 appropriate non-charging state transition을 요구한다. 현재 구현은 charging flag만 off로 만들고 mode transition은 별도 수행하지 않는다. | FR-UC16-02는 Partial |
| Low battery recovery policy | artifact/OOD는 context-dependent recovery 가능성을 언급하지만 현재 구현은 threshold 초과 후 `StandbyMode`로 회복한다. | SRS에는 requirement를 유지하고 현재 policy를 note로 기록 |
| `Direction` value mismatch | OOD summary는 `Stop` 포함 가능성을 언급하지만 현재 구현의 `Direction`은 `FRONT`, `LEFT`, `RIGHT`, `BACK`이며 stop은 `MotorDriver` status로 표현된다. | Glossary와 SDD 작성 시 주의 필요 |
| Performance NFR not verified | N1.1, N1.2, N1.3은 현재 unit/system tests에서 timing assertion으로 검증되지 않는다. | NFR 검증 Gap |
| `Controller` implementation ownership | OOI summary의 code slide는 value member 가능성을 언급하지만 현재 최종 code는 raw pointer ownership과 manual deletion을 사용한다. | SDD에서 implementation evidence로 명시 필요 |

### 15.3 Generation Notes

- 본 SRS의 requirement text는 project artifact source를 기준으로 유지하였다.
- 현재 implementation/test에서 확인된 예외 또는 정책은 `source: implementation/test`로만 보충하였다.
- PlantUML SSD는 원본 artifact 그림의 직접 복사본이 아니라 artifact summary와 repository evidence 기반 재구성이다.
