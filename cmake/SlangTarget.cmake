function(slang_add_target dir type)
    set(no_value_args
        # Don't include in the 'all' target
        EXCLUDE_FROM_ALL
        # This is a tool used for compile-time generation
        GENERATOR_TOOL
        # This is a tool used for testing
        TEST_TOOL
        # This is loaded at runtime as a shared library
        SHARED_LIBRARY_TOOL
        # -Wextra
        USE_EXTRA_WARNINGS
        # don't set -Wall
        USE_FEWER_WARNINGS
    )
    set(single_value_args
        # Set the target name, useful for multiple targets from the same
        # directory.
        # By default this is the name of the last directory component given
        TARGET_NAME
        # Set the output name, otherwise defaults to the target name
        OUTPUT_NAME
        # If this is a shared library then the ${EXPORT_MACRO_PREFIX}_DYNAMIC and
        # ${EXPORT_MACRO_PREFIX}_DYNAMIC_EXPORT macros are set for using and
        # building respectively
        EXPORT_MACRO_PREFIX
    )
    set(multi_value_args
        # Additional directories from which to glob source
        EXTRA_SOURCE_DIRS
        # Targets with which to link privately
        LINK_WITH_PRIVATE
        # Targets whose headers we use, but don't link with
        INCLUDE_FROM_PRIVATE
        # Any shared libraries used at runtime, this adds a build dependency and
        # sets the correct RPATH
        MODULE_DEPENDS
        # Any include directories other targets need to use this target
        INCLUDE_DIRECTORIES_PUBLIC
    )
    cmake_parse_arguments(
        ARG
        "${no_value_args}"
        "${single_value_args}"
        "${multi_value_args}"
        ${ARGV}
    )

    #
    # Set up some variables, including the target name
    #
    get_filename_component(dir_absolute ${dir} ABSOLUTE)
    if(DEFINED ARG_TARGET_NAME)
        set(target ${ARG_TARGET_NAME})
    else()
        get_filename_component(target ${dir_absolute} NAME)
    endif()

    #
    # Find the source for this target
    #
    slang_glob_sources(source "${dir}/*.cpp")
    foreach(extra_dir ${ARG_EXTRA_SOURCE_DIRS})
        slang_glob_sources(source "${extra_dir}/*.cpp")
    endforeach()

    #
    # Create the target
    #
    set(library_types
        STATIC
        SHARED
        OBJECT
        MODULE
        ALIAS
    )
    if(type STREQUAL "EXECUTABLE")
        add_executable(${target} ${source})
    elseif(type STREQUAL "LIBRARY")
        add_library(${target} ${source})
    elseif(type IN_LIST library_types)
        add_library(${target} ${type} ${source})
    else()
        message(
            SEND_ERROR
            "Unsupported target type ${type} in slang_add_target"
        )
        return()
    endif()

    #
    # Set the output directory
    #
    # We don't want the output directory to be sensitive to where slang_add_target
    # is called from, so set it explicitly here.
    #
    if(ARG_GENERATOR_TOOL)
        # generators go in generators/
        set(output_dir "generators")
    elseif(ARG_TEST_TOOL)
        # tests go in test/
        set(output_dir "test")
    else()
        # Everything else is placed according to its source
        file(
            RELATIVE_PATH
            dir_source_relative
            ${CMAKE_SOURCE_DIR}
            ${dir_absolute}
        )
        set(output_dir "${dir_source_relative}")
    endif()

    if(DEFINED output_dir)
        set(output_dir "${CMAKE_BINARY_DIR}/${output_dir}/$<CONFIG>")
        set_target_properties(
            ${target}
            PROPERTIES
                ARCHIVE_OUTPUT_DIRECTORY ${output_dir}
                LIBRARY_OUTPUT_DIRECTORY ${output_dir}
                RUNTIME_OUTPUT_DIRECTORY ${output_dir}
        )
    endif()

    #
    # Set common compile options and properties
    #
    if(ARG_USE_EXTRA_WARNINGS)
        set_default_compile_options(${target} USE_EXTRA_WARNINGS)
    elseif(ARG_USE_FEWER_WARNINGS)
        set_default_compile_options(${target} USE_FEWER_WARNINGS)
    else()
        set_default_compile_options(${target})
    endif()

    set_target_properties(
        ${target}
        PROPERTIES EXCLUDE_FROM_ALL ${ARG_EXCLUDE_FROM_ALL}
    )

    if(DEFINED ARG_OUTPUT_NAME)
        set_target_properties(
            ${target}
            PROPERTIES OUTPUT_NAME ${ARG_OUTPUT_NAME}
        )
    endif()

    #
    # Link and include from dependencies
    #
    target_link_libraries(${target} PRIVATE ${ARG_LINK_WITH_PRIVATE})

    foreach(include_only_target ${ARG_INCLUDE_FROM_PRIVATE})
        target_include_directories(
            ${target}
            PRIVATE
                $<TARGET_PROPERTY:${include_only_target},INTERFACE_INCLUDE_DIRECTORIES>
        )
    endforeach()

    #
    # Set our exported include directories
    #
    foreach(inc ${ARG_INCLUDE_DIRECTORIES_PUBLIC})
        get_filename_component(inc_abs ${inc} ABSOLUTE)
        target_include_directories(
            ${target}
            PUBLIC "$<BUILD_INTERFACE:${inc_abs}>"
        )
    endforeach()

    #
    # Runtime dependencies on other targets
    #
    if(DEFINED ARG_MODULE_DEPENDS)
        add_dependencies(${target} ${ARG_MODULE_DEPENDS})
        list(
            TRANSFORM ARG_MODULE_DEPENDS
            REPLACE "(.+)" "$<TARGET_FILE_DIR:\\1>"
            OUTPUT_VARIABLE build_rpaths
        )
        set_property(
            TARGET ${target}
            APPEND
            PROPERTY BUILD_RPATH ${build_rpaths}
        )
        get_target_property(x ${target} BUILD_RPATH)
    endif()

    #
    # Set up export macros
    #
    if(ARG_SHARED_LIBRARY_TOOL)
        target_compile_definitions(${target} PRIVATE SLANG_SHARED_LIBRARY_TOOL)
    endif()

    get_target_property(target_type ${target} TYPE)
    if(DEFINED ARG_EXPORT_MACRO_PREFIX)
        if(
            target_type STREQUAL SHARED_LIBRARY
            OR target_type STREQUAL MODULE_LIBRARY
        )
            target_compile_definitions(
                ${target}
                PUBLIC "${ARG_EXPORT_MACRO_PREFIX}_DYNAMIC"
                PRIVATE "${ARG_EXPORT_MACRO_PREFIX}_DYNAMIC_EXPORT"
            )
        endif()
    endif()
endfunction()
