find_package(Git)

# Extract a version from the latest tag matching something like v1.2.3.4
function(get_git_version var_numeric var dir)
    if(NOT DEFINED ${var})
        set(version_numeric "0.0.0")
        set(version "0.0.0-unknown")
        set(version_out "")

        # First, try reading from SLANG_VERSION file
        set(version_file "${dir}/SLANG_VERSION")
        if(EXISTS "${version_file}")
            file(READ "${version_file}" version_file_content)
            string(STRIP "${version_file_content}" version_file_content)
            # Check if it's not a git archive format string and matches expected pattern
            if(
                NOT version_file_content MATCHES "^\\$Format:"
                AND version_file_content MATCHES "^v20[2-9][0-9]\\.[0-9]"
            )
                set(version_out "${version_file_content}")
                message(
                    STATUS
                    "Using version from SLANG_VERSION file: ${version_out}"
                )
            endif()
        endif()

        # If SLANG_VERSION file didn't provide a valid version, try git describe
        if(NOT version_out)
            if(GIT_EXECUTABLE AND EXISTS "${dir}/.git")
                set(command
                    "${GIT_EXECUTABLE}"
                    -C
                    "${dir}"
                    describe
                    --tags
                    --match
                    v20[2-9][0-9].[0-9]*
                )
                execute_process(
                    COMMAND ${command}
                    RESULT_VARIABLE result
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    OUTPUT_VARIABLE git_describe_out
                )
                if(result EQUAL 0)
                    set(version_out "${git_describe_out}")
                    message(
                        STATUS
                        "Using version from git describe: ${version_out}"
                    )
                else()
                    message(
                        WARNING
                        "Getting ${var} failed: ${command} returned ${result}\nIs this a Git repo with tags?\nConsider settings -D${var} to specify a version manually"
                    )
                endif()
            elseif(NOT GIT_EXECUTABLE)
                message(
                    WARNING
                    "Couldn't find git executable to get ${var}, please use -D${var}, using ${version} for now"
                )
            endif()
        endif()

        # Parse the version string (from either VERSION file or git describe)
        if(
            version_out
            AND "${version_out}" MATCHES "^v(([0-9]+(\\.[0-9]+)*).*)"
        )
            set(version "${CMAKE_MATCH_1}")
            set(version_numeric "${CMAKE_MATCH_2}")
        elseif(version_out)
            message(
                WARNING
                "Couldn't parse version (like v2025.21 or v2025.21-foo) from ${version_out}, using ${version} for now"
            )
        endif()
    endif()

    set(${var_numeric}
        ${version_numeric}
        CACHE STRING
        "The project version numeric part, detected using git if available"
    )
    set(${var}
        ${version}
        CACHE STRING
        "The project version, detected using git if available"
    )
endfunction()
