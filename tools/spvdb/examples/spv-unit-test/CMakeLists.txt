# examples/spv-unit-test/CMakeLists.txt
#
# Standalone example: GPU unit testing with spvdb.
#
# Build and run:
#
#   cmake -B build \
#     -DSLANG_DIR=/path/to/slang-release \
#     -DSPVDB=/path/to/spvdb/build/spvdb
#   cmake --build build
#   ctest --test-dir build -V

cmake_minimum_required(VERSION 3.20)
project(spv_unit_test_example LANGUAGES NONE)

enable_testing()

# ---- Find slangc ------------------------------------------------------------

find_program(SLANGC slangc HINTS $ENV{SLANG_DIR}/bin REQUIRED)
message(STATUS "slangc: ${SLANGC}")

# ---- Find the spvdb CLI -----------------------------------------------------
#
# Pass -DSPVDB=/absolute/path/to/spvdb if it is not already on PATH.

find_program(SPVDB spvdb REQUIRED)
message(STATUS "spvdb:  ${SPVDB}")

# ---- Compile the test shader to SPIR-V at build time -----------------------

set(TEST_SHADER ${CMAKE_CURRENT_SOURCE_DIR}/test_shader.slang)
set(TEST_SPV    ${CMAKE_CURRENT_BINARY_DIR}/test_shader.spv)

add_custom_command(
    OUTPUT  ${TEST_SPV}
    COMMAND ${SLANGC} -target spirv -g3 -O0 -o ${TEST_SPV} ${TEST_SHADER}
    DEPENDS ${TEST_SHADER}
    COMMENT "Compiling test_shader.slang → test_shader.spv"
)

add_custom_target(test_shader_spv ALL DEPENDS ${TEST_SPV})

# ---- Register one CTest entry per test_ entrypoint -------------------------
#
# Each entry point is run with:
#
#   spvdb test_shader.spv --entry <name> --run
#
# The CLI exits 0 on normal completion (all assertions passed) and 1 on panic
# (an ASSERT_EQ failed and triggered an out-of-bounds write in the shader).
#
# To add a new test, define a [shader("compute")] function named test_* in
# test_shader.slang and add its name to the list below.

set(TEST_ENTRYPOINTS
    test_arithmetic
    test_bitwise
    test_bad_assumption     # intentionally fails — demonstrates failure output
)

foreach(EP ${TEST_ENTRYPOINTS})
    add_test(
        NAME    ${EP}
        COMMAND ${SPVDB} ${TEST_SPV} --entry ${EP} --run
    )
endforeach()
