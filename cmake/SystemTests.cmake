# Simulator-based System Tests
# - Google Test is not used here.
# - Each .rvc file is executed by rvc_simulator.

set(OOAD_SYSTEM_TEST_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/system_tests/tc
)

file(GLOB OOAD_SYSTEM_TEST_SCRIPTS CONFIGURE_DEPENDS
    ${OOAD_SYSTEM_TEST_DIR}/*.rvc
)

foreach(SYSTEM_TEST_SCRIPT ${OOAD_SYSTEM_TEST_SCRIPTS})
    get_filename_component(SYSTEM_TEST_NAME ${SYSTEM_TEST_SCRIPT} NAME_WE)

    add_test(
        NAME system_${SYSTEM_TEST_NAME}
        COMMAND $<TARGET_FILE:rvc_simulator> ${SYSTEM_TEST_SCRIPT}
    )

    set_tests_properties(system_${SYSTEM_TEST_NAME}
        PROPERTIES
            LABELS "system"
    )
endforeach()
