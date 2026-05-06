cmake_minimum_required(VERSION 3.20)
project(OOAD_TEAM1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(OOAD_ENABLE_COVERAGE "Enable coverage flags" OFF)

add_library(ooad_core
    src/rvc/RobotController.cpp
    src/rvc/DustProcessor.cpp
    src/rvc/ObstacleProcessor.cpp
    src/rvc/Modes.cpp
    src/rvc/Controller.cpp
    src/rvc/BatteryDriver.cpp
    src/rvc/CleanerDriver.cpp
    src/rvc/DustSensorDriver.cpp
    src/rvc/MotorDriver.cpp
    src/rvc/ObstacleSensorDriver.cpp
)

target_include_directories(ooad_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(ooad_core PUBLIC cxx_std_17)

if (MSVC)
    target_compile_options(ooad_core PRIVATE /W4)
else()
    target_compile_options(ooad_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (OOAD_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ooad_core PUBLIC -O0 -g --coverage)
    target_link_options(ooad_core PUBLIC --coverage)
endif()

add_executable(OOAD_TEAM1
    src/main.cpp
)

target_link_libraries(OOAD_TEAM1
    PRIVATE
        ooad_core
)

add_executable(rvc_simulator
    simulator/main.cpp
)

target_link_libraries(rvc_simulator
    PRIVATE
        ooad_core
)

include(CTest)

if (BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    add_executable(ooad_tests
        tests/RobotControllerTest.cpp
        tests/DustDetectedTest.cpp
    )

    target_link_libraries(ooad_tests
        PRIVATE
            ooad_core
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(ooad_tests)
endif()