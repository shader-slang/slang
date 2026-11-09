find_package(Git)

# Registers the Git references that determine `git describe` as configure inputs.
#
# CMake otherwise has no reason to reconfigure after a commit or checkout. A generator build can
# then retain the version header from an older revision, which makes an incompatible host generator
# appear current to a cross build.
function(_watch_git_version dir)
    set(git_dir "${dir}/.git")
    if(EXISTS "${git_dir}" AND NOT IS_DIRECTORY "${git_dir}")
        file(READ "${git_dir}" git_dir_file)
        string(STRIP "${git_dir_file}" git_dir_file)
        if(git_dir_file MATCHES "^gitdir: (.+)$")
            set(git_dir "${CMAKE_MATCH_1}")
            if(NOT IS_ABSOLUTE "${git_dir}")
                get_filename_component(git_dir "${dir}/${git_dir}" ABSOLUTE)
            endif()
        endif()
    endif()

    if(NOT EXISTS "${git_dir}/HEAD")
        return()
    endif()

    set_property(
        DIRECTORY
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS "${git_dir}/HEAD"
    )
    file(READ "${git_dir}/HEAD" git_head)
    string(STRIP "${git_head}" git_head)
    if(git_head MATCHES "^ref: (.+)$")
        set_property(
            DIRECTORY
            APPEND
            PROPERTY CMAKE_CONFIGURE_DEPENDS "${git_dir}/${CMAKE_MATCH_1}"
        )
    endif()
endfunction()

# Helper function to display consistent warning message about missing Slang version from git tags
function(_warn_missing_git_version var version context_msg)
    message(
        WARNING
        "${context_msg}\n"
        "Git tags are the AUTHORITATIVE source for version information in this project.\n"
        "If you cloned from https://github.com/shader-slang/slang.git, fetch tags:\n"
        "    git fetch --tags\n"
        "If you cloned from a mirror or fork, fetch tags from the official repository:\n"
        "    git fetch https://github.com/shader-slang/slang.git 'refs/tags/*:refs/tags/*'\n"
        "Key implications of using an incorrect version number:\n"
        "  (a) Version APIs and headers will return incorrect version information\n"
        "  (b) Versioned filenames will be incorrect\n"
        "If you cannot fetch tags, you can use -D${var}=<version> as a fallback,\n"
        "but this is NOT the preferred method.\n"
        "Falling back to default version: ${version}"
    )
endfunction()

# Extract a version from the latest tag matching something like v1.2.3.4
function(get_git_version var_numeric var dir)
    _watch_git_version("${dir}")

    set(version_numeric "0.0.0")
    set(version "0.0.0-unknown")
    set(version_out "")

    # First, try reading from slang_git_version file.
    set(version_file "${dir}/cmake/slang_git_version")
    if(EXISTS "${version_file}")
        file(READ "${version_file}" version_file_content)
        string(STRIP "${version_file_content}" version_file_content)
        if(
            NOT version_file_content MATCHES "^\\$Format:"
            AND version_file_content MATCHES "^v20[2-9][0-9]\\.[0-9]"
        )
            set(version_out "${version_file_content}")
            message(
                STATUS
                "Using version from slang_git_version file: ${version_out}"
            )
        endif()
    endif()

    # If slang_git_version file didn't provide a valid version, try git describe.
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
                --dirty
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
            elseif(NOT DEFINED ${var})
                _warn_missing_git_version(
                    "${var}"
                    "${version}"
                    "Failed to get version information from git tags."
                )
            endif()
        elseif(NOT GIT_EXECUTABLE AND NOT DEFINED ${var})
            _warn_missing_git_version(
                "${var}"
                "${version}"
                "Git executable not found - unable to get version information from git tags.\nPlease install git and fetch tags from the remote repository."
            )
        elseif(NOT EXISTS "${dir}/.git" AND NOT DEFINED ${var})
            _warn_missing_git_version(
                "${var}"
                "${version}"
                "Git repository not found - unable to get version information from git tags.\nIf you cloned the repository, please fetch tags from the remote."
            )
        endif()
    endif()

    # Parse the version string (from either slang_git_version file or git describe).
    if(version_out AND "${version_out}" MATCHES "^v(([0-9]+(\\.[0-9]+)*).*)")
        set(version "${CMAKE_MATCH_1}")
        set(version_numeric "${CMAKE_MATCH_2}")
    elseif(version_out)
        message(
            WARNING
            "Couldn't parse version (like v2025.21 or v2025.21-foo) from ${version_out}, using ${version} for now"
        )
    elseif(DEFINED ${var})
        # An explicit cache value remains the fallback for source archives without usable Git
        # metadata. Git metadata is authoritative when available, so stale auto-detected cache
        # entries cannot survive a checkout.
        set(version "${${var}}")
        if(DEFINED ${var_numeric})
            set(version_numeric "${${var_numeric}}")
        endif()
    endif()

    if(version_out)
        set(cache_update FORCE)
    else()
        set(cache_update "")
    endif()
    set(${var_numeric}
        ${version_numeric}
        CACHE STRING
        "The project version numeric part, detected using git if available"
        ${cache_update}
    )
    set(${var}
        ${version}
        CACHE STRING
        "The project version, detected using git if available"
        ${cache_update}
    )
endfunction()
