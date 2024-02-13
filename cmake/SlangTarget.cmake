#
# A function to make target creation a little more declarative
#
# See the comments on the options below for usage
#
function(slang_add_target dir type)
    set(no_value_args
        # Don't include in the 'all' target
        EXCLUDE_FROM_ALL
        # This is loaded at runtime as a shared library
        SHARED_LIBRARY_TOOL
        # -Wextra
        USE_EXTRA_WARNINGS
        # don't set -Wall
        USE_FEWER_WARNINGS
        # Make this a Windows app, rather than a console app, only makes a
        # difference when compiling for Windows
        WIN32_EXECUTABLE
        # Install this target for a non-component install
        INSTALL
    )
    set(single_value_args
        # Set the target name, useful for multiple targets from the same
        # directory.
        # By default this is the name of the last directory component given
        TARGET_NAME
        # Set the output name, otherwise defaults to the target name
        OUTPUT_NAME
        # Set an explicit output directory relative to the cmake binary
        # directory. otherwise defaults to the binary directory root.
        # Outputs are always placed in a further subdirectory according to
        # build config
        OUTPUT_DIR
        # If this is a shared library then the ${EXPORT_MACRO_PREFIX}_DYNAMIC and
        # ${EXPORT_MACRO_PREFIX}_DYNAMIC_EXPORT macros are set for using and
        # building respectively
        EXPORT_MACRO_PREFIX
        # The folder in which to place this target for IDE-based generators (VS
        # and XCode)
        FOLDER
        # The working directory for debugging
        DEBUG_DIR
        # Install this target as part of a component
        INSTALL_COMPONENT
    )
    set(multi_value_args
        # Use exactly these sources, instead of globbing from the directory
        # argument
        EXPLICIT_SOURCE
        # Additional directories from which to glob source
        EXTRA_SOURCE_DIRS
        # Additional compile definitions
        EXTRA_COMPILE_DEFINITIONS_PRIVATE
        EXTRA_COMPILE_DEFINITIONS_PUBLIC
        # Targets with which to link privately
        LINK_WITH_PRIVATE
        # Targets whose headers we use, but don't link with
        INCLUDE_FROM_PRIVATE
        # Any include directories other targets need to use this target
        INCLUDE_DIRECTORIES_PUBLIC
        # Add a dependency on the new target to the specified targets
        REQUIRED_BY
        # Add a dependency to the new target on the specified targets
        REQUIRES
        # Globs for any headers to install
        PUBLIC_HEADERS
    )
    cmake_parse_arguments(
        ARG
        "${no_value_args}"
        "${single_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )

    if(DEFINED ARG_UNPARSED_ARGUMENTS OR DEFINED ARG_KEYWORDS_MISSING_VALUES)
        foreach(unparsed_arg ${ARG_UNPARSED_ARGUMENTS})
            message(
                SEND_ERROR
                "Unrecognized argument in slang_add_target: ${unparsed_arg}"
            )
        endforeach()
        foreach(bad_kwarg ${ARG_KEYWORDS_MISSING_VALUES})
            message(
                SEND_ERROR
                "Keyword argument missing values in slang_add_target: ${bad_kwarg}"
            )
        endforeach()
        return()
    endif()

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
    if(ARG_EXPLICIT_SOURCE)
        list(APPEND source ${ARG_EXPLICIT_SOURCE})
    else()
        slang_glob_sources(source ${dir})
    endif()
    foreach(extra_dir ${ARG_EXTRA_SOURCE_DIRS})
        slang_glob_sources(source ${extra_dir})
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
    # We don't want the output directory to be sensitive to where
    # slang_add_target is called from, so set it explicitly here.
    #
    if(DEFINED ARG_OUTPUT_DIR)
        set(output_dir "${CMAKE_BINARY_DIR}/${ARG_OUTPUT_DIR}/$<CONFIG>")
    else()
        # Default to placing things in the cmake binary root.
        #
        # While it would be nice to place things according to their
        # subdirectory, Windows' inflexibility in being able to find DLLs makes
        # this tricky there.
        set(output_dir "${CMAKE_BINARY_DIR}/$<CONFIG>")
    endif()
    set(archive_subdir ${library_subdir})
    if(type STREQUAL "MODULE")
        set(library_subdir ${module_subdir})
    endif()
    set_target_properties(
        ${target}
        PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${output_dir}/${archive_subdir}"
            LIBRARY_OUTPUT_DIRECTORY "${output_dir}/${library_subdir}"
            RUNTIME_OUTPUT_DIRECTORY "${output_dir}/${runtime_subdir}"
            PDB_OUTPUT_DIRECTORY "${output_dir}/${runtime_subdir}"
    )

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

    set_target_properties(
        ${target}
        PROPERTIES WIN32_EXECUTABLE ${ARG_WIN32_EXECUTABLE}
    )

    if(DEFINED ARG_OUTPUT_NAME)
        set_target_properties(
            ${target}
            PROPERTIES OUTPUT_NAME ${ARG_OUTPUT_NAME}
        )
    endif()

    if(DEFINED ARG_FOLDER)
        set_target_properties(${target} PROPERTIES FOLDER ${ARG_FOLDER})
    endif()

    if(DEFINED ARG_DEBUG_DIR)
        set_target_properties(
            ${target}
            PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${ARG_DEBUG_DIR}
        )
    endif()

    #
    # Link and include from dependencies
    #
    target_link_libraries(${target} PRIVATE ${ARG_LINK_WITH_PRIVATE})

    foreach(include_from ${ARG_INCLUDE_FROM_PRIVATE})
        target_include_directories(
            ${target}
            PRIVATE
                $<TARGET_PROPERTY:${include_from},INTERFACE_INCLUDE_DIRECTORIES>
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
    # Set up export macros
    #
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
        elseif(
            target_type STREQUAL STATIC_LIBRARY
        )
            target_compile_definitions(
                ${target}
                PUBLIC "${ARG_EXPORT_MACRO_PREFIX}_STATIC"
            )
        endif()
    endif()

    #
    # Other dependencies
    #
    foreach(requirer ${ARG_REQUIRED_BY})
        add_dependencies(${requirer} ${target})
    endforeach()

    if(DEFINED ARG_REQUIRES)
        add_dependencies(${target} ${ARG_REQUIRES})
    endif()

    #
    # Other preprocessor defines
    #
    if(ARG_EXTRA_COMPILE_DEFINITIONS_PRIVATE)
        target_compile_definitions(
            ${target}
            PRIVATE ${ARG_EXTRA_COMPILE_DEFINITIONS_PRIVATE}
        )
    endif()
    if(ARG_EXTRA_COMPILE_DEFINITIONS_PUBLIC)
        target_compile_definitions(
            ${target}
            PUBLIC ${ARG_EXTRA_COMPILE_DEFINITIONS_PUBLIC}
        )
    endif()

    #
    # Since we do a lot of dynamic loading, unconditionally set the build rpath
    # to find our libraries. Ordinarily CMake would sort this out, but we do
    # have libraries which at build time don't depend on any other shared
    # libraries of ours but which do load them at runtime, hence the need to do
    # this explicitly here.
    #
    if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
        set(ORIGIN "@loader_path")
    else()
        set(ORIGIN "$ORIGIN")
    endif()
    set_property(
        TARGET ${target}
        APPEND
        PROPERTY BUILD_RPATH "${ORIGIN}/../${library_subdir}"
    )
    set_property(
        TARGET ${target}
        APPEND
        PROPERTY INSTALL_RPATH "${ORIGIN}/../${library_subdir}"
    )

    # On the same topic, give everything a dylib suffix on Mac OS
    if(CMAKE_SYSTEM_NAME MATCHES "Darwin" AND type STREQUAL "MODULE")
        set_property(TARGET ${target} PROPERTY SUFFIX ".dylib")
    endif()

    #
    # Mark headers for installation
    #
    if(ARG_PUBLIC_HEADERS)
        if(NOT ARG_INSTALL)
            message(
                WARNING
                "${target} was declared with PUBLIC_HEADERS but without INSTALL, the former will do nothing"
            )
        endif()

        glob_append(public_headers ${ARG_PUBLIC_HEADERS})
        if(NOT public_headers)
            message(WARNING "${target}'s PUBLIC_HEADER globs found no matches")
        else()
            set_target_properties(
                ${target}
                PROPERTIES PUBLIC_HEADER "${public_headers}"
            )
        endif()
    endif()

    #
    # Mark for installation
    #
    if(ARG_INSTALL OR ARG_INSTALL_COMPONENT)
        set(component_args)
        if(ARG_INSTALL_COMPONENT)
            set(component_args COMPONENT ${ARG_INSTALL_COMPONENT})
        endif()
        set(exclude_arg)
        if(NOT ARG_INSTALL)
            set(exclude_arg EXCLUDE_FROM_ALL)
        endif()
        install(
            TARGETS ${target}
            EXPORT SlangTargets
            ARCHIVE
            DESTINATION ${archive_subdir}
            ${component_args}
            ${exclude_arg}
            LIBRARY
            DESTINATION ${library_subdir}
            ${component_args}
            ${exclude_arg}
            RUNTIME
            DESTINATION ${runtime_subdir}
            ${component_args}
            ${exclude_arg}
            PUBLIC_HEADER
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            ${component_args}
            ${exclude_arg}
        )
    endif()
endfunction()

# Ideally we'd use CMAKE_INSTALL_LIBDIR and CMAKE_INSTALL_RUNTIMEDIR here,
# however some Slang functionality (specifically generating executables on
# Linux systems) relies on runtime libraries being at "$ORIGIN/../lib". This
# could be improved by setting at configure-time that path to be the relative
# path from CM_I_RD to CM_I_LD.
set(library_subdir lib)
set(runtime_subdir bin)

# On Windows, because there's no RPATH, place modules in bin, next to the
# executables which load them (by deault, CMAKE will place them in lib and
# expect the application to seek them out there)
#
# This variable is used in the above function as and elsewhere for installing
# an imported module (slang-llvm from binary), hence why it's defined here.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(module_subdir ${runtime_subdir})
else()
    set(module_subdir ${library_subdir})
endif()
