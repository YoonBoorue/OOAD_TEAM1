# WEEK1 Summary - RVC Control SW Project Source

> Purpose: This markdown file is a readable AI-input version of `[T1] WEEK1_V2.pdf`.
> It summarizes the project overview, functional requirements, use cases, non-functional requirements, and CI/CD environment from the Week 1 artifact.
> Source type: project artifact summary.

---

## 1. Document Scope

The Week 1 artifact defines the initial requirements and project environment for the RVC Control SW team project.

Main sections:

1. Functional Requirements
2. Use Cases
3. Non-Functional Requirements
4. CI/CD Environment

---

## 2. Core Features

### 2.1 Cleaning and Movement Control

The RVC system performs automatic cleaning by moving through a room while operating the cleaner.

The system should:

- Move forward while cleaning.
- Detect obstacles using front, left, and right obstacle sensors.
- Avoid obstacles by changing direction.
- If front, left, and right directions are blocked, move backward and then turn left or right if a direction becomes available.

### 2.2 Cleaning Mode Control

The system controls cleaning modes based on user input and sensor input.

The system should:

- Start in `StandbyMode` after power-on initialization.
- Enter `NormalMode` when the user presses the start button from standby.
- Enter `BoostMode` when dust is detected during cleaning.
- Return from `BoostMode` to `NormalMode` after the boost duration expires.
- Enter `LowBatteryMode` when battery level becomes low.

### 2.3 Sensor Data Processing

The system receives and processes sensor data in real time.

Relevant input sources:

- `PowerButton`
- `StartButton`
- `BatteryDriver`
- `DustSensorDriver`
- `ObstacleSensorDriver`
- `DigitalClockTick`

---

## 3. Functional Requirements

### 3.1 Common / Hidden Requirements

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-001 | The system shall support the following operating states/modes: `Off`, `StandbyMode`, `NormalMode`, `BoostMode`, and `LowBatteryMode`. | Common | Hidden |
| FR-002 | The system shall periodically check sensor and battery-related state while active. | Common | Hidden |
| FR-003 | The system shall recover from `LowBatteryMode` to an available normal operating mode when the low-battery condition is cleared. | Common | Hidden |
| FR-004 | The system shall manage battery state such as charging and non-charging state. | Common | Hidden |

### 3.2 UC1 - Turn On System

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC1-01 | When the user presses the `PowerButton` while the system is off, the system shall turn on. | UC1. Turn On System | Evident |
| FR-UC1-02 | When the system turns on, it shall initialize hardware-related components such as obstacle sensor driver, dust sensor driver, motor driver, and cleaner driver. | UC1. Turn On System | Evident |
| FR-UC1-03 | When the system turns on, it shall set the default movement direction to `Forward`. | UC1. Turn On System | Evident |
| FR-UC1-04 | When the system turns on, it shall set the operating mode to `StandbyMode`. | UC1. Turn On System | Evident |

### 3.3 UC2 - Set Cleaning Mode

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC2-01 | When the system is in `StandbyMode` and the user presses the `StartButton`, the system shall enter `NormalMode`. | UC2. Set Cleaning Mode | Evident |
| FR-UC2-02 | In `NormalMode`, the system shall periodically operate cleaner and sensor-related drivers according to the digital clock tick. | UC2. Set Cleaning Mode | Hidden |

### 3.4 UC3 - Move Forward

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC3-01 | In `NormalMode` or `BoostMode`, the system shall command the motor to move forward. | UC3. Move Forward | Evident |

### 3.5 UC4 - Start Cleaning

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC4-01 | In `NormalMode` or `BoostMode`, the system shall operate the cleaner continuously while cleaning is active. | UC4. Start Cleaning | Evident |

### 3.6 UC5 - Avoid Obstacle

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC5-01 | The `ObstacleSensorDriver` shall detect obstacles in the front, left, and right directions. | UC5. Avoid Obstacle | Evident |
| FR-UC5-02 | The `ObstacleSensorDriver` shall send obstacle information to the system. | UC5. Avoid Obstacle | Evident |
| FR-UC5-03 | During obstacle avoidance, the system shall request the `CleanerDriver` to stop cleaning. | UC5. Avoid Obstacle | Evident |
| FR-UC5-04 | The system shall determine an obstacle avoidance direction based on obstacle sensor input. | UC5. Avoid Obstacle | Evident |
| FR-UC5-05 | If the current movement direction is blocked, the system shall choose an available direction such as left or right when possible. | UC5. Avoid Obstacle | Evident |
| FR-UC5-06 | After obstacle avoidance, if the current mode is `NormalMode` or `BoostMode`, the system shall resume movement and cleaning. | UC5. Avoid Obstacle | Evident |

