# Maintenance vs Vibe Coding Comparison Report

## 1. 요구사항 변경 요약

이번 변경의 핵심은 오른쪽 장애물 센서를 제거하는 것이다. 기존에는 obstacle input이 `front`, `left`, `right` 3개 센서 값으로 직접 들어왔지만, 변경 후에는 `front`, `left` 2개 센서만 직접 입력된다.

변경 후 오른쪽 확인은 다음 방식으로 처리한다.

- 앞: 전방 센서로 확인
- 왼쪽: 왼쪽 센서로 확인
- 오른쪽: 로봇을 오른쪽으로 회전한 뒤 전방 센서로 재확인
- 모두 막힘: 오른쪽 확인 실패 후 후진 recovery로 진입하고, 후진 중 비어 있는 방향을 재확인
- 후진 후 이동 방향 선택: 왼쪽이 우선이며, 오른쪽은 다시 전방 센서 재확인으로 판단

따라서 이번 비교의 기준은 코드 구조나 다이어그램 모양이 같은지가 아니라, 두 팀이 같은 변경 요구사항을 동일한 기능 수준으로 만족했는지이다.

## 2. 비교 범위

| 구분 | 비교 대상 |
|---|---|
| Maintenance 팀 | `include/`, `src/`, `tests/`, `system_tests/`, `simulator/` |
| Vibe Coding 팀 | `vibe/`, `docs/ai-output/*.md`, `docs/ai-output/*.puml` |

## 3. Maintenance 코드 기준 확인

현재 Maintenance production code도 이미 3센서 모델이 아니라 `front/left` 2센서 모델로 변경되어 있다.

| 확인 위치 | 현재 상태 |
|---|---|
| `include/rvc/Controller.hpp` | `obstacleDetected(const bool direction[2])` |
| `src/rvc/Controller.cpp` | `direction[2]`를 `std::array<bool, 2>`로 변환 |
| `include/rvc/ObstacleProcessor.hpp` | `std::array<bool, 2> direction_` |
| `src/rvc/ObstacleProcessor.cpp` | `decideDirection(const std::array<bool, 2>& ...)` |
| `include/rvc/ObstacleSensorDriver.hpp` | 내부 obstacle state가 `bool direction[2]` |
| `simulator/main.cpp` | `obstacle <front> <left>` 입력을 안내하고 parsing |

따라서 `direction[3]` 흔적은 Maintenance production code의 현재 상태가 아니라, `docs/ai-output/sdd.md` 같은 Vibe 문서 산출물에 남아 있는 오래된/불일치 설명으로 보는 것이 정확하다.

## 4. 구현 방식 비교

| 항목 | Maintenance 팀 | Vibe Coding 팀 |
|---|---|---|
| obstacle 입력 | `Controller::obstacleDetected(const bool direction[2])` | `Controller::obstacleDetected(const bool direction[2])` |
| 오른쪽 센서 제거 | 현재 코드 기준 이미 반영, `front/left`만 전달 | `right` state 제거, `front/left`와 recheck state로 분리 |
| 오른쪽 확인 | `ObstacleProcessor` 내부 `checkR` 상태로 우회전 후 전방 재확인 | `frontAfterRightTurn`, `decideDirectionAfterRightTurn()`로 명시적 재확인 |
| 후진 recovery | motor의 forward 상태와 `checkR` 조합으로 처리 | `backwardRecoveryPending`으로 후진 recovery 상태를 명시적으로 유지 |
| simulator | 기존 CLI script simulator 중심 | script simulator + map simulator + adapter 좌표 변환 |
| 문서 산출물 | production code/test 중심으로 변경 반영 | SRS, SSD/SD/class/state `.puml`, 변경 요약 문서에 반영. 단, `sdd.md` 일부 표에는 오래된 `direction[3]` 설명이 잔존 |

핵심 차이는 상태 표현 방식이다. Maintenance는 변경을 기존 processor 흐름 안에 작게 흡수했다. Vibe는 센서 재확인 상태를 driver/processor/controller에 명시적으로 나누어 표현했다.

## 5. Unit Test 비교

| 항목 | Maintenance 팀 | Vibe Coding 팀 |
|---|---:|---:|
| 전체 unit test | 169개 | 106개 |
| simulator unit test | 별도 없음 | 9개 |
| obstacle 관련 controller test | 21개 | 13개 |
| obstacle processor/recheck test | controller test 안에서 간접 확인 | 9개 processor test로 분리 |

분석:

- Maintenance는 unit test 수가 더 많고, 기존 controller operation 중심 테스트를 넓게 유지했다.
- Vibe는 unit test 수는 적지만 `ControllerObstacleDetectedTest`, `ObstacleProcessorTest`, `SimulationLoopTest`로 책임을 나누어 검증했다.
- obstacle 관점에서는 Maintenance가 controller 시나리오를 많이 직접 검증했고, Vibe는 processor decision과 simulator 동작까지 분리해 검증했다.

## 6. System Test 비교

| 항목 | Maintenance 팀 | Vibe Coding 팀 |
|---|---:|---:|
| system scenario 문서/목록 | 45개 `.rvc` | traceability 기준 42개 |
| 현재 workspace의 실제 `.rvc` 파일 | 45개 | 42개 |
| 실행 결과 | `45/45 passed` | `42/42 passed` |

분석:

