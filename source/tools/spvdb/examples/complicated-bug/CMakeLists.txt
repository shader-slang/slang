cmake_minimum_required(VERSION 3.20)
project(spectrum_features_example LANGUAGES NONE)

# ---------------------------------------------------------------------------
# Locate tools
# ---------------------------------------------------------------------------

find_program(SLANGC slangc REQUIRED
    DOC "Path to the Slang compiler (slangc)")

find_program(SPVDB spvdb REQUIRED
    DOC "Path to the spvdb debugger")

# ---------------------------------------------------------------------------
# Compile the Slang shader to SPIR-V
# ---------------------------------------------------------------------------

set(SHADER  "${CMAKE_CURRENT_SOURCE_DIR}/spectrum_features.slang")
set(OUT_SPV "${CMAKE_CURRENT_BINARY_DIR}/spectrum_features.spv")

add_custom_command(
    OUTPUT  "${OUT_SPV}"
    COMMAND "${SLANGC}"
            -target spirv
            -g3
            -O0
            -o "${OUT_SPV}"
            "${SHADER}"
    DEPENDS "${SHADER}"
    COMMENT "Compiling spectrum_features.slang → spectrum_features.spv"
    VERBATIM
)

add_custom_target(spectrum_features_spv ALL
    DEPENDS "${OUT_SPV}"
)

# ---------------------------------------------------------------------------
# Usage summary
# ---------------------------------------------------------------------------

message(STATUS "")
message(STATUS "Build complete.  To run the demo:")
message(STATUS "")
message(STATUS "  # Interactive session (good input — completes normally):")
message(STATUS "  ${SPVDB} ${OUT_SPV} --source ${CMAKE_CURRENT_SOURCE_DIR}/run_clean.spvdbrc")
message(STATUS "")
message(STATUS "  # Interactive session (bad input — triggers the bug):")
message(STATUS "  ${SPVDB} ${OUT_SPV} --source ${CMAKE_CURRENT_SOURCE_DIR}/run_buggy.spvdbrc")
message(STATUS "")
message(STATUS "  # Interactive debugging session:")
message(STATUS "  ${SPVDB} ${OUT_SPV}")
message(STATUS "")