### 3.7 UC6 - Adjust Boost Mode

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC6-01 | The `DustSensorDriver` shall detect dust. | UC6. Adjust Boost Mode | Evident |
| FR-UC6-02 | The `DustSensorDriver` shall send a dust signal to the system. | UC6. Adjust Boost Mode | Evident |
| FR-UC6-03 | When the system receives a dust signal during cleaning, it shall enter `BoostMode`. | UC6. Adjust Boost Mode | Evident |
| FR-UC6-04 | After the boost duration expires, the system shall return from `BoostMode` to `NormalMode` if no higher-priority mode blocks the transition. | UC6. Adjust Boost Mode | Hidden |

### 3.8 UC7 - Set Stand-by Mode

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC7-01 | When the system is in `NormalMode` or `BoostMode` and the user presses the `StartButton`, the system shall enter `StandbyMode`. | UC7. Set Stand-by Mode | Evident |

### 3.9 UC8 - Stop Moving

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC8-01 | When the system enters `StandbyMode`, it shall stop the `MotorDriver`. | UC8. Stop Moving | Evident |
| FR-UC8-02 | When the system enters `LowBatteryMode`, it shall stop the `MotorDriver`. | UC8. Stop Moving | Evident |
| FR-UC8-03 | When the system enters the off state, it shall stop the `MotorDriver`. | UC8. Stop Moving | Evident |

### 3.10 UC9 - Stop Cleaning

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC9-01 | When the system enters `StandbyMode`, it shall stop the `CleanerDriver`. | UC9. Stop Cleaning | Evident |
| FR-UC9-02 | When the system enters `LowBatteryMode`, it shall stop the `CleanerDriver`. | UC9. Stop Cleaning | Evident |
| FR-UC9-03 | When the system enters the off state, it shall stop the `CleanerDriver`. | UC9. Stop Cleaning | Evident |

### 3.11 UC10 - Charge Battery

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC10-01 | When the user starts charging and charging is allowed, the system shall set the battery state to charging. | UC10. Charge Battery | Hidden |
| FR-UC10-02 | When charging begins, the system shall enter or remain in a safe non-cleaning mode such as `StandbyMode`. | UC10. Charge Battery | Hidden |

### 3.12 UC11 - Turn Off System

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC11-01 | When the user presses the `PowerButton` while the system is on, the system shall turn off. | UC11. Turn Off System | Evident |
| FR-UC11-02 | When the system turns off, it shall stop or deactivate hardware-related components. | UC11. Turn Off System | Evident |

### 3.13 UC12 - Turn Left

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC12-01 | The system shall request the `MotorDriver` to turn left when left direction is selected as the avoidance direction. | UC12. Turn Left | Evident |
| FR-UC12-02 | After turning left, the system shall return the movement direction to forward movement when appropriate. | UC12. Turn Left | Hidden |

### 3.14 UC13 - Turn Right

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC13-01 | The system shall request the `MotorDriver` to turn right when right direction is selected as the avoidance direction. | UC13. Turn Right | Evident |
| FR-UC13-02 | After turning right, the system shall return the movement direction to forward movement when appropriate. | UC13. Turn Right | Hidden |

### 3.15 UC14 - Move Backward

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC14-01 | If the current direction is blocked, the system shall request the `MotorDriver` to move backward. | UC14. Move Backward | Evident |
| FR-UC14-02 | If left or right direction becomes available after moving backward, the system shall turn in the available direction. | UC14. Move Backward | Evident |

### 3.16 UC15 - Enter Low Battery Mode

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC15-01 | When the battery level falls below 10%, the `BatteryDriver` shall send a low-battery signal to the system. | UC15. Enter Low Battery Mode | Evident |
| FR-UC15-02 | When the system receives a low-battery signal, it shall enter `LowBatteryMode`. | UC15. Enter Low Battery Mode | Evident |

### 3.17 UC16 - Stop Charging

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| FR-UC16-01 | When the user stops charging, the system shall stop battery charging. | UC16. Stop Charging | Evident |
| FR-UC16-02 | After charging stops, the system shall transition to an appropriate non-charging state, such as `StandbyMode`. | UC16. Stop Charging | Hidden |

---

## 4. Use Case List

