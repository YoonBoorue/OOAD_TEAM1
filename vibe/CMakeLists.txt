cmake_minimum_required(VERSION 3.20)
project(RVCSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VIBE_BUILD_TESTS "Build GoogleTest-based tests" ON)
option(VIBE_BUILD_SYSTEM_TESTS "Register simulator script system tests" ON)

set(RVC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

add_library(vibe_rvc_core
    "${RVC_ROOT}/src/rvc/DustProcessor.cpp"
    "${RVC_ROOT}/src/rvc/ObstacleProcessor.cpp"
    "${RVC_ROOT}/src/rvc/Modes.cpp"
    "${RVC_ROOT}/src/rvc/Controller.cpp"
    "${RVC_ROOT}/src/rvc/BatteryDriver.cpp"
    "${RVC_ROOT}/src/rvc/CleanerDriver.cpp"
    "${RVC_ROOT}/src/rvc/DustSensorDriver.cpp"
    "${RVC_ROOT}/src/rvc/MotorDriver.cpp"
    "${RVC_ROOT}/src/rvc/ObstacleSensorDriver.cpp"
)

target_include_directories(vibe_rvc_core
    PUBLIC
        "${RVC_ROOT}/include"
)

target_compile_features(vibe_rvc_core PUBLIC cxx_std_17)

add_library(vibe_sim_core
    sim/src/AnsiRenderer.cpp
    sim/src/DirectionUtil.cpp
    sim/src/Environment.cpp
    sim/src/KeyInput.cpp
    sim/src/MapLoader.cpp
    sim/src/Renderer.cpp
    sim/src/Room.cpp
    sim/src/RvcAdapter.cpp
    sim/src/SimulationLoop.cpp
)

target_include_directories(vibe_sim_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/sim/include"
)

target_link_libraries(vibe_sim_core
    PUBLIC
        vibe_rvc_core
)

target_compile_features(vibe_sim_core PUBLIC cxx_std_17)

add_executable(rvc_simulator
    sim/app/main.cpp
)

target_link_libraries(rvc_simulator
    PRIVATE
        vibe_sim_core
)

if (MSVC)
    target_compile_options(vibe_rvc_core PRIVATE /W4)
    target_compile_options(vibe_sim_core PRIVATE /W4)
    target_compile_options(rvc_simulator PRIVATE /W4)
else()
    target_compile_options(vibe_rvc_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(vibe_sim_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(rvc_simulator PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)

if (BUILD_TESTING AND VIBE_BUILD_SYSTEM_TESTS)
    file(GLOB VIBE_SYSTEM_TEST_SCRIPTS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/system_tests/tc/*.rvc"
    )

    foreach(_VIBE_SYSTEM_TEST_SCRIPT IN LISTS VIBE_SYSTEM_TEST_SCRIPTS)
        get_filename_component(_VIBE_SYSTEM_TEST_NAME "${_VIBE_SYSTEM_TEST_SCRIPT}" NAME_WE)
        add_test(
            NAME "system_${_VIBE_SYSTEM_TEST_NAME}"
            COMMAND rvc_simulator --script "${_VIBE_SYSTEM_TEST_SCRIPT}"
        )
        set_tests_properties("system_${_VIBE_SYSTEM_TEST_NAME}" PROPERTIES
            LABELS "system"
        )
    endforeach()
endif()

if (BUILD_TESTING AND VIBE_BUILD_TESTS)
    find_package(GTest QUIET)

    if (NOT TARGET GTest::gtest_main)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        set(LOCAL_GOOGLETEST_DIR "${RVC_ROOT}/build/_deps/googletest-src")

        if (EXISTS "${LOCAL_GOOGLETEST_DIR}/CMakeLists.txt")
            add_subdirectory(
                "${LOCAL_GOOGLETEST_DIR}"
                "${CMAKE_CURRENT_BINARY_DIR}/_deps/googletest-local-build"
                EXCLUDE_FROM_ALL
            )
        else()
            include(FetchContent)

            FetchContent_Declare(
                googletest
                URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
                DOWNLOAD_EXTRACT_TIMESTAMP TRUE
            )

            FetchContent_MakeAvailable(googletest)
        endif()
    endif()

    file(GLOB_RECURSE VIBE_UNIT_TEST_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_tests/*.cpp"
    )

    if (VIBE_UNIT_TEST_SOURCES)
        add_executable(rvc_vibe_tests
            ${VIBE_UNIT_TEST_SOURCES}
        )

        target_link_libraries(rvc_vibe_tests
            PRIVATE
                vibe_rvc_core
                GTest::gtest_main
        )

        include(GoogleTest)
        gtest_discover_tests(rvc_vibe_tests)
    endif()

    add_executable(rvc_simulator_tests
        sim/tests/SimulationLoopTest.cpp
    )

    target_link_libraries(rvc_simulator_tests
        PRIVATE
            vibe_sim_core
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(rvc_simulator_tests)
endif()