- Maintenance는 현재 repo 기준으로 system script가 모두 존재하고, rebuild 후 `45/45 passed`를 확인했다.
- Vibe도 현재 workspace 기준 42개 system test script가 모두 존재하며, 기준 브랜치와 일치하는 상태에서 `42/42 passed`를 확인했다.
- 두 팀 모두 system test 실행 결과는 정상이다.
- 차이는 scenario 개수와 범위이다. Maintenance는 45개, Vibe는 42개이며 Vibe는 별도 simulator unit test도 함께 둔다.

## 7. 요구사항 만족 여부

| 요구사항 | Maintenance 팀 | Vibe Coding 팀 |
|---|---|---|
| 오른쪽 센서 직접 입력 제거 | 만족 | 만족 |
| front/left 2개 입력 사용 | 만족 | 만족 |
| 오른쪽은 우회전 후 전방 센서 재확인 | 만족 | 만족 |
| all-blocked 후 후진 recovery | 만족 | 만족 |
| 후진 중 왼쪽 우선 | 만족 | 만족 |
| 반복 후진 상황에서 잘못된 전진 복귀 방지 | processor state로 처리 | `backwardRecoveryPending`으로 명시 처리 |
| 문서/다이어그램 반영 | production code/test 중심, docs/ai-output 일부 설명은 별도 정합성 확인 필요 | 대부분 반영. 단, `sdd.md` 일부 `direction[3]` 설명은 추가 정합성 보정 필요 |
| system test 산출물 보존 | 좋음 | 42개 존재 및 정상 |

결론적으로 두 구현은 기능적으로 같은 요구사항을 수행한다. Maintenance는 system scenario 수가 더 많고, Vibe는 문서/설계 추적성과 simulator 확장이 강하다.

## 8. 주요 차이점

### Maintenance 팀

- 현재 코드 기준으로 오른쪽 센서 제거 요구사항은 반영되어 있다.
- 기존 구조를 크게 바꾸지 않고 `direction[2]`와 `checkR` 상태로 요구사항을 처리했다.
- system test가 45개 모두 실제 파일로 남아 있고 실행 가능하다.
- 변경이 작고 직접적이어서 빠르게 유지보수하기 좋다.
- 대신 오른쪽 재확인과 후진 recovery 의미가 `ObstacleProcessor` 내부 상태에 묻혀 있어 설계 의도를 외부에서 읽기는 어렵다.

### Vibe Coding 팀

- SRS/PUML과 변경 요약 문서까지 요구사항 변경을 추적 가능하게 반영했다. 다만 `sdd.md` 일부 표에는 오래된 `direction[3]` 설명이 남아 있어 문서 정합성 보정이 필요하다.
- right recheck, backward recheck, map simulator 좌표 변환까지 더 명시적으로 모델링했다.
- `backwardRecoveryPending`으로 후진 recovery 상태를 분리해 앞뒤 왕복 같은 오류를 잡았다.
- 대신 산출물이 많아지면서 system test script, traceability, simulator test처럼 함께 관리해야 할 대상도 늘어났다.

## 9. 발표용 인사이트

- AI는 새 기능을 빠르게 만들 수 있지만, 기존 산출물과 테스트 전체를 끝까지 맞추는 유지보수는 훨씬 어렵다.
- 이번 변경은 단순히 `right` 변수를 삭제하는 문제가 아니라, 센서 모델 자체가 `3센서 직접 판단`에서 `2센서 + 행동 후 재확인`으로 바뀐 문제였다.
- 기존 문서와 테스트가 좋을수록 AI도 더 정확하게 수정한다. 요구사항이 문서에 명확히 남아 있어야 코드, 테스트, simulator가 같은 방향으로 바뀐다.
- 프롬프트보다 중요한 것은 기존 산출물의 품질이다. 산출물이 불명확하면 AI도 구조를 추측하게 되고, 유지보수 누락이 생긴다.
- 테스트는 유지보수 품질을 결정한다. Vibe system script가 기준 브랜치와 동일하게 42개로 정리된 뒤에야 `42/42 passed`를 확인할 수 있었고, 이를 통해 테스트 산출물 자체도 유지보수 대상임을 확인했다.
- 결론적으로 Vibe Coding의 의의는 AI가 코드를 새로 만든 것이 아니라, 기존 코드와 문서와 테스트를 연결해 요구사항 변경을 추적하려고 했다는 점이다.

## 10. 최종 결론

두 팀 모두 오른쪽 센서 제거 요구사항의 핵심 기능은 구현했다. Maintenance는 기존 코드와 system test를 안정적으로 유지하는 데 강했고, Vibe Coding은 요구사항 변경을 문서, 설계, unit test, simulator까지 넓게 반영하는 데 강했다. 단, Vibe 문서 중 `sdd.md` 일부 항목은 최신 코드와 완전히 일치하도록 추가 정리가 필요하다.

정확히 말하면, Maintenance 팀 구현은 빌드와 테스트 안정성이 높고 현재 코드 기준으로 오른쪽 센서 제거 요구사항도 반영되어 있다. 다만 오른쪽 재확인과 후진 recovery 상태가 `ObstacleProcessor` 내부 `checkR` 상태에 묻혀 있어, Vibe 구현보다 설계 의도가 명시적으로 드러나지는 않는다.

이번 비교에서 가장 중요한 차이는 코드 모양이 아니라 유지보수 산출물의 일관성이다. 기능 구현만 보면 둘 다 가능했지만, 발표에서 강조해야 할 지점은 "변경된 요구사항이 문서, 코드, 테스트, simulator 전체에 얼마나 일관되게 남았는가"이다.
