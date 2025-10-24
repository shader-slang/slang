#
# setup the standard module configuration
#

#
# Neural module configuration
# SLANG_STANDARD_MODULE_DIR_NAME is the output directory name for all the standard modules, currently there is only neural module
set(SLANG_STANDARD_MODULE_DIR_NAME "slang-standard-module-${SLANG_VERSION_NUMERIC}" CACHE STRING "Directory name for standard module")

# SLANG_NEURAL_MODULE_FILE_NAME is the file name for the neural module
set(SLANG_NEURAL_MODULE_FILE_NAME "neural.slang-module" CACHE STRING "module file name for neural module")

# Determine the library directory based on platform
# On Windows: shared libraries (.dll) go to bin/
# On Linux/Mac: shared libraries (.so/.dylib) go to lib/
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(slang_library_dir "bin")
else()
    set(slang_library_dir "lib")
endif()

# Set install directory using the platform-specific library directory
set(SLANG_STANDARD_MODULE_INSTALL_DIR "${slang_library_dir}/${SLANG_STANDARD_MODULE_DIR_NAME}" CACHE STRING "Installation directory for neural module")

#
# Generate the standard module configuration header, this is used by the slang compiler to find the standard module
#

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/slang-standard-module-config.h.in
    slang-standard-module-config-header/slang-standard-module-config.h
)

# Create a target to ensure the standard module config header is generated
add_custom_target(
    generate_standard_module_config_header
    DEPENDS
        "${CMAKE_CURRENT_BINARY_DIR}/slang-standard-module-config-header/slang-standard-module-config.h"
)
set_target_properties(generate_standard_module_config_header PROPERTIES FOLDER "generated")

# Build the neural module
add_subdirectory(neural)
