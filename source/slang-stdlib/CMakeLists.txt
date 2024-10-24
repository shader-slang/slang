#
# In this file, for stdlib source embed and stdlib embed itself, we define two
# targets, one which includes the embed and one which does not, these are
# linked in as needed to libslang and libslang-without-embedded-stdlib (for
# slang-bootstrap)
#
# If MSVC supported weak linking we could just have a single library for each
# and just not link it if we didn't want the embed...
#

#
# Generate an embeddable stdlib
#

set(stdlib_generated_header_dir ${CMAKE_CURRENT_BINARY_DIR})
set(stdlib_generated_header
    ${stdlib_generated_header_dir}/slang-stdlib-generated.h
)
add_custom_command(
    OUTPUT ${stdlib_generated_header}
    COMMAND
        slang-bootstrap -archive-type riff-lz4 -save-standard-modules-bin-source
        ${stdlib_generated_header}
    DEPENDS slang-bootstrap
    VERBATIM
)

set(stdlib_common_args
    .
    OBJECT
    EXCLUDE_FROM_ALL
    EXPORT_MACRO_PREFIX SLANG
    EXPORT_TYPE_AS ${SLANG_LIB_TYPE}
    FOLDER generated
    LINK_WITH_PRIVATE core
    USE_EXTRA_WARNINGS
    EXPLICIT_SOURCE ./slang-embedded-stdlib.cpp 
)

slang_add_target(
    ${stdlib_common_args}
    TARGET_NAME slang-embedded-stdlib
    EXPLICIT_SOURCE  ${stdlib_generated_header}
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_EMBED_STDLIB
    INCLUDE_DIRECTORIES_PRIVATE ${stdlib_generated_header_dir}
)

slang_add_target(
    ${stdlib_common_args}
    TARGET_NAME slang-no-embedded-stdlib
)

#
# Compiling the meta.slang files
#

# List of *.meta.slang headers
set(stdlib_meta_source_dir "${slang_SOURCE_DIR}/source/slang")
glob_append(stdlib_meta_source "${stdlib_meta_source_dir}/*.meta.slang")

set(stdlib_meta_output_dir "${CMAKE_CURRENT_BINARY_DIR}/stdlib-meta")

# Generate the output file list
set(stdlib_meta_generated_headers)
foreach(meta_source ${stdlib_meta_source})
    file(
        RELATIVE_PATH
        meta_source_relative
        "${stdlib_meta_source_dir}"
        ${meta_source}
    )
    list(
        APPEND
        stdlib_meta_generated_headers
        "${stdlib_meta_output_dir}/${meta_source_relative}.h"
    )
endforeach()

add_custom_command(
    OUTPUT ${stdlib_meta_generated_headers}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${stdlib_meta_output_dir}
    COMMAND
        slang-generate ${stdlib_meta_source} --target-directory
        ${stdlib_meta_output_dir}
    DEPENDS ${stdlib_meta_source} slang-generate
    WORKING_DIRECTORY "${stdlib_meta_source_dir}"
    VERBATIM
)

add_custom_target(generate-stdlib-headers DEPENDS ${stdlib_meta_generated_headers})
set_target_properties(generate-stdlib-headers PROPERTIES FOLDER generated)

#
# Generate embedded stdlib source
#

set(stdlib_source_common_args
    .
    OBJECT
    EXCLUDE_FROM_ALL
    USE_EXTRA_WARNINGS
    LINK_WITH_PRIVATE 
        core 
        slang-capability-defs 
        slang-reflect-headers 
        SPIRV-Headers 
    INCLUDE_DIRECTORIES_PRIVATE ../slang
    FOLDER generated
    EXPLICIT_SOURCE 
      ./slang-embedded-stdlib-source.cpp
      ${stdlib_meta_generated_headers}
    REQUIRES generate-stdlib-headers
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_EMBED_STDLIB_SOURCE
    INCLUDE_DIRECTORIES_PRIVATE 
      ${stdlib_meta_output_dir}
    EXPORT_MACRO_PREFIX SLANG
    EXPORT_TYPE_AS ${SLANG_LIB_TYPE}
)

slang_add_target(
    ${stdlib_source_common_args}
    TARGET_NAME slang-embedded-stdlib-source
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_EMBED_STDLIB_SOURCE
)

slang_add_target(
    ${stdlib_source_common_args}
    TARGET_NAME slang-no-embedded-stdlib-source
)
