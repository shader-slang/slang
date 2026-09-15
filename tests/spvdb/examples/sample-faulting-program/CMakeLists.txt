# examples/sample-faulting-program/CMakeLists.txt
#
# Standalone example: a realistic compute shader with a subtle bug,
# intended to be explored interactively with the spvdb debugger.
#
# Build:
#   cmake -B build -DSLANGC=/path/to/slangc
#   cmake --build build
#
# Then run the .spv through spvdb as described in the instructions below.

cmake_minimum_required(VERSION 3.20)
project(sample_faulting_program LANGUAGES NONE)

find_program(SLANGC slangc HINTS $ENV{SLANG_DIR}/bin REQUIRED)
message(STATUS "slangc: ${SLANGC}")

set(SHADER ${CMAKE_CURRENT_SOURCE_DIR}/signal_smooth.slang)
set(SPV    ${CMAKE_CURRENT_BINARY_DIR}/signal_smooth.spv)

# -g3: maximal debug info (full source locations in every frame)
# -O0: no inlining, so apply_kernel and fetch_clamped appear as distinct frames
add_custom_command(
    OUTPUT  ${SPV}
    COMMAND ${SLANGC} -target spirv -g3 -O0 -o ${SPV} ${SHADER}
    DEPENDS ${SHADER}
    COMMENT "Compiling signal_smooth.slang → signal_smooth.spv"
)

add_custom_target(signal_smooth_spv ALL DEPENDS ${SPV})
