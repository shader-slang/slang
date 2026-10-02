#
# In this file, for core module source embed and core module embed itself, we define two
# targets, one which includes the embed and one which does not, these are
# linked in as needed to libslang and libslang-without-embedded-core-module (for
# slang-bootstrap)
#
# If MSVC supported weak linking we could just have a single library for each
# and just not link it if we didn't want the embed...
#

#
# Compiling the meta.slang files
#

# List of *.meta.slang headers
set(core_module_meta_source_dir "${slang_SOURCE_DIR}/source/slang")
glob_append(
    core_module_meta_source
    "${core_module_meta_source_dir}/*.meta.slang"
)

set(core_module_meta_output_dir "${CMAKE_CURRENT_BINARY_DIR}/core-module-meta")

# Generate the output file list
set(core_module_meta_generated_headers)
foreach(meta_source ${core_module_meta_source})
    file(
        RELATIVE_PATH
        meta_source_relative
        "${core_module_meta_source_dir}"
        ${meta_source}
    )
    list(
        APPEND
        core_module_meta_generated_headers
        "${core_module_meta_output_dir}/${meta_source_relative}.h"
    )
endforeach()

add_custom_command(
    OUTPUT ${core_module_meta_generated_headers}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${core_module_meta_output_dir}
    COMMAND
        slang-generate ${core_module_meta_source} --target-directory
        ${core_module_meta_output_dir}
    DEPENDS ${core_module_meta_source} slang-generate
    WORKING_DIRECTORY "${core_module_meta_source_dir}"
    VERBATIM
)

#
# Generate embedded core module source library (or a dummy library with just nullptrs)
#

set(core_module_source_common_args
    .
    OBJECT
    EXCLUDE_FROM_ALL
    USE_EXTRA_WARNINGS
    LINK_WITH_PRIVATE
    core
    slang-capability-defs
    slang-fiddle-output
    SPIRV-Headers::SPIRV-Headers
    INCLUDE_DIRECTORIES_PRIVATE
    ../slang
    ${core_module_meta_output_dir}
    FOLDER
    generated
    EXPLICIT_SOURCE
    ./slang-embedded-core-module-source.cpp
    EXPORT_MACRO_PREFIX
    SLANG
    EXPORT_TYPE_AS
    ${SLANG_LIB_TYPE}
)

slang_add_target(
    ${core_module_source_common_args}
    TARGET_NAME slang-embedded-core-module-source
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_EMBED_CORE_MODULE_SOURCE
    EXPLICIT_SOURCE ${core_module_meta_generated_headers}
    EXCLUDE_FROM_ALL
)

slang_add_target(
    ${core_module_source_common_args}
    TARGET_NAME slang-no-embedded-core-module-source
    EXCLUDE_FROM_ALL
)

#
# Use that source to generate an embeddable core module, this uses
# slang-bootstrap which will include the embedded source we have above
#

set(core_module_generated_header_dir ${CMAKE_CURRENT_BINARY_DIR})
set(core_module_generated_header
    ${core_module_generated_header_dir}/slang-core-module-generated.h
)
set(core_module_archive_without_timestamp
    ${CMAKE_CURRENT_BINARY_DIR}/slang-core-module-without-timestamp.bin
)
set(autodiff_module_generated_header
    ${core_module_generated_header_dir}/slang-autodiff-module-generated.h
)
set(autodiff_module_archive_without_timestamp
    ${CMAKE_CURRENT_BINARY_DIR}/slang-autodiff-module-without-timestamp.bin
)
set(glsl_module_generated_header_dir
    ${CMAKE_CURRENT_BINARY_DIR}/../slang-glsl-module
)
set(glsl_module_generated_header
    ${glsl_module_generated_header_dir}/slang-glsl-module-generated.h
)

# Propagate to parent directory scope, so they're visible to
# slang-glsl-module/CMakeLists.txt
set(glsl_module_generated_header_dir
    ${glsl_module_generated_header_dir}
    PARENT_SCOPE
)
set(glsl_module_generated_header ${glsl_module_generated_header} PARENT_SCOPE)
set(core_module_archive_without_timestamp ${core_module_archive_without_timestamp} PARENT_SCOPE)

