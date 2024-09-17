#
# Compiling the meta.slang files
#

# List of *.meta.slang headers
glob_append(SLANG_STDLIB_META_SOURCE "*.meta.slang")

set(SLANG_STDLIB_META_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/stdlib-meta")

# Generate the output file list
set(SLANG_STDLIB_META_GENERATED_HEADERS)
foreach(meta_source ${SLANG_STDLIB_META_SOURCE})
    file(
        RELATIVE_PATH
        meta_source_relative
        ${CMAKE_CURRENT_LIST_DIR}
        ${meta_source}
    )
    list(
        APPEND
        SLANG_STDLIB_META_GENERATED_HEADERS
        "${SLANG_STDLIB_META_OUTPUT_DIR}/${meta_source_relative}.h"
    )
endforeach()

add_custom_command(
    OUTPUT ${SLANG_STDLIB_META_GENERATED_HEADERS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SLANG_STDLIB_META_OUTPUT_DIR}
    COMMAND
        slang-generate ${SLANG_STDLIB_META_SOURCE} --target-directory
        ${SLANG_STDLIB_META_OUTPUT_DIR}
    DEPENDS ${SLANG_STDLIB_META_SOURCE} slang-generate
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    VERBATIM
)

add_library(
    slang-meta-headers
    INTERFACE
    EXCLUDE_FROM_ALL
    ${SLANG_STDLIB_META_GENERATED_HEADERS}
)
set_target_properties(slang-meta-headers PROPERTIES FOLDER generated)
target_include_directories(
    slang-meta-headers
    INTERFACE ${SLANG_STDLIB_META_OUTPUT_DIR}
)

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
        ${SLANG_CAPABILITY_OUTPUT_DIR}
    DEPENDS ${SLANG_CAPABILITY_SOURCE} slang-capability-generator
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    VERBATIM
)
slang_add_target(
    slang-capability-defs
    OBJECT
    USE_EXTRA_WARNINGS
    EXPLICIT_SOURCE ${SLANG_CAPABILITY_GENERATED_HEADERS}
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
    USE_EXTRA_WARNINGS
    EXPLICIT_SOURCE ${SLANG_CAPABILITY_GENERATED_SOURCE}
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

if (NOT SLANG_USE_SYSTEM_SPIRV_HEADERS)
    set(SLANG_SPIRV_HEADERS_INCLUDE_DIR "${slang_SOURCE_DIR}/external/spirv-headers/include")
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
        slang-spirv-embed-generator
        ${SLANG_SPIRV_CORE_SOURCE_JSON}
        ${SLANG_SPIRV_CORE_GRAMMAR_SOURCE}
    DEPENDS ${SLANG_SPIRV_CORE_SOURCE_JSON} slang-spirv-embed-generator
    VERBATIM
)

slang_add_target(
    slang-lookup-tables
    OBJECT
    USE_EXTRA_WARNINGS
    EXPLICIT_SOURCE ${SLANG_LOOKUP_GENERATED_SOURCE} ${SLANG_SPIRV_CORE_GRAMMAR_SOURCE}
    LINK_WITH_PRIVATE core SPIRV-Headers
    EXCLUDE_FROM_ALL
    FOLDER generated
)

set(SLANG_CAPTURE_REPLAY_SYSTEM
    "${slang_SOURCE_DIR}/source/slang-capture-replay"
)

#
# Generate an embeddable stdlib
#

set(SLANG_STDLIB_GENERATED_HEADER
    ${CMAKE_CURRENT_BINARY_DIR}/slang-stdlib-generated.h
)
add_custom_command(
    OUTPUT ${SLANG_STDLIB_GENERATED_HEADER}
    COMMAND
        slang-bootstrap -archive-type riff-lz4 -save-stdlib-bin-source
        ${SLANG_STDLIB_GENERATED_HEADER}
    DEPENDS slang-bootstrap
    VERBATIM
)
add_library(
    slang-stdlib-embed-headers
    INTERFACE
    EXCLUDE_FROM_ALL
    ${SLANG_STDLIB_GENERATED_HEADER}
)
set_target_properties(slang-stdlib-embed-headers PROPERTIES FOLDER generated)
target_include_directories(
    slang-stdlib-embed-headers
    INTERFACE ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Generate the version header
#

configure_file(${slang_SOURCE_DIR}/slang-tag-version.h.in slang-version-header/slang-tag-version.h)

#
# Slang itself
#

set(slang_common_args
    EXTRA_SOURCE_DIRS ${SLANG_CAPTURE_REPLAY_SYSTEM}
    USE_EXTRA_WARNINGS
    LINK_WITH_PRIVATE
        core
        compiler-core
        slang-capability-defs
        slang-capability-lookup
        slang-reflect-headers
        slang-lookup-tables
        SPIRV-Headers
    EXTRA_COMPILE_OPTIONS_PRIVATE
        # a warning is disabled for a memset boundary check.
        # everything looked fine and it is unclear why the checking fails
        $<$<CXX_COMPILER_ID:GNU>:-Wno-error=stringop-overflow>
    # slang.h is in the project root, so include that directory in the interface
    # for slang
    INCLUDE_DIRECTORIES_PUBLIC ${slang_SOURCE_DIR}
    INCLUDE_DIRECTORIES_PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/slang-version-header
    EXPORT_MACRO_PREFIX SLANG
)

#
# Minimal static slang used in generating the embedded stdlib
#

slang_add_target(
    .
    OBJECT
    ${slang_common_args}
    TARGET_NAME slang-no-embedded-stdlib
    EXCLUDE_FROM_ALL
    LINK_WITH_PRIVATE slang-meta-headers
    EXTRA_COMPILE_DEFINITIONS_PUBLIC SLANG_STATIC
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_WITHOUT_EMBEDDED_STD_LIB
    FOLDER generators
)

#
# Slang with whatever options
#

slang_add_target(
    .
    ${SLANG_LIB_TYPE}
    ${slang_common_args}
    LINK_WITH_PRIVATE
        prelude
        $<$<BOOL:${SLANG_EMBED_STDLIB}>:slang-stdlib-embed-headers>
        $<$<BOOL:${SLANG_EMBED_STDLIB_SOURCE}>:slang-meta-headers>
    EXTRA_COMPILE_DEFINITIONS_PRIVATE
        $<$<NOT:$<BOOL:${SLANG_EMBED_STDLIB}>>:SLANG_WITHOUT_EMBEDDED_STD_LIB>
        $<$<NOT:$<BOOL:${SLANG_EMBED_STDLIB_SOURCE}>>:SLANG_DISABLE_STDLIB_SOURCE>
    EXPORT_MACRO_PREFIX SLANG
    INSTALL
    PUBLIC_HEADERS ${slang_SOURCE_DIR}/slang*.h ${CMAKE_CURRENT_BINARY_DIR}/slang-version-header/*.h
)
