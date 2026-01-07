#
# Build neural.slang shaders into a slang module and copy to build output
#

glob_append(neural_shader_sources "*.slang")

# Set output directories for build tree
set(neural_output_dir "${CMAKE_BINARY_DIR}/$<CONFIG>/${slang_library_dir}/${SLANG_STANDARD_MODULE_DIR_NAME}")
set(neural_module_file "${neural_output_dir}/${SLANG_NEURAL_MODULE_FILE_NAME}")

# Copy neural shader files to output directory
set(neural_copied_files)
foreach(shader_file ${neural_shader_sources})
    get_filename_component(shader_name ${shader_file} NAME)
    set(dest_file "${neural_output_dir}/${shader_name}")
    add_custom_command(
        OUTPUT ${dest_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${neural_output_dir}
        COMMAND ${CMAKE_COMMAND} -E copy ${shader_file} ${dest_file}
        DEPENDS ${shader_file}
        VERBATIM
    )
    list(APPEND neural_copied_files ${dest_file})
endforeach()

# Determine which compiler to use for building the module
# For cross-compilation, use slang-bootstrap from SLANG_GENERATORS_PATH
# (slang-bootstrap is a standalone tool with no external dependencies)
# Otherwise, use the slangc built as part of this build
if(SLANG_GENERATORS_PATH)
    if(CMAKE_HOST_WIN32)
        set(CMAKE_HOST_EXECUTABLE_SUFFIX ".exe")
    else()
        set(CMAKE_HOST_EXECUTABLE_SUFFIX "")
    endif()
    set(SLANG_COMPILER "${SLANG_GENERATORS_PATH}/slang-bootstrap${CMAKE_HOST_EXECUTABLE_SUFFIX}")
    set(SLANG_COMPILER_DEPENDENCY)
elseif(SLANG_ENABLE_SLANGC)
    set(SLANG_COMPILER slangc)
    set(SLANG_COMPILER_DEPENDENCY slangc)
else()
    # As a fallback, use the system-installed `slangc` compiler
    set(SLANG_COMPILER slangc)
    set(SLANG_COMPILER_DEPENDENCY)
endif()

# Build neural.slang-module and output to the output directory
add_custom_command(
    OUTPUT ${neural_module_file}
    COMMAND ${SLANG_COMPILER} neural.slang -o ${SLANG_NEURAL_MODULE_FILE_NAME}
    DEPENDS ${neural_copied_files} ${SLANG_COMPILER_DEPENDENCY}
    WORKING_DIRECTORY ${neural_output_dir}
    VERBATIM
)

# Create a target to ensure the neural module is built
add_custom_target(
    slang-neural-module
    ALL
    DEPENDS ${neural_module_file}
)
set_target_properties(slang-neural-module PROPERTIES FOLDER generated)

# Install the neural directory to the configured location in the release package
install(
    DIRECTORY ${neural_output_dir}/
    DESTINATION ${SLANG_STANDARD_MODULE_INSTALL_DIR}
    FILES_MATCHING
    PATTERN "*.slang"
    PATTERN "*.slang-module"
)
