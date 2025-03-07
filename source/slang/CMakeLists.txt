#
# generate capability files
#
glob_append(SLANG_CAPABILITY_SOURCE "*.capdef")

set(SLANG_CAPABILITY_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/capability")

# Generate the output file list
set(SLANG_CAPABILITY_GENERATED_HEADERS
    "${SLANG_CAPABILITY_OUTPUT_DIR}/slang-generated-capability-defs.h"
    "${SLANG_CAPABILITY_OUTPUT_DIR}/slang-generated-capability-defs-impl.h"
)
set(SLANG_CAPABILITY_GENERATED_SOURCE
    ${SLANG_CAPABILITY_GENERATED_HEADERS}
    "${SLANG_CAPABILITY_OUTPUT_DIR}/slang-lookup-capability-defs.cpp"
)
add_custom_command(
    OUTPUT ${SLANG_CAPABILITY_GENERATED_SOURCE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SLANG_CAPABILITY_OUTPUT_DIR}
    COMMAND
        slang-capability-generator ${SLANG_CAPABILITY_SOURCE} --target-directory
        ${SLANG_CAPABILITY_OUTPUT_DIR} --doc
        "${slang_SOURCE_DIR}/docs/user-guide/a3-02-reference-capability-atoms.md"
    DEPENDS ${SLANG_CAPABILITY_SOURCE} slang-capability-generator
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    VERBATIM
)
slang_add_target(
    slang-capability-defs
    OBJECT
    EXPORT_MACRO_PREFIX SLANG
    USE_EXTRA_WARNINGS
    EXPLICIT_SOURCE ${SLANG_CAPABILITY_GENERATED_HEADERS}
    EXPORT_TYPE_AS ${SLANG_LIB_TYPE}
    LINK_WITH_PRIVATE core
    INCLUDE_DIRECTORIES_PUBLIC
        "${SLANG_CAPABILITY_OUTPUT_DIR}"
        "${slang_SOURCE_DIR}/source/slang"
    EXCLUDE_FROM_ALL
    FOLDER generated
)
slang_add_target(
    slang-capability-lookup
    OBJECT
    EXPORT_MACRO_PREFIX SLANG
    USE_EXTRA_WARNINGS
    EXPLICIT_SOURCE ${SLANG_CAPABILITY_GENERATED_SOURCE}
    EXPORT_TYPE_AS ${SLANG_LIB_TYPE}
    LINK_WITH_PRIVATE core slang-capability-defs
    EXCLUDE_FROM_ALL
    FOLDER generated
)

#
# generated headers for reflection
#

set(SLANG_REFLECT_INPUT
    slang-ast-support-types.h
    slang-ast-base.h
    slang-ast-decl.h
    slang-ast-expr.h
    slang-ast-modifier.h
    slang-ast-stmt.h
    slang-ast-type.h
    slang-ast-val.h
)
# Make them absolute
list(TRANSFORM SLANG_REFLECT_INPUT PREPEND "${CMAKE_CURRENT_LIST_DIR}/")

set(SLANG_REFLECT_GENERATED_HEADERS
    slang-generated-obj.h
    slang-generated-obj-macro.h
    slang-generated-ast.h
    slang-generated-ast-macro.h
    slang-generated-value.h
    slang-generated-value-macro.h
)
set(SLANG_REFLECT_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/ast-reflect")
list(
    TRANSFORM SLANG_REFLECT_GENERATED_HEADERS
    PREPEND "${SLANG_REFLECT_OUTPUT_DIR}/"
)

add_custom_command(
    OUTPUT ${SLANG_REFLECT_GENERATED_HEADERS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SLANG_REFLECT_OUTPUT_DIR}
    COMMAND
        slang-cpp-extractor ${SLANG_REFLECT_INPUT} -strip-prefix slang- -o
        ${SLANG_REFLECT_OUTPUT_DIR}/slang-generated -output-fields -mark-suffix
        _CLASS
    DEPENDS ${SLANG_REFLECT_INPUT} slang-cpp-extractor
    VERBATIM
)

add_library(
    slang-reflect-headers
    INTERFACE
    EXCLUDE_FROM_ALL
    ${SLANG_REFLECT_GENERATED_HEADERS}
)
set_target_properties(slang-reflect-headers PROPERTIES FOLDER generated)
target_include_directories(
    slang-reflect-headers
    INTERFACE ${SLANG_REFLECT_OUTPUT_DIR}
)

#
# generated lookup tables
#

if(NOT SLANG_USE_SYSTEM_SPIRV_HEADERS)
    if(NOT SLANG_OVERRIDE_SPIRV_HEADERS_PATH)
        set(SLANG_SPIRV_HEADERS_INCLUDE_DIR
            "${slang_SOURCE_DIR}/external/spirv-headers/include"
        )
    else()
        set(SLANG_SPIRV_HEADERS_INCLUDE_DIR
            "${SLANG_OVERRIDE_SPIRV_HEADERS_PATH}/include"
        )
    endif()
endif()

set(SLANG_LOOKUP_GENERATOR_INPUT_JSON
    "${SLANG_SPIRV_HEADERS_INCLUDE_DIR}/spirv/unified1/extinst.glsl.std.450.grammar.json"
)
set(SLANG_LOOKUP_GENERATOR_OUTPUT_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/slang-lookup-tables/"
)
set(SLANG_LOOKUP_GENERATED_SOURCE
    "${SLANG_LOOKUP_GENERATOR_OUTPUT_DIR}/slang-lookup-GLSLstd450.cpp"
)
set(SLANG_RECORD_REPLAY_SYSTEM
    "${slang_SOURCE_DIR}/source/slang-record-replay/record"
    "${slang_SOURCE_DIR}/source/slang-record-replay/util"
)

add_custom_command(
    OUTPUT ${SLANG_LOOKUP_GENERATED_SOURCE}
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${SLANG_LOOKUP_GENERATOR_OUTPUT_DIR}
    COMMAND
        slang-lookup-generator ${SLANG_LOOKUP_GENERATOR_INPUT_JSON}
        ${SLANG_LOOKUP_GENERATED_SOURCE} "GLSLstd450" "GLSLstd450"
        "spirv/unified1/GLSL.std.450.h"
    DEPENDS ${SLANG_LOOKUP_GENERATOR_INPUT_JSON} slang-lookup-generator
    VERBATIM
)

set(SLANG_SPIRV_CORE_SOURCE_JSON
    "${SLANG_SPIRV_HEADERS_INCLUDE_DIR}/spirv/unified1/spirv.core.grammar.json"
)
set(SLANG_SPIRV_CORE_GRAMMAR_SOURCE
    "${SLANG_LOOKUP_GENERATOR_OUTPUT_DIR}/slang-spirv-core-grammar-embed.cpp"
)
add_custom_command(
    OUTPUT ${SLANG_SPIRV_CORE_GRAMMAR_SOURCE}
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${SLANG_LOOKUP_GENERATOR_OUTPUT_DIR}
    COMMAND
        slang-spirv-embed-generator ${SLANG_SPIRV_CORE_SOURCE_JSON}
        ${SLANG_SPIRV_CORE_GRAMMAR_SOURCE}
    DEPENDS ${SLANG_SPIRV_CORE_SOURCE_JSON} slang-spirv-embed-generator
    VERBATIM
)

slang_add_target(
    slang-lookup-tables
    OBJECT
    EXPORT_MACRO_PREFIX SLANG
    USE_EXTRA_WARNINGS
    EXPLICIT_SOURCE
        ${SLANG_LOOKUP_GENERATED_SOURCE}
        ${SLANG_SPIRV_CORE_GRAMMAR_SOURCE}
    EXPORT_TYPE_AS ${SLANG_LIB_TYPE}
    LINK_WITH_PRIVATE core SPIRV-Headers
    EXCLUDE_FROM_ALL
    FOLDER generated
)

#
# Generate the version header
#

configure_file(
    ${slang_SOURCE_DIR}/slang-tag-version.h.in
    slang-version-header/slang-tag-version.h
)

#
# Copy headers into the build tree to support source linking
#
add_custom_target(
    copy_slang_headers
    COMMAND
        ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_BINARY_DIR}/$<CONFIG>/include"
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory "${slang_SOURCE_DIR}/include"
        "${CMAKE_BINARY_DIR}/$<CONFIG>/include"
    COMMAND
        ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_BINARY_DIR}/slang-version-header/slang-tag-version.h"
        "${CMAKE_BINARY_DIR}/$<CONFIG>/include/slang-tag-version.h"
)
set_target_properties(copy_slang_headers PROPERTIES FOLDER "generated")

