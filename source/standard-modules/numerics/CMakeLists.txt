#
# Build the experimental numeric interface modules and place them in the standard-module tree.
#

set(numerics_standard_module_root
    "${slang_BINARY_DIR}/$<CONFIG>/${slang_library_dir}/${SLANG_STANDARD_MODULE_DIR_NAME}"
)
set(numerics_output_dir "${numerics_standard_module_root}/slang")
set(numerics_private_output_dir "${numerics_output_dir}/numerics")

set(numerics_builtin_module_file
    "${numerics_private_output_dir}/__builtin.slang-module"
)
set(numerics_module_file "${numerics_output_dir}/numerics.slang-module")
set(numerics_differentiable_builtin_module_file
    "${numerics_private_output_dir}/__builtin_differentiable.slang-module"
)
set(numerics_differentiable_module_file
    "${numerics_private_output_dir}/differentiable.slang-module"
)

set(numerics_base_sources
    numerics.slang
    interfaces.slang
    builtin-conformances.slang
    floating-point-conformances.slang
    cooperative-vector-conformances.slang
    wrappers.slang
)
set(numerics_differentiable_sources
    differentiable.slang
    differentiable-interfaces.slang
    differentiable-exp10.slang
    differentiable-conformances.slang
    differentiable-wrappers.slang
)
set(numerics_adapter_sources
    __builtin.slang
    __builtin-differentiable.slang
)

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

set(SLANG_COMPILER_CORE_MODULE_ARGS -load-core-module ${core_module_archive_without_timestamp})
set(SLANG_COMPILER_EXTRA_DEPS generate_core_module)

# These adapter modules deliberately do not import the public numerics modules. Their lookup
# environments therefore cannot contain the public interface-generic overloads, so their calls
# resolve to established core-module operations instead of recursively selecting a witness-backed
# overload. The public modules import these adapters, which makes the serialized adapter artifacts
# ordinary build dependencies of the public artifacts.
add_custom_command(
    OUTPUT ${numerics_builtin_module_file}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${numerics_private_output_dir}
    COMMAND
        ${SLANG_COMPILER} ${SLANG_COMPILER_CORE_MODULE_ARGS} -experimental-feature
        ${CMAKE_CURRENT_SOURCE_DIR}/__builtin.slang -o ${numerics_builtin_module_file}
    DEPENDS
        __builtin.slang
        ${SLANG_COMPILER_DEPENDENCY}
        ${SLANG_COMPILER_EXTRA_DEPS}
    VERBATIM
)

add_custom_command(
    OUTPUT ${numerics_differentiable_builtin_module_file}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${numerics_private_output_dir}
    COMMAND
        ${SLANG_COMPILER} ${SLANG_COMPILER_CORE_MODULE_ARGS} -experimental-feature
        ${CMAKE_CURRENT_SOURCE_DIR}/__builtin-differentiable.slang
        -o ${numerics_differentiable_builtin_module_file}
    DEPENDS
        __builtin-differentiable.slang
        ${SLANG_COMPILER_DEPENDENCY}
        ${SLANG_COMPILER_EXTRA_DEPS}
    VERBATIM
)

