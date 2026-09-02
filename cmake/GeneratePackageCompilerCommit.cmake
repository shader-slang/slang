# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(commit "unknown")
set(dirty 0)
if(GIT_EXECUTABLE AND EXISTS "${SOURCE_DIR}/.git")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${SOURCE_DIR}" rev-parse HEAD
        RESULT_VARIABLE commit_result
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE commit_output
    )
    if(commit_result EQUAL 0)
        set(commit "${commit_output}")
        execute_process(
            COMMAND
                "${GIT_EXECUTABLE}" -C "${SOURCE_DIR}" diff --quiet
                --ignore-submodules HEAD --
            RESULT_VARIABLE diff_result
        )
        if(NOT diff_result EQUAL 0)
            set(dirty 1)
        endif()
    endif()
endif()

set(contents
    "#define SLANG_PACKAGE_COMPILER_COMMIT \"${commit}\"\n#define SLANG_PACKAGE_COMPILER_DIRTY ${dirty}\n"
)
set(write_file TRUE)
if(EXISTS "${OUTPUT_FILE}")
    file(READ "${OUTPUT_FILE}" old_contents)
    if("${old_contents}" STREQUAL "${contents}")
        set(write_file FALSE)
    endif()
endif()
if(write_file)
    file(WRITE "${OUTPUT_FILE}" "${contents}")
endif()
