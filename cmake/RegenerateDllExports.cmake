# RegenerateDllExports.cmake
#
# This script extracts exported function names from a Windows DLL using dumpbin
# and regenerates a module definition (.def) file with forwarding directives.
#
# Usage:
#   cmake -DDLL_PATH=<path-to-dll> -DOUTPUT_DEF=<output-def-path> -DTARGET_DLL_NAME=<target-name> -P RegenerateDllExports.cmake
#
# Required variables:
#   DLL_PATH - Path to the DLL to extract exports from
#   OUTPUT_DEF - Path where the generated .def file should be written
#   TARGET_DLL_NAME - Name of the DLL to forward to (e.g., "slang-compiler")

if(NOT DEFINED DLL_PATH)
    message(FATAL_ERROR "DLL_PATH must be defined")
endif()

if(NOT DEFINED OUTPUT_DEF)
    message(FATAL_ERROR "OUTPUT_DEF must be defined")
endif()

if(NOT DEFINED TARGET_DLL_NAME)
    message(FATAL_ERROR "TARGET_DLL_NAME must be defined")
endif()

if(NOT EXISTS "${DLL_PATH}")
    message(FATAL_ERROR "DLL not found: ${DLL_PATH}")
endif()

# Find dumpbin - should be available in Visual Studio environment
find_program(DUMPBIN_EXECUTABLE dumpbin)
if(NOT DUMPBIN_EXECUTABLE)
    message(
        FATAL_ERROR
        "dumpbin not found. Make sure you are running in a Visual Studio Developer Command Prompt."
    )
endif()

message(STATUS "Extracting exports from: ${DLL_PATH}")
message(STATUS "Using dumpbin: ${DUMPBIN_EXECUTABLE}")

# Run dumpbin to get exports
execute_process(
    COMMAND "${DUMPBIN_EXECUTABLE}" /EXPORTS "${DLL_PATH}"
    OUTPUT_VARIABLE DUMPBIN_OUTPUT
    ERROR_VARIABLE DUMPBIN_ERROR
    RESULT_VARIABLE DUMPBIN_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT DUMPBIN_RESULT EQUAL 0)
    message(FATAL_ERROR "dumpbin failed with error: ${DUMPBIN_ERROR}")
endif()

# Parse the dumpbin output
# Expected format:
#     ordinal hint RVA      name
#           1    0 00001000 function_name
#           2    1 00001010 another_function
#
# We need to extract the function names (4th column)
# Skip lines that contain "DATA" as those are data exports, not functions

set(EXPORT_FUNCTIONS "")
set(IN_EXPORTS_SECTION FALSE)

string(REPLACE "\n" ";" DUMPBIN_LINES "${DUMPBIN_OUTPUT}")

foreach(LINE ${DUMPBIN_LINES})
    # Look for the section header
    if(LINE MATCHES "ordinal hint")
        set(IN_EXPORTS_SECTION TRUE)
        continue()
    endif()

    # Skip until we're in the exports section
    if(NOT IN_EXPORTS_SECTION)
        continue()
    endif()

    # Stop at the "Summary" section
    if(LINE MATCHES "Summary")
        break()
    endif()

    # Skip empty lines
    string(STRIP "${LINE}" LINE_STRIPPED)
    if(LINE_STRIPPED STREQUAL "")
        continue()
    endif()

    # Skip DATA exports
    if(LINE MATCHES "DATA")
        continue()
    endif()

    # Parse the line to extract the function name
    # Match pattern: whitespace ordinal whitespace hint whitespace RVA whitespace name [= implementation]
    # We want the name (before the = sign if present, or the last token)
    if(
        LINE_STRIPPED
            MATCHES
            "^[ \t]*[0-9]+[ \t]+[0-9A-Fa-f]+[ \t]+[0-9A-Fa-f]+[ \t]+(.+)$"
    )
        set(NAME_PART "${CMAKE_MATCH_1}")
        # If there's an = sign, take everything before it, otherwise take the whole thing
        if(NAME_PART MATCHES "^([^ \t]+)[ \t]*=")
            set(FUNCTION_NAME "${CMAKE_MATCH_1}")
        else()
            set(FUNCTION_NAME "${NAME_PART}")
        endif()

        string(STRIP "${FUNCTION_NAME}" FUNCTION_NAME)

        if(FUNCTION_NAME)
            list(APPEND EXPORT_FUNCTIONS "${FUNCTION_NAME}")
        endif()
    endif()
endforeach()

list(LENGTH EXPORT_FUNCTIONS NUM_EXPORTS)
message(STATUS "Found ${NUM_EXPORTS} exported functions")

if(NUM_EXPORTS EQUAL 0)
    message(FATAL_ERROR "No exports found in ${DLL_PATH}")
endif()

# Generate the .def file with forwarding directives
set(DEF_CONTENT "LIBRARY slang\n")
set(DEF_CONTENT "${DEF_CONTENT}EXPORTS\n")

foreach(FUNC ${EXPORT_FUNCTIONS})
    set(DEF_CONTENT "${DEF_CONTENT}    ${FUNC} = ${TARGET_DLL_NAME}.${FUNC}\n")
endforeach()

# Write the .def file
file(WRITE "${OUTPUT_DEF}" "${DEF_CONTENT}")
message(STATUS "Generated DEF file: ${OUTPUT_DEF}")