# When SLANG_EMBED_CORE_MODULE=OFF, slang-without-embedded-core-module is an ALIAS
# for slang, whose DLL (slang-compiler.dll) is placed in bin/ rather than generators/.
# slang-bootstrap.exe lives in generators/, and the Windows DLL loader searches the
# EXE directory first, so it cannot find slang-compiler.dll across directories.
# Pre-copy slang-compiler.dll into the generators/ directory before invoking
# slang-bootstrap so the loader finds it in the expected location.
if(NOT SLANG_EMBED_CORE_MODULE AND WIN32 AND SLANG_LIB_TYPE STREQUAL "SHARED")
    set(bootstrap_pre_copy_command
        COMMAND
        ${CMAKE_COMMAND}
        -E
        copy_if_different
        "$<TARGET_FILE:slang>"
        "$<TARGET_FILE_DIR:slang-bootstrap>/$<TARGET_FILE_NAME:slang>"
    )
    set(bootstrap_extra_deps slang)
else()
    set(bootstrap_pre_copy_command "")
    set(bootstrap_extra_deps "")
endif()

add_custom_command(
    OUTPUT
        ${core_module_archive_without_timestamp}
        ${core_module_generated_header}
        ${autodiff_module_archive_without_timestamp}
        ${autodiff_module_generated_header}
        ${glsl_module_generated_header}
    ${bootstrap_pre_copy_command}
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${core_module_generated_header_dir}
        ${glsl_module_generated_header_dir}
    COMMAND
        "$<TARGET_FILE:slang-bootstrap>"
        -compile-core-module
        -archive-type
        riff-lz4
        -save-core-module
        ${core_module_archive_without_timestamp}
        -save-core-module-bin-source
        ${core_module_generated_header}
        -save-autodiff-module
        ${autodiff_module_archive_without_timestamp}
        -save-autodiff-module-bin-source
        ${autodiff_module_generated_header}
        -save-glsl-module-bin-source
        ${glsl_module_generated_header}
    DEPENDS
        slang-bootstrap
        slang-without-embedded-core-module
        ${bootstrap_extra_deps}
    VERBATIM
)

add_custom_target(
    generate_core_module
    DEPENDS
        ${core_module_archive_without_timestamp}
        ${core_module_generated_header}
        ${autodiff_module_archive_without_timestamp}
        ${autodiff_module_generated_header}
        ${glsl_module_generated_header}
)
set_target_properties(generate_core_module PROPERTIES FOLDER generated)

# Downstream targets depend on generate_core_module instead of the generated
# files directly. With Visual Studio, a file-level dependency on a byproduct
# copies the producer command into the dependent utility project, which makes
# multi-target builds run the core generation command more than once.
add_custom_target(generate_glsl_module_header)
add_dependencies(generate_glsl_module_header generate_core_module)
set_target_properties(generate_glsl_module_header PROPERTIES FOLDER generated)

# Add a target so that existing dependents can request both generated headers.
add_custom_target(generate_core_module_headers)
add_dependencies(generate_core_module_headers generate_core_module generate_glsl_module_header)
set_target_properties(generate_core_module_headers PROPERTIES FOLDER generated)

set(core_module_common_args
    .
    OBJECT
    EXCLUDE_FROM_ALL
    EXPORT_MACRO_PREFIX
    SLANG
    EXPORT_TYPE_AS
    ${SLANG_LIB_TYPE}
    FOLDER
    generated
    LINK_WITH_PRIVATE
    core
    USE_EXTRA_WARNINGS
    EXPLICIT_SOURCE
    ./slang-embedded-core-module.cpp
)

slang_add_target(
    ${core_module_common_args}
    TARGET_NAME slang-embedded-core-module
    REQUIRES generate_core_module
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_EMBED_CORE_MODULE
    INCLUDE_DIRECTORIES_PRIVATE ${core_module_generated_header_dir}
)

slang_add_target(
    ${core_module_common_args}
    TARGET_NAME slang-no-embedded-core-module
)
