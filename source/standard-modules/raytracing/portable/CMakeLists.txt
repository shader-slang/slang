# Build the experimental slang.raytracing.portable standard module.

set(portable_raytracing_shader_sources
    raytracing.slang
    compiler-intrinsics.slang
    ray-types.slang
    contexts.slang
    stage-inputs.slang
    stage-contracts.slang
    program-schema.slang
    descriptor.slang
    trace.slang
)

# Keep the source and serialized module under the `slang/raytracing/` import namespace used by clients.
set(portable_raytracing_output_dir
    "${slang_BINARY_DIR}/$<CONFIG>/${slang_library_dir}/${SLANG_STANDARD_MODULE_DIR_NAME}/slang/raytracing"
)

set(portable_raytracing_copied_files)
foreach(shader_file ${portable_raytracing_shader_sources})
    set(source_file "${CMAKE_CURRENT_SOURCE_DIR}/${shader_file}")
    set(dest_file "${portable_raytracing_output_dir}/${shader_file}")
    if(shader_file STREQUAL "raytracing.slang")
        # Source imports resolve slang.raytracing.portable through portable.slang.
        set(dest_file "${portable_raytracing_output_dir}/portable.slang")
    endif()
    add_custom_command(
        OUTPUT ${dest_file}
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${portable_raytracing_output_dir}
        COMMAND ${CMAKE_COMMAND} -E copy ${source_file} ${dest_file}
        DEPENDS ${source_file}
        VERBATIM
    )
    list(APPEND portable_raytracing_copied_files ${dest_file})
endforeach()

# Keep the generated enum with the ordinary module sources so source imports and installations
# use the same builtin IDs as serialized imports. generator() supplies a host executable when
# SLANG_GENERATORS_PATH is set for cross compilation.
set(portable_raytracing_builtin_source
    "${portable_raytracing_output_dir}/raytracing-builtins.slang"
)
add_custom_command(
    OUTPUT ${portable_raytracing_builtin_source}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${portable_raytracing_output_dir}
    COMMAND
        slang-standard-module-generator ${portable_raytracing_builtin_source}
    DEPENDS slang-standard-module-generator
    VERBATIM
)
list(
    APPEND
    portable_raytracing_copied_files
    ${portable_raytracing_builtin_source}
)

set(portable_raytracing_module_file
    "${portable_raytracing_output_dir}/portable.slang-module"
)

# Build with this tree's compiler and core module so the intrinsic types match. Cross builds use
# the supplied host generator. The source uses ordinary Slang module compilation.
if(SLANG_GENERATORS_PATH)
    if(CMAKE_HOST_WIN32)
        set(CMAKE_HOST_EXECUTABLE_SUFFIX ".exe")
    else()
        set(CMAKE_HOST_EXECUTABLE_SUFFIX "")
    endif()
    set(PORTABLE_RAYTRACING_SLANG_COMPILER
        "${SLANG_GENERATORS_PATH}/slang-bootstrap${CMAKE_HOST_EXECUTABLE_SUFFIX}"
    )
    set(PORTABLE_RAYTRACING_SLANG_COMPILER_DEPENDENCY)
elseif(SLANG_ENABLE_SLANGC)
    set(PORTABLE_RAYTRACING_SLANG_COMPILER "$<TARGET_FILE:slangc>")
    set(PORTABLE_RAYTRACING_SLANG_COMPILER_DEPENDENCY slangc slang)
else()
    set(PORTABLE_RAYTRACING_SLANG_COMPILER slang-bootstrap)
    set(PORTABLE_RAYTRACING_SLANG_COMPILER_DEPENDENCY slang-bootstrap)
endif()

add_custom_command(
    OUTPUT ${portable_raytracing_module_file}
    COMMAND
        ${PORTABLE_RAYTRACING_SLANG_COMPILER} -load-core-module
        ${core_module_archive_without_timestamp} -I ${CMAKE_CURRENT_SOURCE_DIR}
        portable.slang -o portable.slang-module
    DEPENDS
        ${portable_raytracing_copied_files}
        ${PORTABLE_RAYTRACING_SLANG_COMPILER_DEPENDENCY}
        generate_core_module
    WORKING_DIRECTORY ${portable_raytracing_output_dir}
    VERBATIM
)

add_custom_target(
    slang-portable-raytracing-module
    ALL
    DEPENDS ${portable_raytracing_module_file}
)
set_target_properties(
    slang-portable-raytracing-module
    PROPERTIES FOLDER generated
)

install(
    DIRECTORY ${portable_raytracing_output_dir}/
    DESTINATION ${SLANG_STANDARD_MODULE_INSTALL_DIR}/slang/raytracing
    FILES_MATCHING
    PATTERN "*.slang"
    PATTERN "*.slang-module"
)
