# OOI Summary - RVC Control SW Project Source

> Purpose: This markdown file is a readable AI-input version of `[T1] OOI.pdf`.
> It summarizes implementation, unit test, system test, code review, static analysis/coverage, and simulator evidence from the OOI artifact.
> Source type: project artifact summary.

---

## 1. Document Scope

The OOI artifact explains the implementation and testing stage of the RVC Control SW project.

Main sections:

1. Revised CI/CD Overview
2. Revised Class Diagram
3. Revised Sequence Diagrams
4. Program / Code Mapping
5. Code Review
6. Unit Test
7. Coverage Measurement
8. System Test and Simulator

---

## 2. Revised CI/CD Overview

The CI/CD pipeline uses the following elements:

| Area | Tool |
|---|---|
| Development IDE | VS Code |
| Build | CMake, Ninja |
| Unit Test | Google Test |
| Version Control | Git, GitHub |
| CI/CD | GitHub Actions |
| Build & Test in CI | CMake + gtest |
| Static Analysis | CI static analysis step |
| Coverage | Coverage report generation |
| Deploy | Deployment stage |
| Deployment Server | AWS EC2 |
| Collaboration | Discord, Notion, GitHub Issues |

Pipeline summary:

1. Developer works in VS Code.
2. Code is built using CMake/Ninja.
3. Google Test unit tests are executed locally and in CI.
4. Git/GitHub manages version control and pull requests.
5. GitHub Actions runs build, test, static analysis, coverage, and deployment-related jobs.
6. Deployment server is AWS EC2.

---

## 3. Implementation Structure

The OOI code slide identifies the following main implementation roles.

| Component | Responsibility |
|---|---|
| `Controller` | Receives system operations and passes messages to internal objects. |
| `OperatingMode` | Manages current mode and determines mode transitions. |
| Drivers | Execute actual operations and store simulated hardware state. Examples: `CleanerDriver`, `MotorDriver`, `BatteryDriver`. |
| Processors | Encapsulate decision logic based on sensor information. Examples: `DustProcessor`, `ObstacleProcessor`. |

### Controller Members Observed in Code Slide

The code slide shows `Controller` with private members similar to:

```cpp
bool power;
bool isNowCharging;
OperatingMode* currentMode;
BatteryDriver batteryDriver;
CleanerDriver cleanerDriver;
DustSensorDriver dustSensorDriver;
MotorDriver motorDriver;
ObstacleSensorDriver obstacleSensorDriver;
ObstacleProcessor obstacleProcessor;
DustProcessor dustProcessor;
```

Depending on the final repository version, these may be value members, pointers, or smart-pointer-like ownership structures.

---

## 4. System Operations Implemented

The OOI artifact maps system operations to sequence diagrams.

| System Operation | Related SD | Related Use Case |
|---|---|---|
| `dustDetected()` | SD-06 Adjust Boost Mode | UC6 |
| `startButtonPressed()` | SD-02 Set Cleaning Mode, SD-07 Set Stand-by Mode | UC2, UC7 |
| `powerButtonPressed()` | SD-01 Turn On System, SD-11 Turn Off System | UC1, UC11 |
| `chargeBattery()` | SD-10 Charge Battery | UC10 |
| `stopCharging()` | SD-16 Stop Charging | UC16 |
| `lowBatteryDetected()` | SD-15 Enter Low Battery Mode | UC15 |
| `obstacleDetected()` | SD-05 Avoid Obstacle | UC5 |
| `TurnOn()` | SD-01 Turn On System | UC1 |
| `TurnOff()` | SD-11 Turn Off System | UC11 |

Note: In final code, method names may use lowerCamelCase. Keep names consistent with repository code when generating documentation.

---

## 5. Revised Class Diagram Summary

The revised class diagram contains the following main design groups.

### 5.1 Controller Group

- `Controller`
  - Owns or references current mode.
  - Owns or references drivers and processors.
  - Provides system operation methods.

### 5.2 Mode Group

- `OperatingMode`
  - Abstract mode interface/base class.
  - Defines mode behavior and transition methods.
- `StandbyMode`
- `NormalMode`
- `BoostMode`
- `LowBatteryMode`

### 5.3 Driver Group

- `BatteryDriver`
- `CleanerDriver`
- `DustSensorDriver`
- `MotorDriver`
- `ObstacleSensorDriver`

### 5.4 Processor Group

- `DustProcessor`
- `ObstacleProcessor`

### 5.5 Supporting Types

- `Direction`
- Sensor state structures or flags, depending on implementation.

---

## 6. Revised Sequence Diagrams

The OOI artifact includes revised sequence diagrams for the following:

