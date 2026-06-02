# Vibe Right Sensor Removal Change Summary

## 기준

- 비교 기준: `front`, `left`, `right` 3개 장애물 센서가 존재하던 기존 구현
- 비교 범위: `vibe/`
- 요구사항 변경: 오른쪽 센서를 제거하고, 오른쪽 확인은 로봇을 오른쪽으로 회전한 뒤 전방 센서로 재측정한다.

## 전체 요약

기존 구현은 obstacle 입력을 `front/left/right` 3개 센서 값으로 직접 처리했다. 현재 구현은 right sensor 값을 직접 받지 않고, `front/left` 2개 즉시 입력과 "우회전 후 전방 센서 재확인", "후진 중 좌/우 확인 재확인" 상태를 조합해 회피 동작을 수행한다.

핵심 변화는 다음과 같다.

| 구분 | 기존 3센서 구현 | 현재 변경 후 |
|---|---|---|
| 장애물 입력 | `direction[3] = front, left, right` | `direction[2] = front, left` |
| 오른쪽 판단 | right sensor 값을 직접 확인 | `turnRight()` 후 front sensor 재확인 |
| 모두 막힌 상황 | front/left/right가 모두 blocked면 backward | right 재확인도 blocked면 원위치 보정 후 backward, 이후 left 우선 재확인 |
| 테스트 모델 | 3방향 센서 조합 검증 | front/left 입력, right-through-front 재확인, 후진 후 left 우선 검증 |
| simulator 입력 | `obstacle_right`가 right sensor blocked 의미 | `obstacle_right_clear`가 우회전 후 front clear 의미 |

## Production Code 변경

### `Controller`

- `Controller::obstacleDetected(const bool direction[3])`가 `Controller::obstacleDetected(const bool direction[2])`로 변경되었다.
- 기존에는 `direction[0]`, `direction[1]`, `direction[2]`를 각각 `front`, `left`, `right`에 저장했다.
- 현재는 `front/left`만 저장하고, right 판단은 `ObstacleSensorDriver`에 저장된 recheck state를 통해 처리한다.
- 장애물 회피 흐름이 `handleObstacleAvoidance()`로 분리되었다.
- front와 left가 모두 막힌 경우:
  - 먼저 `turnRight()` 수행
  - 우회전 후 front sensor recheck가 clear이면 `moveForward()`
  - recheck도 blocked이면 `turnLeft()`로 방향을 복구하고 `moveBackward()`
  - 후진 중 재확인 결과에서 left가 비어 있으면 left 우선, 아니면 right-through-front, 모두 막히면 backward 유지
- simulator/test에서 `Controller` 객체 주소가 재사용될 때 이전 내부 상태가 섞이지 않도록 `Controller()` 생성자 초기화가 추가되었다.

### `ObstacleSensorDriver`

- 기존 `bool right` state가 제거되었다.
- 추가된 recheck state:
  - `frontAfterRightTurn`
  - `leftAfterBackward`
  - `frontAfterBackwardRightCheck`
- 추가된 메서드:
  - `setObstacleInput(bool frontBlocked, bool leftBlocked)`
  - `setFrontAfterRightTurn(bool frontBlocked)`
  - `setBackwardRecheck(bool leftBlocked, bool frontBlockedForRightCheck)`
  - `isFrontClearAfterRightTurn()`
  - `isLeftClearAfterBackward()`
  - `isRightClearAfterBackward()`
- `hasObstacle()`는 더 이상 right를 보지 않고 `front || left`만 사용한다.

### `ObstacleProcessor`

- 기존 판단은 `front -> left -> right -> backward` 순서의 3센서 판단이었다.
- 현재 1차 판단은 `front/left`만 사용한다.
  - front clear: `Forward`
  - front blocked, left clear: `Left`
  - front blocked, left blocked: `Right`
- `Right`는 즉시 오른쪽이 비어 있다는 뜻이 아니라, "오른쪽으로 회전해서 front sensor로 다시 확인해야 함"을 뜻한다.
- 추가 판단 함수:
  - `decideDirectionAfterRightTurn()`: 우회전 후 front가 clear이면 `Forward`, blocked이면 `Backward`
  - `decideDirectionAfterBackwardRecheck()`: 후진 중 left clear 우선, 그다음 right-through-front, 모두 blocked면 `Backward`

## Unit Test 변경

### 입력 helper 변경

- `SendObstacle(controller, front, left, right)`가 `SendObstacle(controller, front, left)`로 변경되었다.
- right 재확인이 필요한 테스트를 위해 `SendObstacleWithRecheck()` helper가 추가되었다.

### Controller obstacle test 변경

- right sensor state 직접 기대값이 제거되었다.
- `NormalFrontAndLeftBlockedRightClearResumesForwardAfterTurn`는 더 이상 right sensor clear가 아니라, `frontAfterRightTurn=false`를 통해 "우회전 후 전방 clear"를 검증한다.
- all blocked test는 `front/left` blocked, 우회전 후 front blocked, 후진 중 left/right-through-front 모두 blocked 조합으로 `Backward`를 검증한다.
- no-argument obstacle path도 기존 `front/left/right` 저장 상태 대신 `front/left + recheck state`를 사용한다.

### ObstacleProcessor test 변경

- 기존 8개 조합 테스트는 front/left/right 3센서 조합을 직접 검증했다.
- 현재는 9개 테스트로 변경되었다.
  - front/left 기반 1차 판단
  - 우회전 후 front recheck 판단
  - 후진 중 left 우선, right-through-front, all blocked 판단

