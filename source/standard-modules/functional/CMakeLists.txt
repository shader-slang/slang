# Build the experimental functional module and place it in the standard-module tree.

set(functional_standard_module_root
    "${slang_BINARY_DIR}/$<CONFIG>/${slang_library_dir}/${SLANG_STANDARD_MODULE_DIR_NAME}"
)
set(functional_output_dir "${functional_standard_module_root}/slang")
set(functional_module_file "${functional_output_dir}/functional.slang-module")

if(SLANG_GENERATORS_PATH)
    if(CMAKE_HOST_WIN32)
        set(CMAKE_HOST_EXECUTABLE_SUFFIX ".exe")
    else()
        set(CMAKE_HOST_EXECUTABLE_SUFFIX "")
    endif()
    set(SLANG_COMPILER
        "${SLANG_GENERATORS_PATH}/slang-bootstrap${CMAKE_HOST_EXECUTABLE_SUFFIX}"
    )
    set(SLANG_COMPILER_DEPENDENCY)
else()
    set(SLANG_COMPILER slang-bootstrap)
    set(SLANG_COMPILER_DEPENDENCY slang-bootstrap)
endif()

set(SLANG_COMPILER_CORE_MODULE_ARGS
    -load-core-module
    ${core_module_archive_without_timestamp}
)
set(SLANG_COMPILER_EXTRA_DEPS generate_core_module)

add_custom_command(
    OUTPUT ${functional_module_file}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${functional_output_dir}
    COMMAND
        ${SLANG_COMPILER} ${SLANG_COMPILER_CORE_MODULE_ARGS}
        -experimental-feature ${CMAKE_CURRENT_SOURCE_DIR}/functional.slang -o
        ${functional_module_file}
    DEPENDS
        functional.slang
        ${SLANG_COMPILER_DEPENDENCY}
        ${SLANG_COMPILER_EXTRA_DEPS}
    VERBATIM
)

add_custom_target(slang-functional-module ALL DEPENDS ${functional_module_file})
set_target_properties(slang-functional-module PROPERTIES FOLDER generated)

# Record this module's serialized output in the shared manifest consumed by the
# optional slang-standard-modules monolithic library.
list(APPEND SLANG_STANDARD_MODULE_FILES ${functional_module_file})
set(SLANG_STANDARD_MODULE_FILES "${SLANG_STANDARD_MODULE_FILES}"
    CACHE INTERNAL "Built standard .slang-module files"
)
list(APPEND SLANG_STANDARD_MODULE_TARGETS slang-functional-module)
set(SLANG_STANDARD_MODULE_TARGETS "${SLANG_STANDARD_MODULE_TARGETS}"
    CACHE INTERNAL "Producer targets for standard .slang-module files"
)

if(NOT SLANG_BUILD_MONOLITHIC_STANDARD_MODULE_LIBRARY)
    install(
        FILES ${functional_module_file}
        DESTINATION ${SLANG_STANDARD_MODULE_INSTALL_DIR}/slang
    )
endif()
