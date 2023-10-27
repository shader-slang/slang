#
# Compiling the meta.slang files
#

# List of *.meta.slang headers
glob_append(SLANG_STDLIB_META_SOURCE "*.meta.slang")

# Generate the output file list
list(
    TRANSFORM SLANG_STDLIB_META_SOURCE
    APPEND .h
    OUTPUT_VARIABLE SLANG_STDLIB_META_GENERATED_HEADERS
)

add_custom_command(
    OUTPUT ${SLANG_STDLIB_META_GENERATED_HEADERS}
    COMMAND
        slang-generate ${SLANG_STDLIB_META_SOURCE} --target-directory
        ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${SLANG_STDLIB_META_SOURCE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
)

add_library(slang-meta-headers INTERFACE EXCLUDE_FROM_ALL)
target_sources(
    slang-meta-headers
    INTERFACE ${SLANG_STDLIB_META_GENERATED_HEADERS}
)
target_include_directories(
    slang-meta-headers
    INTERFACE ${CMAKE_CURRENT_BINARY_DIR}
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
# generated headers for reflection
#

glob_append(SLANG_REFLECT_SOURCE "slang-ast-*.h")

set(SLANG_REFLECT_GENERATED_HEADERS)
foreach(type obj ast value)
    foreach(macro "" -macro)
        list(
            APPEND
            SLANG_REFLECT_GENERATED_HEADERS
            "slang-generated-${type}${macro}.h"
        )
    endforeach()
endforeach()

add_custom_command(
    OUTPUT ${SLANG_REFLECT_GENERATED_HEADERS}
    COMMAND
        slang-cpp-extractor -strip-prefix slang- -o
        ${CMAKE_CURRENT_BINARY_DIR}/slang-generated -output-fields -mark-suffix
        _CLASS ${SLANG_REFLECT_SOURCE}
    DEPENDS ${SLANG_REFLECT_SOURCE}
    VERBATIM
)
add_library(slang-reflect-headers INTERFACE EXCLUDE_FROM_ALL)
target_sources(slang-reflect-headers INTERFACE ${SLANG_REFLECT_SOURCE})
target_include_directories(
    slang-reflect-headers
    INTERFACE ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Slang itself
#
slang_add_target(
    .
    SHARED
    LINK_WITH_PRIVATE core compiler-core prelude SPIRV-Headers
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
