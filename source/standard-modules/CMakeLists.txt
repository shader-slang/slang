#
# setup the standard module configuration
#

#
# Neural module configuration
# SLANG_STANDARD_MODULE_DIR_NAME is the output directory name for all the standard modules
set(SLANG_STANDARD_MODULE_DIR_NAME "slang-standard-module-${SLANG_VERSION_NUMERIC}" CACHE STRING "Directory name for standard module")

# SLANG_NEURAL_MODULE_FILE_NAME is the file name for the neural module
set(SLANG_NEURAL_MODULE_FILE_NAME "neural.slang-module" CACHE STRING "module file name for neural module")

# SLANG_WORKGRAPH_MODULE_FILE_NAME is the file name for the experimental workgraph module
set(SLANG_WORKGRAPH_MODULE_FILE_NAME "workgraph.slang-module" CACHE STRING "module file name for workgraph module")

if(NOT "${SLANG_DISABLE_NOTE_IDS}" STREQUAL "")
    if(NOT SLANG_DISABLE_NOTE_IDS MATCHES "^[0-9]+(,[0-9]+)*$")
        message(
            FATAL_ERROR
            "SLANG_DISABLE_NOTE_IDS must be comma-separated numeric IDs"
        )
    endif()
endif()

# Determine the library directory based on platform
# On Windows: shared libraries (.dll) go to bin/
# On Linux/Mac: shared libraries (.so/.dylib) go to lib/
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(slang_library_dir "bin")
else()
    set(slang_library_dir "lib")
endif()

# Set install directory using the platform-specific library directory
set(SLANG_STANDARD_MODULE_INSTALL_DIR "${slang_library_dir}/${SLANG_STANDARD_MODULE_DIR_NAME}" CACHE STRING "Installation directory for standard modules")

# Build-tree location (relative to ${slang_BINARY_DIR}/$<CONFIG>) where the module
# producers below actually write the .slang-module tree. This is deliberately
# kept separate from SLANG_STANDARD_MODULE_INSTALL_DIR: that is the install
# destination and is user-overridable, so it may point somewhere the build tree
# never populates. The optional embedding library scans this build-tree root.
set(SLANG_STANDARD_MODULE_BUILD_SUBDIR "${slang_library_dir}/${SLANG_STANDARD_MODULE_DIR_NAME}" CACHE INTERNAL "Build-tree subdir of the standard module files")

# The loose slang-standard-module-<version>/ tree is always built below (it is the
# input to the optional embedding library). When SLANG_EMBED_STANDARD_MODULES is
# on, the module subdirectories below skip their install() rules (each guarded on
# NOT SLANG_EMBED_STANDARD_MODULES), so the install/package contains only the
# embedding library in place of the loose files. This affects install()/CPack
# packaging only; consumers that copy straight from the build tree (e.g. CI
# artifact staging) still see the loose tree.

# Shared manifest of the serialized .slang-module files produced by the module
# subdirectories below. The optional embedding library
# (source/slang-standard-modules) DEPENDS on these actual outputs so that editing
# a module regenerates its embedded payload. Cleared here each configure (INTERNAL
# force-writes); each module subdirectory appends its own outputs.
set(SLANG_STANDARD_MODULE_FILES "" CACHE INTERNAL "Built standard .slang-module files")

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

# Build experimental modules
add_subdirectory(experimental)

# Build the experimental numeric interface modules.
add_subdirectory(numerics)

# Build the experimental functional module.
add_subdirectory(functional)
