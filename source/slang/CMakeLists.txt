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
    DEPENDS ${SLANG_STDLIB_META_SOURCE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    VERBATIM
)

add_library(slang-meta-headers INTERFACE EXCLUDE_FROM_ALL)
target_sources(
    slang-meta-headers
    INTERFACE ${SLANG_STDLIB_META_GENERATED_HEADERS}
)
target_include_directories(
    slang-meta-headers
    INTERFACE ${SLANG_STDLIB_META_OUTPUT_DIR}
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
    DEPENDS ${SLANG_REFLECT_INPUT}
    VERBATIM
)

add_library(slang-reflect-headers INTERFACE EXCLUDE_FROM_ALL)
target_sources(
    slang-reflect-headers
    INTERFACE ${SLANG_REFLECT_GENERATED_HEADERS}
)
target_include_directories(
    slang-reflect-headers
    INTERFACE ${SLANG_REFLECT_OUTPUT_DIR}
)

#
# Slang static
#

slang_add_target(
    .
    STATIC
    TARGET_NAME slang-static
    EXCLUDE_FROM_ALL
    LINK_WITH_PRIVATE
        core
        compiler-core
        prelude
        slang-meta-headers
        slang-reflect-headers
        SPIRV-Headers
)
target_compile_definitions(
    slang-static
    PUBLIC SLANG_STATIC
    PRIVATE SLANG_WITHOUT_EMBEDDED_STD_LIB
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
    VERBATIM
)
add_library(slang-stdlib-embed-headers INTERFACE EXCLUDE_FROM_ALL)
target_sources(
    slang-stdlib-embed-headers
    INTERFACE ${SLANG_STDLIB_GENERATED_HEADER}
)
target_include_directories(
    slang-stdlib-embed-headers
    INTERFACE ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Slang itself
#
slang_add_target(
    .
    SHARED
    LINK_WITH_PRIVATE
        core
        compiler-core
        prelude
        slang-meta-headers
        slang-reflect-headers
        SPIRV-Headers
    # slang.h is in the project root, so include that directory in the interface
    # for slang
    INCLUDE_DIRECTORIES_PUBLIC ${SLANG_ROOT_DIR}
    EXPORT_MACRO_PREFIX SLANG
)

if(NOT SLANG_EMBED_STDLIB_SOURCE)
    target_compile_definitions(slang PRIVATE SLANG_DISABLE_STDLIB_SOURCE)
endif()

if(SLANG_EMBED_STDLIB)
    target_link_libraries(slang PRIVATE slang-stdlib-embed-headers)
else()
    target_compile_definitions(slang PRIVATE SLANG_WITHOUT_EMBEDDED_STD_LIB)
endif()
