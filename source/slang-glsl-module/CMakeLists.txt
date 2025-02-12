#
# Generate an embeddable glsl module
#
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

slang_add_target(
    .
    MODULE
    FOLDER generated
    LINK_WITH_PRIVATE core
    USE_EXTRA_WARNINGS
    EXPLICIT_SOURCE
        ./slang-embedded-glsl-module.cpp
        ${glsl_module_generated_header}
    EXCLUDE_FROM_ALL
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_SHARED_LIBRARY_TOOL
    EXPORT_SET_NAME SlangTargets
    INCLUDE_DIRECTORIES_PRIVATE ${glsl_module_generated_header_dir}
    INSTALL
)