# A bootstrap compiler that was given the generated core archive on its command line cannot load a
# second freshly serialized module that records a dependency on that core module. Stage the adapter
# and base sources under their import paths so that dependent numeric modules can compile in the
# same build. Installed clients still import the serialized modules produced above and below.
set(numerics_source_stage_root "${CMAKE_CURRENT_BINARY_DIR}/source")
set(numerics_staged_base_source "${numerics_source_stage_root}/slang/numerics.slang")
set(numerics_staged_interface_source "${numerics_source_stage_root}/slang/interfaces.slang")
set(numerics_staged_builtin_conformances_source
    "${numerics_source_stage_root}/slang/builtin-conformances.slang"
)
set(numerics_staged_floating_point_conformances_source
    "${numerics_source_stage_root}/slang/floating-point-conformances.slang"
)
set(numerics_staged_cooperative_vector_conformances_source
    "${numerics_source_stage_root}/slang/cooperative-vector-conformances.slang"
)
set(numerics_staged_wrappers_source "${numerics_source_stage_root}/slang/wrappers.slang")
set(numerics_staged_builtin_source
    "${numerics_source_stage_root}/slang/numerics/__builtin.slang"
)
set(numerics_staged_differentiable_builtin_source
    "${numerics_source_stage_root}/slang/numerics/__builtin_differentiable.slang"
)
set(numerics_staged_sources
    ${numerics_staged_base_source}
    ${numerics_staged_interface_source}
    ${numerics_staged_builtin_conformances_source}
    ${numerics_staged_floating_point_conformances_source}
    ${numerics_staged_cooperative_vector_conformances_source}
    ${numerics_staged_wrappers_source}
    ${numerics_staged_builtin_source}
    ${numerics_staged_differentiable_builtin_source}
)

add_custom_command(
    OUTPUT ${numerics_staged_sources}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${numerics_source_stage_root}/slang/numerics
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different numerics.slang ${numerics_staged_base_source}
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different interfaces.slang
        ${numerics_staged_interface_source}
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different builtin-conformances.slang
        ${numerics_staged_builtin_conformances_source}
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different floating-point-conformances.slang
        ${numerics_staged_floating_point_conformances_source}
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different cooperative-vector-conformances.slang
        ${numerics_staged_cooperative_vector_conformances_source}
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different wrappers.slang
        ${numerics_staged_wrappers_source}
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different __builtin.slang
        ${numerics_staged_builtin_source}
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different __builtin-differentiable.slang
        ${numerics_staged_differentiable_builtin_source}
    DEPENDS ${numerics_base_sources} ${numerics_adapter_sources}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
)

add_custom_command(
    OUTPUT ${numerics_module_file}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${numerics_output_dir}
    COMMAND
        ${SLANG_COMPILER} ${SLANG_COMPILER_CORE_MODULE_ARGS}
        -I ${numerics_source_stage_root} -experimental-feature
        ${CMAKE_CURRENT_SOURCE_DIR}/numerics.slang -o ${numerics_module_file}
    DEPENDS
        ${numerics_base_sources}
        ${numerics_staged_sources}
        ${numerics_builtin_module_file}
        ${SLANG_COMPILER_DEPENDENCY}
        ${SLANG_COMPILER_EXTRA_DEPS}
    VERBATIM
)

add_custom_command(
    OUTPUT ${numerics_differentiable_module_file}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${numerics_private_output_dir}
    COMMAND
        ${SLANG_COMPILER} ${SLANG_COMPILER_CORE_MODULE_ARGS}
        -I ${numerics_source_stage_root} -experimental-feature
        ${CMAKE_CURRENT_SOURCE_DIR}/differentiable.slang
        -o ${numerics_differentiable_module_file}
    DEPENDS
        ${numerics_differentiable_sources}
        ${numerics_staged_sources}
        ${numerics_module_file}
        ${numerics_differentiable_builtin_module_file}
        ${SLANG_COMPILER_DEPENDENCY}
        ${SLANG_COMPILER_EXTRA_DEPS}
    VERBATIM
)

add_custom_target(
    slang-numerics-modules ALL
    DEPENDS
        ${numerics_builtin_module_file}
        ${numerics_module_file}
        ${numerics_differentiable_builtin_module_file}
        ${numerics_differentiable_module_file}
)
set_target_properties(slang-numerics-modules PROPERTIES FOLDER generated)

install(
    FILES ${numerics_module_file}
    DESTINATION ${SLANG_STANDARD_MODULE_INSTALL_DIR}/slang
)
install(
    FILES
        ${numerics_builtin_module_file}
        ${numerics_differentiable_builtin_module_file}
        ${numerics_differentiable_module_file}
    DESTINATION ${SLANG_STANDARD_MODULE_INSTALL_DIR}/slang/numerics
)
