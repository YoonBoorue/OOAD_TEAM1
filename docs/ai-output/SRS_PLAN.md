# RVC Control SW SRS Draft Generation Plan

## Summary
- 생성 대상은 `docs/ai-output/srs.md`와 `docs/ai-output/srs_diagrams.puml`이다.
- 구현 코드는 수정하지 않는다.
- 본문 설명은 한국어로 작성하고, class/operation/diagram/file 식별자는 영어를 유지한다.
- `week1_summary.md`와 `ooa_summary.md`를 SRS의 주 source로 사용하고, `ooi_summary.md`, 현재 구현, unit/system tests는 검증 및 traceability 근거로만 사용한다.

## Key Changes
- `docs/ai-output/srs.md`를 새로 작성한다. 포함 섹션:
  - Introduction
  - System Overview
  - System Scope and Boundary
  - Actors and External Interfaces
  - Functional Requirements
  - Non-Functional Requirements
  - Use Case List
  - Refined Use Case Descriptions
  - System Sequence Diagrams
  - System Operations
  - Operation Contracts
  - Domain Model / Glossary
  - FR - Use Case - System Operation - Unit Test - System Test Traceability Table
  - AI Inspection Findings
- 모든 requirement는 `source: project artifact` 또는 `source: implementation/test`로 표시한다.
- 프로젝트 artifact와 구현이 다른 경우 requirement 자체를 바꾸지 않고 `AI Inspection Findings`에 차이를 명시한다.
- `docs/ai-output/srs_diagrams.puml`에는 PlantUML SSD를 작성한다:
  - User control SSD: `powerButtonPressed()`, `startButtonPressed()`, `chargeBattery()`, `stopCharging()`
  - Sensor/event SSD: `dustDetected()`, `obstacleDetected()`, `lowBatteryDetected()`, `timerExpired()`
  - Control operation SSD: `moveForward()`, `startCleaning()`, `stopMoving()`, `stopCleaning()`, `turnLeft()`, `turnRight()`, `moveBackward()`
- `srs.md`의 diagram 섹션은 `srs_diagrams.puml`의 diagram 이름을 참조한다.

## Traceability Rules
- Functional requirements는 `week1_summary.md`의 FR ID를 기준으로 유지한다.
- Use case 설명은 `ooa_summary.md`의 UC1-UC16을 기준으로 작성한다.
- Unit test column은 관련 test file 또는 test group 단위로 연결한다.
- System test column은 관련 `TCxx_*.rvc` 파일 범위로 연결한다.
- 지원이 불완전한 항목은 `Partial` 또는 `Gap`으로 표시한다.

## Known Findings To Include
- `DigitalClockTick` 기반 일반 periodic operation은 구현에 없음.
- sensor/driver가 직접 signal을 보내는 구조는 아니며, 현재는 `Controller` system operation 직접 호출 중심임.
- obstacle avoidance 중 cleaner stop 요구와 달리 현재 구현/테스트는 cleaner 유지 동작을 기대함.
- `Direction`에는 `Stop` 값이 없고, 정지는 `MotorDriver` 상태로 표현됨.
- charging은 active cleaning 상태에서 standby로 전환하지 않고 거부되며, power-off charging은 허용됨.
- low-battery recovery는 현재 구현상 `StandbyMode`로 복귀함.
- `Controller`는 최종 코드에서 raw pointer ownership을 사용함.

## Validation
- 생성 후 `srs.md`와 `srs_diagrams.puml`의 섹션 완전성, source label 누락, traceability 누락을 점검한다.
- 구현 코드는 빌드/테스트하지 않아도 되지만, 필요 시 문서 검증용으로만 `rg` 기반 참조 확인을 수행한다.

## Assumptions
- artifact summary 파일의 내용이 원본 PDF를 대체하는 primary source로 간주된다.
- SRS는 black-box 관점이므로 내부 class 설계 설명은 최소화하고, 상세 class/sequence 설계는 추후 SDD에 둔다.
- PlantUML SSD는 원본 그림의 복사본이 아니라 artifact summary와 repo evidence 기반 재구성으로 표시한다.