### Traceability 변경

- unit test 총 개수: `104`개에서 `105`개로 변경
- `ObstacleProcessorTest`: `8`개에서 `9`개로 변경
- 추적성 문서가 right sensor 직접 조합 대신 recheck 기반 장애물 회피로 갱신되었다.

## System Test 변경

### P12 변경

기존 P12는 `obstacle_all` 하나로 all blocked 상황에서 `Backward`를 확인했다.

현재 P12는 요구사항 변경을 반영해 두 단계를 확인한다.

1. `obstacle_right_clear`
   - front/left blocked
   - 우회전 후 front clear
   - 기대 결과: `Forward`

2. `obstacle_all`
   - front/left blocked
   - 우회전 후 front blocked
   - 후진 중 left/right-through-front도 blocked
   - 기대 결과: `Backward`

### System Traceability 변경

- P12 관련 use case에 `UC13 Turn Right`가 추가되었다.
- system operation 설명이 `obstacleDetected(all)`에서 `obstacleDetected(front/left) plus front recheck`로 변경되었다.

## Simulator 변경

### Script simulator

- 기존 `bool direction[3]` 입력을 `ObstacleScenario` 구조체로 대체했다.
- `ObstacleScenario`는 다음 값을 가진다.
  - `frontBlocked`
  - `leftBlocked`
  - `frontAfterRightTurnBlocked`
  - `leftAfterBackwardBlocked`
  - `frontAfterBackwardRightCheckBlocked`
- 기존 `obstacle_right` 명령은 제거되고, `obstacle_right_clear` 명령으로 변경되었다.
- interactive menu의 6번 항목도 `Obstacle Right`에서 `Right Path Clear`로 변경되었다.

### Map simulator

- `SensorSnapshot::obstacleBlocked`가 `std::array<bool, 3>`에서 `std::array<bool, 2>`로 변경되었다.
- `Environment::sense()`는 front/left만 immediate obstacle input으로 제공한다.
- 오른쪽 방향은 `rightOf(heading)`을 즉시 right sensor로 넘기지 않고, `frontAfterRightTurnBlocked`로 분리해 전달한다.
- `RvcAdapter::feedSensors()`는 obstacle/recheck signal이 있을 때만 obstacle flow를 호출한다.
- `RvcAdapter`는 `Controller::obstacleDetected(direction[2])` 대신 sensor driver에 recheck state를 세팅한 뒤 `Controller::obstacleDetected()`를 호출한다.
- map simulator에서는 Controller의 "turn right 후 forward" 명령을 좌표계 기준 오른쪽 이동으로 보여주기 위해 adapter가 robot heading과 실제 movement direction을 유지한다.

### Simulator test 변경

- all blocked simulator test는 `obstacleBlocked = {true, true}`와 recheck blocked state를 함께 설정한다.
- right path clear simulator test는 front/left blocked 후 front recheck clear일 때 map 좌표상 `Right` 이동으로 변환되는지 확인한다.
- simulator loop test 일부는 현재 simulator 정책에 맞게 독립 실행 가능하도록 조정되었다.
- 특히 autoStart 상태에서 charging key는 충전을 시작하지 않고, active cleaning tick의 배터리 소모만 반영하는 것으로 검증한다.

## 변경 파일 목록

| 영역 | 파일 |
|---|---|
| Production code | `vibe/include/rvc/Controller.hpp` |
| Production code | `vibe/include/rvc/ObstacleProcessor.hpp` |
| Production code | `vibe/include/rvc/ObstacleSensorDriver.hpp` |
| Production code | `vibe/src/Controller.cpp` |
| Production code | `vibe/src/ObstacleProcessor.cpp` |
| Unit test | `vibe/tests/unit_tests/rvc_unit_tests.cpp` |
| Unit test traceability | `vibe/tests/unit_tests/TEST_TRACEABILITY.md` |
| System test | `vibe/tests/system_tests/tc/P12_obstacle_all_blocked_active_avoiding.rvc` |
| System test traceability | `vibe/tests/system_tests/SYSTEM_TEST_TRACEABILITY.md` |
| Script simulator | `vibe/simulator/main.cpp` |
| Map simulator | `vibe/sim/include/sim/Environment.hpp` |
| Map simulator | `vibe/sim/src/Environment.cpp` |
| Map simulator | `vibe/sim/src/RvcAdapter.cpp` |
| Simulator test | `vibe/sim/tests/SimulationLoopTest.cpp` |

## 검증 결과

아래 명령으로 변경 후 동작을 확인했다.

```bash
cmake --build /private/tmp/ooad_vibe_verify_all
/private/tmp/ooad_vibe_verify_all/rvc_vibe_tests
ctest --test-dir /private/tmp/ooad_vibe_verify_all -L system --output-on-failure
/private/tmp/ooad_vibe_verify_all/rvc_vibe_simulator_tests
ctest --test-dir /private/tmp/ooad_vibe_verify_all --output-on-failure
```

결과:

- Unit test: `105/105 passed`
- System test: `42/42 passed`
- Simulator test: `9/9 passed`
- 전체 CTest: `156/156 passed`
- `git diff --check`: passed

## 핵심 결론

이번 변경은 단순히 `right` 필드를 삭제하는 작업이 아니었다. 기존 3센서 판단 모델을 2센서 입력과 행동 후 재측정 모델로 바꾸는 작업이었다. 따라서 production code뿐 아니라 unit test, system test, script simulator, map simulator가 모두 같은 센서 모델을 공유하도록 함께 수정해야 했다.
