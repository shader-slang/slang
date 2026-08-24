# Build the experimental slang.raytracing standard module.

set(
    raytracing_shader_sources
    raytracing.slang
    ray-types.slang
    contexts.slang
    stage-inputs.slang
    stage-contracts.slang
    program-layout.slang
    descriptor.slang
    trace.slang
)

set(
    raytracing_output_dir
    "${CMAKE_BINARY_DIR}/$<CONFIG>/${slang_library_dir}/${SLANG_STANDARD_MODULE_DIR_NAME}/slang"
)

set(raytracing_copied_files)
foreach(shader_file ${raytracing_shader_sources})
    set(source_file "${CMAKE_CURRENT_SOURCE_DIR}/${shader_file}")
    set(dest_file "${raytracing_output_dir}/${shader_file}")
    add_custom_command(
        OUTPUT ${dest_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${raytracing_output_dir}
        COMMAND ${CMAKE_COMMAND} -E copy ${source_file} ${dest_file}
        DEPENDS ${source_file}
        VERBATIM
    )
    list(APPEND raytracing_copied_files ${dest_file})
endforeach()

set(
    raytracing_module_file
    "${raytracing_output_dir}/raytracing.slang-module"
)

if(SLANG_GENERATORS_PATH)
    if(CMAKE_HOST_WIN32)
        set(CMAKE_HOST_EXECUTABLE_SUFFIX ".exe")
    else()
        set(CMAKE_HOST_EXECUTABLE_SUFFIX "")
    endif()
    set(
        RAYTRACING_SLANG_COMPILER
        "${SLANG_GENERATORS_PATH}/slang-bootstrap${CMAKE_HOST_EXECUTABLE_SUFFIX}"
    )
    set(RAYTRACING_SLANG_COMPILER_DEPENDENCY)
elseif(SLANG_ENABLE_SLANGC)
    set(RAYTRACING_SLANG_COMPILER "$<TARGET_FILE:slangc>")
    set(RAYTRACING_SLANG_COMPILER_DEPENDENCY slangc slang)
else()
    set(RAYTRACING_SLANG_COMPILER slang-bootstrap)
    set(RAYTRACING_SLANG_COMPILER_DEPENDENCY slang-bootstrap)
endif()
set(
    RAYTRACING_SLANG_COMPILER_CORE_MODULE_ARGS
    -load-core-module
    ${core_module_archive_without_timestamp}
)

add_custom_command(
    OUTPUT ${raytracing_module_file}
    COMMAND
        ${RAYTRACING_SLANG_COMPILER} ${RAYTRACING_SLANG_COMPILER_CORE_MODULE_ARGS} -I
        ${CMAKE_CURRENT_SOURCE_DIR} raytracing.slang -o raytracing.slang-module
    DEPENDS
        ${raytracing_copied_files}
        ${RAYTRACING_SLANG_COMPILER_DEPENDENCY}
        generate_core_module
    WORKING_DIRECTORY ${raytracing_output_dir}
    VERBATIM
)

add_custom_target(slang-raytracing-module ALL DEPENDS ${raytracing_module_file})
set_target_properties(slang-raytracing-module PROPERTIES FOLDER generated)

install(
    DIRECTORY ${raytracing_output_dir}/
    DESTINATION ${SLANG_STANDARD_MODULE_INSTALL_DIR}/slang
    FILES_MATCHING
    PATTERN "*.slang"
    PATTERN "*.slang-module"
)
