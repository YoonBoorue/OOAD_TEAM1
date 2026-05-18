cmake_minimum_required(VERSION 3.20)
project(RVCVibeControlSW LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VIBE_BUILD_TESTS "Build Google Test unit tests" ON)

add_library(rvc_vibe_core
    src/Controller.cpp
    src/DustProcessor.cpp
    src/ObstacleProcessor.cpp
    src/OperatingMode.cpp
)

target_include_directories(rvc_vibe_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(rvc_vibe_core PUBLIC cxx_std_17)

if (MSVC)
    target_compile_options(rvc_vibe_core PRIVATE /W4)
else()
    target_compile_options(rvc_vibe_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(rvc_vibe_simulator
    simulator/main.cpp
)

target_link_libraries(rvc_vibe_simulator
    PRIVATE
        rvc_vibe_core
)

include(CTest)

if (BUILD_TESTING AND VIBE_BUILD_TESTS)
    find_package(GTest QUIET)

    if (NOT GTest_FOUND)
        set(_LOCAL_GOOGLETEST_SRC
            "${CMAKE_CURRENT_SOURCE_DIR}/../build/_deps/googletest-src"
        )

        if (EXISTS "${_LOCAL_GOOGLETEST_SRC}/CMakeLists.txt")
            add_subdirectory(
                "${_LOCAL_GOOGLETEST_SRC}"
                "${CMAKE_CURRENT_BINARY_DIR}/_deps/googletest-build"
                EXCLUDE_FROM_ALL
            )
        else()
            include(FetchContent)
            FetchContent_Declare(
                googletest
                URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
            )
            set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
            FetchContent_MakeAvailable(googletest)
        endif()
    endif()

    file(GLOB_RECURSE VIBE_TEST_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
    )

    if (VIBE_TEST_SOURCES)
        add_executable(rvc_vibe_tests
            ${VIBE_TEST_SOURCES}
        )

        target_link_libraries(rvc_vibe_tests
            PRIVATE
                rvc_vibe_core
        )

        if (TARGET GTest::gtest_main)
            target_link_libraries(rvc_vibe_tests PRIVATE GTest::gtest_main)
        else()
            target_link_libraries(rvc_vibe_tests PRIVATE gtest_main)
        endif()

        include(GoogleTest)
        gtest_discover_tests(rvc_vibe_tests)
    endif()
endif()
