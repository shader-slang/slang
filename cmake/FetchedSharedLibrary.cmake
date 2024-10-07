function(install_fetched_shared_llvm_library)
    #
    # Do some stupid little dance to put everything in the right shape with
    # correct dependencies
    #

    set(slang_llvm_filename
        "${CMAKE_SHARED_LIBRARY_PREFIX}slang-llvm${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )
    macro(from_glob dir)
        # A little helper function
        file(
            GLOB_RECURSE slang_llvm_source_object
            "${dir}/${slang_llvm_filename}"
        )
        list(LENGTH slang_llvm_source_object nmatches)
        if(nmatches EQUAL 0)
            message(
                SEND_ERROR
                "Unable to find ${slang_llvm_filename} in ${SLANG_SLANG_LLVM_BINARY_URL}"
            )
        elseif(nmatches GREATER 1)
            message(
                SEND_ERROR
                "Found multiple files named ${slang_llvm_filename} in ${SLANG_SLANG_LLVM_BINARY_URL}"
            )
        endif()
    endmacro()

    if(IS_DIRECTORY "${SLANG_SLANG_LLVM_BINARY_URL}")
        # Just glob directly from a local directory
        from_glob("${SLANG_SLANG_LLVM_BINARY_URL}")
    elseif(
        SLANG_SLANG_LLVM_BINARY_URL
            MATCHES
            "${CMAKE_SHARED_LIBRARY_PREFIX}.+${CMAKE_SHARED_LIBRARY_SUFFIX}$"
        AND EXISTS "${SLANG_SLANG_LLVM_BINARY_URL}"
    )
        # Otherwise, if it's a direct path to a shared object, use that
        set(slang_llvm_source_object "${SLANG_SLANG_LLVM_BINARY_URL}")
    else()
        # Otherwise, download and extract from whatever URL we have
        fetchcontent_declare(slang-llvm URL "${SLANG_SLANG_LLVM_BINARY_URL}")
        fetchcontent_populate(slang-llvm)
        from_glob("${slang-llvm_SOURCE_DIR}")
    endif()

    set(slang_llvm_dest_object
        ${CMAKE_BINARY_DIR}/$<CONFIG>/${module_subdir}/${slang_llvm_filename}
    )
    add_custom_command(
        OUTPUT ${slang_llvm_dest_object}
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${slang_llvm_source_object}
            ${slang_llvm_dest_object}
        DEPENDS ${slang_llvm_source_object}
        VERBATIM
    )
    # Give this copying action a name
    add_custom_target(copy-slang-llvm DEPENDS ${slang_llvm_dest_object})
    set_target_properties(copy-slang-llvm PROPERTIES FOLDER generated)

    # Put this into a library target
    add_library(slang-llvm MODULE IMPORTED GLOBAL)
    add_dependencies(slang-llvm copy-slang-llvm)
    set_property(
        TARGET slang-llvm
        PROPERTY IMPORTED_LOCATION ${slang_llvm_dest_object}
    )
    install(PROGRAMS ${slang_llvm_dest_object} DESTINATION ${module_subdir})
endfunction()
