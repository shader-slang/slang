#
# Build neural.slang shaders into a slang module and copy to build output
#

glob_append(neural_shader_sources "*.slang")

# Set output directories for build tree
# Files go under a "slang/" subdirectory so that `import slang.neural;` resolves
# to <standard-module-dir>/slang/neural.slang-module at runtime.
set(neural_output_dir
    "${CMAKE_BINARY_DIR}/$<CONFIG>/${slang_library_dir}/${SLANG_STANDARD_MODULE_DIR_NAME}/slang"
)
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
    set(SLANG_COMPILER
        "${SLANG_GENERATORS_PATH}/slang-bootstrap${CMAKE_HOST_EXECUTABLE_SUFFIX}"
    )
    set(SLANG_COMPILER_DEPENDENCY)
    set(SLANG_COMPILER_EXTRA_DEPS)
elseif(NOT SLANG_EMBED_CORE_MODULE)
    # When the binary core module is not embedded, slangc (isBootstrap=false) would
    # try to load the binary GLSL module from slang-glsl-module.dll. That binary was
    # compiled by slang-bootstrap against its own core-from-source compilation and may
    # not be compatible with slangc's independently compiled core module, causing a
    # crash. Use slang-bootstrap instead: it compiles core and GLSL from source
    # (isBootstrap=true skips binary-module loading) so there is no mismatch.
    # We also depend on generate_core_module_headers, which (on Windows shared builds)
    # copies slang-compiler.dll into the generators/ directory so slang-bootstrap
    # can find it at runtime.
    set(SLANG_COMPILER slang-bootstrap)
    set(SLANG_COMPILER_DEPENDENCY slang-bootstrap)
    set(SLANG_COMPILER_EXTRA_DEPS generate_core_module_headers)
elseif(SLANG_ENABLE_SLANGC)
    set(SLANG_COMPILER slangc)
    set(SLANG_COMPILER_DEPENDENCY slangc)
    set(SLANG_COMPILER_EXTRA_DEPS)
else()
    # As a fallback, use the system-installed `slangc` compiler
    set(SLANG_COMPILER slangc)
    set(SLANG_COMPILER_DEPENDENCY)
    set(SLANG_COMPILER_EXTRA_DEPS)
endif()

# Determine compiler flags based on build type
if(SLANG_STANDARD_MODULE_DEVELOP_BUILD)
    set(NEURAL_MODULE_DEFINES "-DUNIT_TEST")
else()
    set(NEURAL_MODULE_DEFINES "")
endif()

# Build neural.slang-module and output to the output directory
add_custom_command(
    OUTPUT ${neural_module_file}
    COMMAND
        ${SLANG_COMPILER} ${NEURAL_MODULE_DEFINES} neural.slang -o
        ${SLANG_NEURAL_MODULE_FILE_NAME}
    DEPENDS
        ${neural_copied_files}
        ${SLANG_COMPILER_DEPENDENCY}
        ${SLANG_COMPILER_EXTRA_DEPS}
    WORKING_DIRECTORY ${neural_output_dir}
    VERBATIM
)

# Create a target to ensure the neural module is built.
# When SLANG_EMBED_CORE_MODULE=OFF the compiler runs slang-bootstrap from source and currently
# crashes (exit 0xC0000005) when processing neural.slang's CoopMat/spirv_asm code. Exclude the
# module from the default ALL build in that configuration so development builds with
# SLANG_EMBED_CORE_MODULE=OFF do not fail. The target can still be built explicitly.
if(SLANG_EMBED_CORE_MODULE)
    add_custom_target(slang-neural-module ALL DEPENDS ${neural_module_file})
else()
    add_custom_target(slang-neural-module DEPENDS ${neural_module_file})
endif()
set_target_properties(slang-neural-module PROPERTIES FOLDER generated)

# Install the neural directory to the configured location in the release package
# Installs into <install-dir>/slang/ to match the `import slang.neural;` convention.
install(
    DIRECTORY ${neural_output_dir}/
    DESTINATION ${SLANG_STANDARD_MODULE_INSTALL_DIR}/slang
    FILES_MATCHING
    PATTERN "*.slang"
    PATTERN "*.slang-module"
)
