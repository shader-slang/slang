if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(dep_shared_library_prefix "")
else()
    set(dep_shared_library_prefix "lib")
endif()

# Helper function to download and extract an archive
function(download_and_extract archive_name url)
    cmake_path(GET url FILENAME filename_with_ext)
    cmake_path(GET url STEM LAST_ONLY file_stem)
    set(archive_path "${CMAKE_CURRENT_BINARY_DIR}/${filename_with_ext}")
    set(extract_dir "${CMAKE_CURRENT_BINARY_DIR}/${file_stem}")

    # Check if already extracted
    file(GLOB EXTRACT_DIR_CONTENTS "${extract_dir}/*")
    if(EXTRACT_DIR_CONTENTS)
        message(STATUS "Using existing extracted files in ${extract_dir}")
    else()
        # Check if archive already exists
        if(EXISTS ${url})
            message(STATUS "Using local file for ${archive_name}: ${url}")
            set(archive_path ${url})
        elseif(EXISTS ${archive_path})
            message(
                STATUS
                "Using existing archive for ${archive_name}: ${archive_path}"
            )
        else()
            message(STATUS "Downloading ${archive_name} from ${url} ...")
            # Retry before giving up: transient HTTP failures are common when many CI
            # jobs fetch release assets from behind a single shared egress address.
            # Any failure is retried, so a genuinely missing asset pays the full backoff.
            set(max_attempts 3)
            # A stalled connection has to fail before the retry below can get control
            # back, so bound it. INACTIVITY_TIMEOUT rather than TIMEOUT: it fires only
            # when no data is arriving, whereas a wall-clock limit would also abort a
            # slow but healthy transfer of an archive this size.
            set(inactivity_timeout_seconds 60)
            # Linear backoff between attempts: 5s then 10s, with no wait after the last try.
            set(retry_base_delay_seconds 5)
            foreach(attempt RANGE 1 ${max_attempts})
                file(
                    DOWNLOAD ${url} ${archive_path}
                    STATUS status
                    INACTIVITY_TIMEOUT ${inactivity_timeout_seconds}
                )

                list(GET status 0 status_code)
                list(GET status 1 status_string)
                if(status_code EQUAL 0)
                    break()
                endif()

                # A failed download still leaves a file behind, and extracting an empty
                # archive succeeds silently, so the partial file has to be removed. Left
                # in place it would be picked up by the "existing archive" branch above
                # on every subsequent configure, and the fetch would never recover.
                file(REMOVE ${archive_path})

                if(attempt LESS max_attempts)
                    math(
                        EXPR
                        retry_delay
                        "${retry_base_delay_seconds} * ${attempt}"
                    )
                    message(
                        STATUS
                        "Download of ${archive_name} failed (${status_string}), retrying in ${retry_delay}s ..."
                    )
                    execute_process(
                        COMMAND ${CMAKE_COMMAND} -E sleep ${retry_delay}
                    )
                endif()
            endforeach()

            # status_code is the last attempt's result: nonzero unless the loop broke on success.
            if(NOT status_code EQUAL 0)
                message(
                    WARNING
                    "Failed to download ${archive_name} from ${url}: ${status_string} with status code ${status_code}"
                )
                return()
            endif()
        endif()

        file(ARCHIVE_EXTRACT INPUT ${archive_path} DESTINATION ${extract_dir})
        message(STATUS "${archive_name} extracted to ${extract_dir}")
    endif()

    set(${archive_name}_SOURCE_DIR ${extract_dir} PARENT_SCOPE)
endfunction()

# Add rules to copy & install shared library of name 'library_name' in the 'module_subdir' directory.
# If 'url' is a directory, the shared library (with platform-specific shared library prefixes and suffixes) will be
# taken from the directory, and whatever is found there will be used to produce the install rule.
# If the 'url' is a path to a file with the platform-specific shared library prefix and suffix, then that file
# will be used to produce the install rule.
# Otherwise, the 'url' is interpreted as an URL, and the content of the URL will be fetched, extracted and searched
# for the shared library to produce the install rule.
function(copy_fetched_shared_library library_name url)
    cmake_parse_arguments(ARG "IGNORE_FAILURE" "" "" ${ARGN})

    if(ARG_IGNORE_FAILURE)
        set(error_type STATUS)
    else()
        set(error_type SEND_ERROR)
    endif()

    set(shared_library_filename
        "${dep_shared_library_prefix}${library_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )
    macro(from_glob dir)
        # A little helper function
        file(GLOB_RECURSE source_object "${dir}/${shared_library_filename}")
        list(LENGTH source_object nmatches)
        if(nmatches EQUAL 0)
            message(
                ${error_type}
                "Unable to find ${shared_library_filename} in ${url}"
            )
        elseif(nmatches GREATER 1)
            message(
                ${error_type}
                "Found multiple files named ${shared_library_filename} in ${url}"
            )
        endif()
    endmacro()

    if(IS_DIRECTORY "${url}")
        # Just glob directly from a local directory
        from_glob("${url}")
    elseif(
        url
            MATCHES
            "${dep_shared_library_prefix}.+${CMAKE_SHARED_LIBRARY_SUFFIX}$"
        AND EXISTS "${url}"
    )
        # Otherwise, if it's a direct path to a shared object, use that
        set(source_object "${url}")
    else()
        # Otherwise, download and extract from whatever URL we have
        download_and_extract("${library_name}" "${url}")
        if(DEFINED ${library_name}_SOURCE_DIR)
            from_glob(${${library_name}_SOURCE_DIR})
        elseif(ARG_IGNORE_FAILURE)
            return()
        else()
            message(
                SEND_ERROR
                "Unable to download and extract ${library_name} from ${url}"
            )
            return()
        endif()
    endif()

    # We didn't find it, just return and don't create a target and operation
    # which will fail
    if((NOT EXISTS "${source_object}") AND ARG_IGNORE_FAILURE)
        return()
    endif()

    set(dest_object
        ${CMAKE_BINARY_DIR}/$<CONFIG>/${module_subdir}/${shared_library_filename}
    )
    add_custom_command(
        OUTPUT ${dest_object}
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${source_object}
            ${dest_object}
        DEPENDS ${source_object}
        VERBATIM
    )
    # Give this copying action a name
    add_custom_target(copy-${library_name} DEPENDS ${dest_object})
    set_target_properties(copy-${library_name} PROPERTIES FOLDER generated)

    # Put this into a library target
    add_library(${library_name} MODULE IMPORTED GLOBAL)
    add_dependencies(${library_name} copy-${library_name})
    set_property(
        TARGET ${library_name}
        PROPERTY IMPORTED_LOCATION ${dest_object}
    )
endfunction()

function(install_fetched_shared_library library_name url)
    copy_fetched_shared_library(${library_name} ${url} ${ARGN})
    set(shared_library_filename
        "${dep_shared_library_prefix}${library_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )
    set(dest_object
        ${CMAKE_BINARY_DIR}/$<CONFIG>/${module_subdir}/${shared_library_filename}
    )
    if(TARGET ${library_name})
        install(PROGRAMS ${dest_object} DESTINATION ${module_subdir})
    endif()
endfunction()
