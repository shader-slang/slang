#
# Build experimental standard modules and copy them to the build output.
#

glob_append(workgraph_shader_sources "*.slang")

set(workgraph_output_dir
    "${CMAKE_BINARY_DIR}/$<CONFIG>/${slang_library_dir}/${SLANG_STANDARD_MODULE_DIR_NAME}/experimental"
)
set(workgraph_module_file "${workgraph_output_dir}/${SLANG_WORKGRAPH_MODULE_FILE_NAME}")

set(workgraph_copied_files)
foreach(shader_file ${workgraph_shader_sources})
    get_filename_component(shader_name ${shader_file} NAME)
    set(dest_file "${workgraph_output_dir}/${shader_name}")
    add_custom_command(
        OUTPUT ${dest_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${workgraph_output_dir}
        COMMAND ${CMAKE_COMMAND} -E copy ${shader_file} ${dest_file}
        DEPENDS ${shader_file}
        VERBATIM
    )
    list(APPEND workgraph_copied_files ${dest_file})
endforeach()

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
    set(SLANG_COMPILER_EXTRA_DEPS)
elseif(NOT SLANG_EMBED_CORE_MODULE)
    set(SLANG_COMPILER slang-bootstrap)
    set(SLANG_COMPILER_DEPENDENCY slang-bootstrap)
    set(SLANG_COMPILER_EXTRA_DEPS generate_core_module_headers)
elseif(SLANG_ENABLE_SLANGC)
    set(SLANG_COMPILER "$<TARGET_FILE:slangc>")
    set(SLANG_COMPILER_DEPENDENCY slangc slang)
    set(SLANG_COMPILER_EXTRA_DEPS)
else()
    set(SLANG_COMPILER slang-bootstrap)
    set(SLANG_COMPILER_DEPENDENCY slang-bootstrap)
    set(SLANG_COMPILER_EXTRA_DEPS generate_core_module_headers)
endif()

add_custom_command(
    OUTPUT ${workgraph_module_file}
    COMMAND
        ${SLANG_COMPILER} -I ${CMAKE_CURRENT_SOURCE_DIR}
        workgraph.slang -o ${SLANG_WORKGRAPH_MODULE_FILE_NAME}
    DEPENDS
        ${workgraph_copied_files}
        ${SLANG_COMPILER_DEPENDENCY}
        ${SLANG_COMPILER_EXTRA_DEPS}
    WORKING_DIRECTORY ${workgraph_output_dir}
    VERBATIM
)

add_custom_target(slang-workgraph-module ALL DEPENDS ${workgraph_module_file})
set_target_properties(slang-workgraph-module PROPERTIES FOLDER generated)

install(
    DIRECTORY ${workgraph_output_dir}/
    DESTINATION ${SLANG_STANDARD_MODULE_INSTALL_DIR}/experimental
    FILES_MATCHING
    PATTERN "*.slang"
    PATTERN "*.slang-module"
)