| Group | Use Cases |
|---|---|
| Power | UC1. Turn On System, UC11. Turn Off System, UC10. Charge Battery, UC16. Stop Charging |
| Cleaning | UC4. Start Cleaning, UC9. Stop Cleaning, UC6. Adjust Boost Mode |
| Movement | UC3. Move Forward, UC5. Avoid Obstacle, UC8. Stop Moving, UC12. Turn Left, UC13. Turn Right, UC14. Move Backward |
| Mode Transition | UC2. Set Cleaning Mode, UC7. Set Stand-by Mode, UC15. Enter Low Battery Mode |

---

## 5. Actors and External Devices

| Actor / External Device | Role |
|---|---|
| User | Presses power/start buttons and starts/stops charging. |
| PowerButton | User input for turning system on/off. |
| StartButton | User input for starting or stopping cleaning mode. |
| BatteryDriver | Provides battery status, low-battery signal, and charging behavior. |
| ObstacleSensorDriver | Provides front/left/right obstacle information. |
| DustSensorDriver | Provides dust detection signal. |
| MotorDriver | Performs movement commands such as forward, left, right, backward, and stop. |
| CleanerDriver | Performs cleaner activation, cleaner stop, and boost-related cleaning behavior. |

---

## 6. Non-Functional Requirements

| ID | Requirement | Related Use Case | Category |
|---|---|---|---|
| N1.1 | Cleaning start/stop shall be completed within 2 seconds. | UC1, UC11 | Performance |
| N1.2 | Direction change shall begin within 0.5 seconds after obstacle detection. | UC5 | Performance |
| N1.3 | Transition to `BoostMode` shall occur within 1 second after dust detection. | UC6 | Performance |
| N2.1 | The system shall operate periodically according to the `DigitalClockTick`. | All | Operating Environment |
| N3.1 | The low-battery threshold shall be 10% or lower. | UC15 | Safety |

---

## 7. CI/CD Environment

| Area | Tool / Technology |
|---|---|
| IDE | Visual Studio Code |
| Version Control | Git, GitHub |
| Build System | CMake |
| Compiler | g++ |
| Unit Test | Google Test / gtest |
| CI Tool | GitHub Actions |
| Deployment Server | AWS EC2 |
| Team Communication | Discord |
| Documentation / Knowledge Base | Notion |
| Issue Tracking / Task Management | GitHub Issues |

---

## 8. Initial Traceability

| Use Case | Main FR IDs | Main NFR IDs |
|---|---|---|
| UC1. Turn On System | FR-UC1-01, FR-UC1-02, FR-UC1-03, FR-UC1-04 | N1.1 |
| UC2. Set Cleaning Mode | FR-UC2-01, FR-UC2-02 | N2.1 |
| UC3. Move Forward | FR-UC3-01 | N2.1 |
| UC4. Start Cleaning | FR-UC4-01 | N1.1, N2.1 |
| UC5. Avoid Obstacle | FR-UC5-01 ~ FR-UC5-06 | N1.2, N2.1 |
| UC6. Adjust Boost Mode | FR-UC6-01 ~ FR-UC6-04 | N1.3, N2.1 |
| UC7. Set Stand-by Mode | FR-UC7-01 | N1.1 |
| UC8. Stop Moving | FR-UC8-01 ~ FR-UC8-03 | N1.1 |
| UC9. Stop Cleaning | FR-UC9-01 ~ FR-UC9-03 | N1.1 |
| UC10. Charge Battery | FR-UC10-01, FR-UC10-02 | N2.1 |
| UC11. Turn Off System | FR-UC11-01, FR-UC11-02 | N1.1 |
| UC12. Turn Left | FR-UC12-01, FR-UC12-02 | N1.2 |
| UC13. Turn Right | FR-UC13-01, FR-UC13-02 | N1.2 |
| UC14. Move Backward | FR-UC14-01, FR-UC14-02 | N1.2 |
| UC15. Enter Low Battery Mode | FR-UC15-01, FR-UC15-02 | N3.1 |
| UC16. Stop Charging | FR-UC16-01, FR-UC16-02 | N2.1 |

---

## 9. Notes for AI Generation

- Use this document as the Week 1 requirements source.
- Keep requirement IDs consistent where possible.
- Do not invent new functional requirements beyond this document, implementation evidence, or tests.
- If a requirement is inferred from implementation or test files, mark it separately as `source: implementation/test`.
- Simulator belongs to testing support and does not need to follow OOAD architecture.
