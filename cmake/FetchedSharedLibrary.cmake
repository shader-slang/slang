# Add rule to install shared library of name 'library_name' in the 'module_subdir' directory.
# If 'url' is a directory, the shared library (with platform-specific shared library prefixes and suffixes) will be
# taken from the directory, and whatever is found there will be used to produce the install rule.
# If the 'url' is a path to a file with the platform-specific shared library prefix and suffix, then that file
# will be used to produce the install rule.
# Otherwise, the 'url' is interpreted as an URL, and the content of the URL will be fetched, extracted and searched
# for the shared library to produce the install rule.
function(install_fetched_shared_library library_name url)
    set(shared_library_filename
        "${CMAKE_SHARED_LIBRARY_PREFIX}${library_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )
    macro(from_glob dir)
        # A little helper function
        file(
            GLOB_RECURSE source_object
            "${dir}/${shared_library_filename}"
        )
        list(LENGTH source_object nmatches)
        if(nmatches EQUAL 0)
            message(
                SEND_ERROR
                "Unable to find ${shared_library_filename} in ${url}"
            )
        elseif(nmatches GREATER 1)
            message(
                SEND_ERROR
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
            "${CMAKE_SHARED_LIBRARY_PREFIX}.+${CMAKE_SHARED_LIBRARY_SUFFIX}$"
        AND EXISTS "${url}"
    )
        # Otherwise, if it's a direct path to a shared object, use that
        set(source_object "${url}")
    else()
        # Otherwise, download and extract from whatever URL we have
        fetchcontent_declare(${library_name} URL "${url}")
        fetchcontent_populate(${library_name})
        from_glob(${${library_name}_SOURCE_DIR})
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
    install(PROGRAMS ${dest_object} DESTINATION ${module_subdir})
endfunction()
