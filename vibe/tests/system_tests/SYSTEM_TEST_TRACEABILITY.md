# RVC Simulator System Test Traceability

이 문서는 `vibe/tests/system_tests/tc/*.rvc` 기반 black-box system test의 추적성을 정리한다.
모든 테스트는 Google Test를 사용하지 않고 `rvc_vibe_simulator --script <script.rvc>`로 실행된다.

## Summary

| 구분 | 개수 |
|---|---:|
| Positive scenarios | 12 |
| Negative scenarios | 30 |
| Total scenarios | 42 |
| Positive : Negative | 1 : 2.5 |

## Scenario Mapping

| ID | Script | SRS use case | System operation | Class | Expected result |
|---|---|---|---|---|---|
| P01 | `tc/P01_power_on_enters_standby.rvc` | UC1 Turn On System | `powerButtonPressed()` | Positive | `Off`에서 전원 입력 후 `StandbyMode`, motor/cleaner/boost/charging off, direction `Forward` |
| P02 | `tc/P02_power_off_from_standby_returns_off.rvc` | UC11 Turn Off System | `powerButtonPressed()` | Positive | `StandbyMode`에서 전원 입력 후 `Off`, motor/cleaner/boost/charging off |
| P03 | `tc/P03_start_from_standby_enters_normal.rvc` | UC2 Set Cleaning Mode, UC3 Move Forward, UC4 Start Cleaning | `startButtonPressed()` | Positive | `StandbyMode`에서 start 후 `NormalMode`, motor/cleaner on, boost off |
| P04 | `tc/P04_start_from_normal_returns_standby.rvc` | UC7 Set Stand-by Mode, UC8 Stop Moving, UC9 Stop Cleaning | `startButtonPressed()` | Positive | `NormalMode`에서 start 후 `StandbyMode`, motor/cleaner/boost off |
| P05 | `tc/P05_dust_in_normal_enters_boost.rvc` | UC6 Adjust Boost Mode | `dustDetected()` | Positive | `NormalMode`에서 dust 후 `BoostMode`, cleaner boost on |
| P06 | `tc/P06_timer_in_boost_returns_normal.rvc` | UC6 Adjust Boost Mode | `timerExpired()` | Positive | `BoostMode`에서 timer 후 `NormalMode`, cleaner on, boost off |
| P07 | `tc/P07_low_battery_in_normal_stops_motor.rvc` | UC15 Enter Low Battery Mode, UC8 Stop Moving, UC9 Stop Cleaning | `lowBatteryDetected()` | Positive | `NormalMode`에서 low battery 후 `LowBatteryMode`, motor/cleaner off |
| P08 | `tc/P08_low_battery_in_boost_stops_motor.rvc` | UC15 Enter Low Battery Mode, UC8 Stop Moving, UC9 Stop Cleaning | `lowBatteryDetected()` | Positive | `BoostMode`에서 low battery 후 `LowBatteryMode`, motor/cleaner/boost off |
| P09 | `tc/P09_charging_accepted_in_standby.rvc` | UC10 Charge Battery | `chargeBattery()` | Positive | 낮은 배터리에서 `StandbyMode` 충전 요청 후 charging on, cleaning inactive |
| P10 | `tc/P10_stop_charging_safe_non_cleaning.rvc` | UC16 Stop Charging | `stopCharging()` | Positive | charging 중 stop 요청 후 `StandbyMode`, charging off, motor/cleaner off |
| P11 | `tc/P11_obstacle_front_blocked_left_clear_active.rvc` | UC5 Avoid Obstacle, UC12 Turn Left | `obstacleDetected(front)` | Positive | `NormalMode`에서 front blocked/left clear 입력 후 system active, motor/cleaner on |
| P12 | `tc/P12_obstacle_all_blocked_active_avoiding.rvc` | UC5 Avoid Obstacle, UC14 Move Backward | `obstacleDetected(all)` | Positive | `NormalMode`에서 all blocked 입력 후 system active, direction `Backward`, motor/cleaner on |
| N01 | `tc/N01_start_while_off_does_not_enter_normal.rvc` | UC2 Set Cleaning Mode | `startButtonPressed()` | Negative | `Off`에서 start 입력은 `NormalMode`로 전이하지 않고 motor/cleaner off 유지 |
| N02 | `tc/N02_dust_while_off_does_not_enter_boost.rvc` | UC6 Adjust Boost Mode | `dustDetected()` | Negative | `Off`에서 dust 입력은 `BoostMode`로 전이하지 않음 |
| N03 | `tc/N03_dust_while_standby_does_not_enter_boost.rvc` | UC6 Adjust Boost Mode | `dustDetected()` | Negative | `StandbyMode`에서 dust 입력은 `BoostMode`로 전이하지 않음 |
| N04 | `tc/N04_dust_while_low_battery_does_not_enter_boost.rvc` | UC6 Adjust Boost Mode, UC15 Enter Low Battery Mode | `dustDetected()` | Negative | `LowBatteryMode`에서 dust 입력은 `BoostMode`로 전이하지 않고 stopped 상태 유지 |
| N05 | `tc/N05_timer_while_off_no_change.rvc` | UC6 Adjust Boost Mode | `timerExpired()` | Negative | `Off`에서 timer 입력은 mode를 바꾸지 않음 |
| N06 | `tc/N06_timer_while_standby_no_change.rvc` | UC6 Adjust Boost Mode | `timerExpired()` | Negative | `StandbyMode`에서 timer 입력은 mode를 바꾸지 않음 |
| N07 | `tc/N07_timer_while_normal_no_change.rvc` | UC6 Adjust Boost Mode | `timerExpired()` | Negative | `NormalMode`에서 timer 입력은 mode를 바꾸지 않음 |
| N08 | `tc/N08_charge_while_normal_rejected.rvc` | UC10 Charge Battery | `chargeBattery()` | Negative | `NormalMode`에서 charge 요청은 rejected, charging off, cleaning active 유지 |
| N09 | `tc/N09_charge_while_boost_rejected.rvc` | UC10 Charge Battery | `chargeBattery()` | Negative | `BoostMode`에서 charge 요청은 rejected, charging off, boost cleaning 유지 |
| N10 | `tc/N10_obstacle_while_off_does_not_start_motor.rvc` | UC5 Avoid Obstacle | `obstacleDetected(front)` | Negative | `Off`에서 obstacle 입력은 motor를 start하지 않음 |
| N11 | `tc/N11_obstacle_while_standby_does_not_start_motor.rvc` | UC5 Avoid Obstacle | `obstacleDetected(front)` | Negative | `StandbyMode`에서 obstacle 입력은 motor를 start하지 않음 |
| N12 | `tc/N12_obstacle_while_low_battery_does_not_start_motor.rvc` | UC5 Avoid Obstacle, UC15 Enter Low Battery Mode | `obstacleDetected(all)` | Negative | `LowBatteryMode`에서 obstacle 입력은 motor/cleaner off 유지 |
| N13 | `tc/N13_start_while_low_battery_does_not_enter_normal.rvc` | UC2 Set Cleaning Mode, UC15 Enter Low Battery Mode | `startButtonPressed()` | Negative | `LowBatteryMode`에서 start 입력은 `NormalMode`로 전이하지 않음 |
| N14 | `tc/N14_repeated_start_only_valid_toggle.rvc` | UC2 Set Cleaning Mode, UC7 Set Stand-by Mode | `startButtonPressed()` | Negative | 반복 start는 `StandbyMode`/`NormalMode` 사이의 유효 toggle만 수행 |
| N15 | `tc/N15_repeated_power_only_safe_toggle.rvc` | UC1 Turn On System, UC11 Turn Off System | `powerButtonPressed()` | Negative | 반복 power는 on/off만 안전하게 toggle하고 stopped 상태를 유지 |
| N16 | `tc/N16_stop_charging_when_not_charging_keeps_off.rvc` | UC16 Stop Charging | `stopCharging()` | Negative | charging off 상태에서 stop 요청은 charging off와 safe standby 유지 |
| N17 | `tc/N17_repeated_stop_charging_remains_safe.rvc` | UC16 Stop Charging | `stopCharging()` | Negative | stop charging 반복 호출은 charging off와 safe standby 유지 |
| N18 | `tc/N18_low_battery_while_off_ignored.rvc` | UC15 Enter Low Battery Mode | `lowBatteryDetected()` | Negative | `Off`에서 low battery 입력은 ignored, mode `Off` 유지 |
| N19 | `tc/N19_repeated_low_battery_remains_low_battery.rvc` | UC15 Enter Low Battery Mode | `lowBatteryDetected()` | Negative | `LowBatteryMode`에서 low battery 반복 입력은 `LowBatteryMode` 유지 |
| N20 | `tc/N20_low_battery_cleared_outside_low_battery_no_invalid_transition.rvc` | UC15 Enter Low Battery Mode | `lowBatteryCleared()` | Negative | `NormalMode`에서 low battery cleared 입력은 invalid transition 없이 `NormalMode` 유지 |
| N21 | `tc/N21_charge_tick_not_charging_no_change.rvc` | UC10 Charge Battery | `chargingTick()` | Negative | charging off 상태의 charge tick은 mode/actuator 상태를 바꾸지 않음 |
| N22 | `tc/N22_clock_tick_while_off_no_change.rvc` | Common periodic behavior | `clockTick()` | Negative | `Off`에서 clock tick은 mode/actuator 상태를 바꾸지 않음 |
| N23 | `tc/N23_clock_tick_standby_no_sensor_no_change.rvc` | Common periodic behavior | `clockTick()` | Negative | sensor event 없는 `StandbyMode` clock tick은 mode를 바꾸지 않음 |
| N24 | `tc/N24_clock_tick_normal_no_sensor_no_change.rvc` | Common periodic behavior, UC2 Set Cleaning Mode | `clockTick()` | Negative | sensor event 없는 `NormalMode` clock tick은 cleaning active 상태를 유지 |
| N25 | `tc/N25_dust_while_charging_ignored.rvc` | UC6 Adjust Boost Mode, UC10 Charge Battery | `dustDetected()` while charging | Negative | charging 중 dust 입력은 current implementation guard에 의해 ignored, `NormalMode`와 boost off 유지 |
| N26 | `tc/N26_obstacle_front_clear_standby_no_motor.rvc` | UC5 Avoid Obstacle | `obstacleDetected(left)` | Negative | front clear인 obstacle 입력도 `StandbyMode`에서는 motor를 start하지 않음 |
| N27 | `tc/N27_obstacle_all_blocked_standby_no_motor.rvc` | UC5 Avoid Obstacle | `obstacleDetected(all)` | Negative | all blocked obstacle 입력도 `StandbyMode`에서는 motor를 start하지 않음 |
| N28 | `tc/N28_timer_after_boost_return_stays_normal.rvc` | UC6 Adjust Boost Mode | `timerExpired()` | Negative | `BoostMode`에서 `NormalMode`로 복귀한 뒤 추가 timer는 `NormalMode` 유지 |
| N29 | `tc/N29_charge_after_cleaning_mode_rejected.rvc` | UC10 Charge Battery | `chargeBattery()` | Negative | cleaning mode 진입 뒤 charge 요청은 rejected, standby 복귀 뒤에는 charging accepted |
| N30 | `tc/N30_unknown_command_no_crash.rvc` | Test simulator interface | unknown CLI command | Negative | unknown command 출력 후 crash 없이 `Off` 및 stopped 상태 유지 |
