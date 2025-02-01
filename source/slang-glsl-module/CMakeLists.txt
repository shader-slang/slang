#
# Generate an embeddable glsl module
#
if (${SLANG_EMBED_CORE_MODULE})

set(glsl_module_generated_header_dir ${CMAKE_CURRENT_BINARY_DIR})
set(glsl_module_generated_header
    ${glsl_module_generated_header_dir}/slang-glsl-module-generated.h
)
add_custom_command(
    OUTPUT ${glsl_module_generated_header}
    COMMAND
        slang-bootstrap -archive-type riff-lz4 -save-glsl-module-bin-source
        ${glsl_module_generated_header}
    DEPENDS slang-bootstrap
    VERBATIM
)

set(glsl_module_common_args
    .
    MODULE
    FOLDER
    generated
    LINK_WITH_PRIVATE
    core
    USE_EXTRA_WARNINGS
    EXPLICIT_SOURCE
    ./slang-embedded-glsl-module.cpp
)

slang_add_target(
    ${glsl_module_common_args}
    TARGET_NAME slang-glsl-module
    EXCLUDE_FROM_ALL
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_SHARED_LIBRARY_TOOL
    EXPORT_SET_NAME SlangTargets
    EXPLICIT_SOURCE ${glsl_module_generated_header}
    INCLUDE_DIRECTORIES_PRIVATE ${glsl_module_generated_header_dir}
    INSTALL
)

endif()