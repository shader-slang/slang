find_package(Git)

# Extract a version from the latest tag matching something like v1.2.3.4
function(get_git_version var dir)
    if(NOT DEFINED ${var})
        set(version "0.0")
        if(GIT_EXECUTABLE)
            set(command
                "${GIT_EXECUTABLE}"
                -C
                "${dir}"
                describe
                --abbrev=0
                --tags
                --match
                v*
            )
            execute_process(
                COMMAND ${command}
                RESULT_VARIABLE result
                OUTPUT_STRIP_TRAILING_WHITESPACE
                OUTPUT_VARIABLE version
            )
            if(NOT result EQUAL 0)
                message(
                    WARNING
                    "Getting ${var} failed: ${command} returned ${result}"
                )
            elseif("${version}" MATCHES "^v([0-9]+(\.[0-9]+)*).*")
                set(version "${CMAKE_MATCH_1}")
            else()
                message(
                    WARNING
                    "Couldn't parse numeric version (like v1.2.3) from ${version}"
                )
            endif()
        else()
            message(
                WARNING
                "Couldn't find git executable to get ${var}, please use -D${var}"
            )
        endif()
    endif()

    set(${var}
        ${version}
        CACHE STRING
        "The project version, detected using git if available"
    )
endfunction()