| SD | Related Use Case | Summary |
|---|---|---|
| SD-05 | UC5. Avoid Obstacle | Obstacle sensor provides obstacle input; controller uses obstacle processor and motor/cleaner control. |
| SD-06 | UC6. Adjust Boost Mode | Dust sensor provides dust signal; controller/mode transitions to boost and later returns. |
| SD-10 | UC10. Charge Battery | User starts charging; controller checks condition; battery charging begins. |
| SD-15 | UC15. Enter Low Battery Mode | Battery sends low-battery signal; controller enters low-battery mode and stops motion/cleaning. |
| SD-16 | UC16. Stop Charging | User stops charging; controller stops battery charging. |
| SD-11 | UC11. Turn Off System | Power button turns system off and drivers are stopped/deactivated. |
| SD-07 | UC7. Set Stand-by Mode | Start button from cleaning mode returns system to standby. |
| SD-02 | UC2. Set Cleaning Mode | Start button from standby enters normal cleaning mode. |
| SD-03 | UC3. Move Forward | Mode requests forward movement from motor. |
| SD-04 | UC4. Start Cleaning | Mode requests cleaner activation. |
| SD-08 | UC8. Stop Moving | Mode/off/low-battery state requests motor stop. |
| SD-09 | UC9. Stop Cleaning | Mode/off/low-battery state requests cleaner stop. |
| SD-12 | UC12. Turn Left | Obstacle avoidance requests left turn. |
| SD-13 | UC13. Turn Right | Obstacle avoidance requests right turn. |
| SD-14 | UC14. Move Backward | Obstacle avoidance requests backward movement. |

---

## 7. Code Review Evidence

The OOI artifact includes code review screenshots for multiple pull requests or commits.

Observed review topics include:

1. `dustDetected()` related unit test review.
2. `startButtonPressed()` related unit test review.
3. `dustDetected()` implementation/test review.
4. Comments about controller behavior, mode transition, and test expectations.

For final documentation:

- Include at least three code review examples.
- For each example, summarize:
  - reviewed branch or feature,
  - reviewer comment,
  - issue raised,
  - response or resolution.

---

## 8. Unit Test Strategy

The OOI artifact states that unit tests use Google Test.

### 8.1 Unit Test Principles

- Unit tests are designed around system operations.
- Drivers can act as real actors calling system operations in tests.
- Stub concepts can replace real behavior where necessary.
- Tests are executed in CI through GitHub Actions.

### 8.2 Unit Test Areas Shown

The unit test result slides include test groups for:

| Area | Meaning |
|---|---|
| ChargeBattery | Tests charging conditions and charging state. |
| LowBatteryMode | Tests entering and behavior of low battery mode. |
| StopCharging | Tests stopping charging. |
| TurnOff | Tests power-off behavior. |
| dustDetected | Tests dust detection and boost mode behavior. |
| TurnOn | Tests power-on initialization and standby transition. |
| Power Button / Start Button | Tests user button operations. |
| ObstacleDetected | Tests obstacle detection and avoidance behavior. |

---

## 9. Coverage Measurement

The OOI artifact includes a GCC Code Coverage Report.

Observed coverage summary:

| Metric | Approximate Value from Slide |
|---|---|
| Line coverage | About 42% |
| Function coverage | About 73% |
| Branch coverage | About 19% |

Notes:

- Coverage is measured after unit test execution.
- Coverage report is generated as an artifact or report output.
- Branch coverage is lower than function coverage, so additional negative/edge cases may improve test quality.

---

## 10. System Test and Simulator

The OOI artifact describes a C++ CLI RVC simulator.

### 10.1 Simulator Modes

| Mode | Description |
|---|---|
| Interactive Mode | Menu-based manual operation for system actions such as power, start, dust detection, and obstacle detection. |
| Script Mode | Automatically executes system test cases from files. |

### 10.2 System Test Automation

The simulator supports:

- Executing system test case scripts.
- Checking state using `expect` commands.
- Producing PASS/FAIL results.
- Running through CTest after simulator build.
- Storing test logs as CI artifacts.

### 10.3 CI Integration

System tests are integrated into CI:

1. Pull request or main branch push triggers tests.
2. CMake builds the simulator.
3. CTest runs script-based system tests.
4. Test logs are stored as artifacts.
5. Results can be checked in GitHub Actions.

---

## 11. Recommended Traceability for Final Documentation

| Requirement / UC | System Operation | Unit Test Area | System Test Area |
|---|---|---|---|
| UC1 Turn On System | `powerButtonPressed()` / `TurnOn()` | TurnOn, PowerButton | TC for power-on |
| UC2 Set Cleaning Mode | `startButtonPressed()` | StartButton | TC for start from standby |
| UC5 Avoid Obstacle | `obstacleDetected()` | ObstacleDetected | TC for obstacle patterns |
| UC6 Adjust Boost Mode | `dustDetected()` / `timerExpired()` | DustDetected | TC for dust/boost/timer |
| UC10 Charge Battery | `chargeBattery()` | ChargeBattery | TC for charging |
| UC11 Turn Off System | `powerButtonPressed()` / `TurnOff()` | TurnOff, PowerButton | TC for power-off |
| UC15 Enter Low Battery Mode | `lowBatteryDetected()` | LowBatteryMode | TC for low-battery |
| UC16 Stop Charging | `stopCharging()` | StopCharging | TC for stop charging |

---

## 12. Notes for AI Generation

- Use this file when writing the OOI, testing, and implementation sections of the final report or presentation.
- When generating SRS, only use OOI content as supporting validation evidence, not as the primary source of requirements.
- When generating SDD, use OOI class/sequence/code mapping as design evidence.
- When generating final PPT/PDF, include the AI-assisted workflow, benefits, limitations, and reflections.
