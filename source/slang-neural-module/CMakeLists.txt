#
# Build neural.slang shaders into a slang module and copy to build output
#

#
# Neural module configuration
# SLANG_NEURAL_MODULE_DIR_NAME is the output directory name for all the prebuilt modules, currently there is only neural module
# SLANG_NEURAL_MODULE_FILE_NAME is the file name for the neural module
set(SLANG_NEURAL_MODULE_DIR_NAME "slang-prebuilt-module-${SLANG_VERSION_FULL}" CACHE STRING "Directory name for neural module")
set(SLANG_NEURAL_MODULE_FILE_NAME "neural.slang-module" CACHE STRING "File name for neural module")

# Determine the library directory based on platform
# On Windows: shared libraries (.dll) go to bin/
# On Linux/Mac: shared libraries (.so/.dylib) go to lib/
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(slang_library_dir "bin")
else()
    set(slang_library_dir "lib")
endif()

# Set install directory using the platform-specific library directory
set(SLANG_NEURAL_MODULE_INSTALL_DIR "${slang_library_dir}/${SLANG_NEURAL_MODULE_DIR_NAME}" CACHE STRING "Installation directory for neural module")

#
# Generate the neural module configuration header, this is used by the slang compiler to find the neural module
#

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/slang-neural-config.h.in
    slang-neural-config-header/slang-neural-config.h
)

# Create a target to ensure the neural config header is generated
add_custom_target(
    generate_neural_config_header
    DEPENDS
        "${CMAKE_CURRENT_BINARY_DIR}/slang-neural-config-header/slang-neural-config.h"
)
set_target_properties(generate_neural_config_header PROPERTIES FOLDER "generated")

glob_append(neural_shader_sources "*.slang")

# Set output directories for build tree
set(neural_output_dir "${CMAKE_BINARY_DIR}/$<CONFIG>/${slang_library_dir}/${SLANG_NEURAL_MODULE_DIR_NAME}")
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

# Build neural.slang-module and output to the output directory
add_custom_command(
    OUTPUT ${neural_module_file}
    COMMAND slangc neural.slang -o ${SLANG_NEURAL_MODULE_FILE_NAME}
    DEPENDS ${neural_copied_files} slangc
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
    DESTINATION ${SLANG_NEURAL_MODULE_INSTALL_DIR}
    FILES_MATCHING
    PATTERN "*.slang"
    PATTERN "*.slang-module"
)