#
# Slang itself
#

set(slang_build_args
    USE_EXTRA_WARNINGS
    EXTRA_COMPILE_OPTIONS_PRIVATE
    # a warning is disabled for a memset boundary check.
    # everything looked fine and it is unclear why the checking fails
    $<$<CXX_COMPILER_ID:GNU>:-Wno-error=stringop-overflow>
    INCLUDE_DIRECTORIES_PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/slang-version-header
    EXPORT_MACRO_PREFIX
    SLANG
    EXPORT_TYPE_AS
    ${SLANG_LIB_TYPE}
    EXTRA_SOURCE_DIRS
    ${SLANG_RECORD_REPLAY_SYSTEM}
    REQUIRES
    copy_slang_headers
)
set(slang_link_args
    LINK_WITH_PRIVATE
    core
    prelude
    compiler-core
    slang-capability-defs
    slang-capability-lookup
    slang-reflect-headers
    slang-lookup-tables
    SPIRV-Headers
)
set(slang_interface_args INCLUDE_DIRECTORIES_PUBLIC ${slang_SOURCE_DIR}/include)
set(slang_public_lib_args
    PUBLIC_HEADERS
    ${slang_SOURCE_DIR}/include/slang*.h
    ${CMAKE_CURRENT_BINARY_DIR}/slang-version-header/*.h
    LINK_WITH_PRIVATE
    $<IF:$<BOOL:${SLANG_EMBED_CORE_MODULE}>,slang-embedded-core-module,slang-no-embedded-core-module>
    $<IF:$<BOOL:${SLANG_EMBED_CORE_MODULE_SOURCE}>,slang-embedded-core-module-source,slang-no-embedded-core-module-source>
    INSTALL
    EXPORT_SET_NAME
    SlangTargets
)

#
# Minimal static slang used in generating the embedded core module
#

#
# Slang itself
#

if(NOT SLANG_EMBED_CORE_MODULE)
    # If we're not embedding core module we can just do a normal build of slang,
    # including all the options, this can also serve as our no embedded core module
    # library for slang-bootstrap (not that we need that anyway)
    slang_add_target(
        .
        ${SLANG_LIB_TYPE}
        ${slang_build_args}
        ${slang_link_args}
        ${slang_interface_args}
        ${slang_public_lib_args}
        INSTALL_COMPONENT generators
    )
    add_library(slang-without-embedded-core-module ALIAS slang)
else()
    # However if we're embedding core module, we need to make two different
    # libraries, one with the embedded core module and one without, so first define
    # the shared objects as one target so we don't build them twice.
    slang_add_target(
        .
        OBJECT
        ${slang_build_args}
        ${slang_link_args}
        TARGET_NAME slang-common-objects
        EXCLUDE_FROM_ALL
    )
    slang_add_target(
        .
        ${SLANG_LIB_TYPE}
        EXPORT_MACRO_PREFIX SLANG
        ${slang_link_args}
        ${slang_interface_args}
        NO_SOURCE
        TARGET_NAME slang-without-embedded-core-module
        EXCLUDE_FROM_ALL
        LINK_WITH_PRIVATE
            slang-common-objects
            slang-no-embedded-core-module
            slang-embedded-core-module-source
        OUTPUT_DIR generators
        FOLDER generators
        INSTALL_COMPONENT generators
        EXPORT_SET_NAME SlangGeneratorTargets
    )
    slang_add_target(
        .
        ${SLANG_LIB_TYPE}
        EXPORT_MACRO_PREFIX SLANG
        ${slang_link_args}
        ${slang_interface_args}
        ${slang_public_lib_args}
        LINK_WITH_PRIVATE slang-common-objects
        NO_SOURCE
    )
endif()
