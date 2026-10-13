# CLI regression tests.
#
# Each test runs spvdb in --source (non-interactive) mode and validates stdout
# with PASS_REGULAR_EXPRESSION / FAIL_REGULAR_EXPRESSION.
#
# Scripts use @SPIRV_BIN_DIR@ and @CLI_TEST_DIR@ placeholders that are
# substituted by configure_file() below. SPIRV_BIN_DIR is inherited from the
# parent directory scope (tests/CMakeLists.txt), which is where the
# spvdb_test_spirv target actually assembles the .spv binaries this points at.

set(CLI_TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# Configure script files (substitute path placeholders).
set(SCRIPTS basic_run.spvdbrc source_command.spvdbrc)
foreach(SCRIPT ${SCRIPTS})
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT}"
        "${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT}"
        @ONLY
    )
endforeach()

# Convenience macro.
macro(add_cli_test NAME SCRIPT)
    add_test(
        NAME cli_${NAME}
        COMMAND $<TARGET_FILE:spvdb-cli> --source "${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT}.spvdbrc"
    )
    set_tests_properties(cli_${NAME} PROPERTIES
        DEPENDS spvdb_test_spirv
        ${ARGN}
    )
endmacro()

# basic_run: load add_uint.spv, bind [3]+[4], run, expect output 7.
add_cli_test(basic_run basic_run
    PASS_REGULAR_EXPRESSION "Program finished normally"
    FAIL_REGULAR_EXPRESSION "Panic|Error|Unknown command"
)

# source_command: run basic_run via 'source'; same expected output proves
# the source command dispatches correctly.
add_cli_test(source_command source_command
    PASS_REGULAR_EXPRESSION "Program finished normally"
    FAIL_REGULAR_EXPRESSION "Panic|Error|Unknown command|Cannot open file"
)
