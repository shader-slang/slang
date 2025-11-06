#
# Invoke the "fiddle" tool to generate source
#

set(SLANG_FIDDLE_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SLANG_FIDDLE_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/fiddle")

file(
    GLOB SLANG_FIDDLE_INPUT_FILE_NAMES
    CONFIGURE_DEPENDS
    RELATIVE "${SLANG_FIDDLE_INPUT_DIR}"
    "*.h"
    "*.cpp"
)

list(
    TRANSFORM SLANG_FIDDLE_INPUT_FILE_NAMES
    PREPEND "${SLANG_FIDDLE_INPUT_DIR}/"
    OUTPUT_VARIABLE SLANG_FIDDLE_INPUTS
)
glob_append(SLANG_FIDDLE_LUA_INPUTS "*.lua")

list(
    TRANSFORM SLANG_FIDDLE_INPUT_FILE_NAMES
    APPEND ".fiddle"
    OUTPUT_VARIABLE SLANG_FIDDLE_OUTPUTS
)
list(TRANSFORM SLANG_FIDDLE_OUTPUTS PREPEND "${SLANG_FIDDLE_OUTPUT_DIR}/")

add_custom_command(
    OUTPUT ${SLANG_FIDDLE_OUTPUTS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SLANG_FIDDLE_OUTPUT_DIR}
    COMMAND
        slang-fiddle -i "${SLANG_FIDDLE_INPUT_DIR}/" -o
        "${SLANG_FIDDLE_OUTPUT_DIR}/" ${SLANG_FIDDLE_INPUT_FILE_NAMES}
    DEPENDS ${SLANG_FIDDLE_INPUTS} ${SLANG_FIDDLE_LUA_INPUTS} slang-fiddle
    WORKING_DIRECTORY ${slang_SOURCE_DIR}
    VERBATIM
)
add_library(
    slang-fiddle-output
    INTERFACE
    EXCLUDE_FROM_ALL
    ${SLANG_FIDDLE_OUTPUTS}
)
set_target_properties(slang-fiddle-output PROPERTIES FOLDER generated)
target_include_directories(
    slang-fiddle-output
    INTERFACE ${SLANG_FIDDLE_OUTPUT_DIR}
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
# generated lookup tables
#

get_target_property(
    SLANG_SPIRV_HEADERS_INCLUDE_DIR
    SPIRV-Headers::SPIRV-Headers
    INTERFACE_INCLUDE_DIRECTORIES
)

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
    LINK_WITH_PRIVATE core SPIRV-Headers::SPIRV-Headers
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
    ${CMAKE_CURRENT_BINARY_DIR}/../standard-modules/slang-standard-module-config-header
    EXPORT_MACRO_PREFIX
    SLANG
    EXPORT_TYPE_AS
    ${SLANG_LIB_TYPE}
    EXTRA_SOURCE_DIRS
    ${SLANG_RECORD_REPLAY_SYSTEM}
    REQUIRES
    copy_slang_headers
    generate_standard_module_config_header
)
set(slang_link_args
    LINK_WITH_PRIVATE
    core
    prelude
    compiler-core
    slang-capability-defs
    slang-capability-lookup
    slang-fiddle-output
    slang-lookup-tables
    SPIRV-Headers::SPIRV-Headers
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
        OUTPUT_NAME slang-compiler
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
        OUTPUT_NAME slang-compiler
        EXPORT_MACRO_PREFIX SLANG
        ${slang_link_args}
        ${slang_interface_args}
        ${slang_public_lib_args}
        LINK_WITH_PRIVATE slang-common-objects
        NO_SOURCE
    )
endif()

set_target_properties(slang PROPERTIES
    VERSION 0.${SLANG_VERSION_NUMERIC}
    SOVERSION 0.${SLANG_VERSION_NUMERIC}
    MACHO_COMPATIBILITY_VERSION 0.0.0
    MACHO_CURRENT_VERSION ${SLANG_VERSION_NUMERIC}
)

#
# Windows proxy DLL for backward compatibility
# Creates slang.dll that forwards all calls to slang-compiler.dll
#
if(WIN32 AND SLANG_LIB_TYPE STREQUAL "SHARED")
    # Create a minimal C++ source file (some linkers require at least one source)
    set(SLANG_PROXY_STUB_CPP "${CMAKE_CURRENT_BINARY_DIR}/slang-proxy-stub.cpp")
    file(WRITE "${SLANG_PROXY_STUB_CPP}" 
        "// Minimal stub file for slang.dll proxy\n"
        "// All exports are forwarded to slang-compiler.dll via the .def file\n"
    )

    # Generate the DEF file at build time from slang-compiler.dll
    set(SLANG_PROXY_DEF "${CMAKE_CURRENT_BINARY_DIR}/slang-proxy.def")

    # Create a wrapper script with the actual DLL path (since generator expressions
    # don't work with -P script mode). Use per-config wrapper to avoid file(GENERATE) conflicts.
    set(DEF_GEN_WRAPPER "${CMAKE_CURRENT_BINARY_DIR}/generate-def-wrapper-$<CONFIG>.cmake")
    file(GENERATE OUTPUT "${DEF_GEN_WRAPPER}" CONTENT "
set(DLL_PATH \"$<TARGET_FILE:slang>\")
set(OUTPUT_DEF \"${SLANG_PROXY_DEF}\")
set(TARGET_DLL_NAME \"slang-compiler\")
include(\"${slang_SOURCE_DIR}/cmake/RegenerateDllExports.cmake\")
")
    
    # Custom command to generate the DEF file from slang-compiler.dll exports
    add_custom_command(
        OUTPUT "${SLANG_PROXY_DEF}"
        COMMAND ${CMAKE_COMMAND} -P "${DEF_GEN_WRAPPER}"
        DEPENDS slang "$<TARGET_FILE:slang>"
        COMMENT "Generating slang-proxy.def from slang-compiler.dll exports"
        VERBATIM
    )
    
    # Create the proxy DLL target
    add_library(slang-proxy SHARED
        "${SLANG_PROXY_STUB_CPP}"
        "${SLANG_PROXY_DEF}"
    )
    
    # Link against the slang-compiler import library so symbols can be forwarded
    target_link_libraries(slang-proxy PRIVATE slang)
    
    # Get the output directories from the main slang target to match build locations
    get_target_property(slang_runtime_dir slang RUNTIME_OUTPUT_DIRECTORY)
    get_target_property(slang_archive_dir slang ARCHIVE_OUTPUT_DIRECTORY)
    
    # Set output name to slang (produces slang.dll) and match directories
    set_target_properties(slang-proxy PROPERTIES
        OUTPUT_NAME slang
        RUNTIME_OUTPUT_DIRECTORY "${slang_runtime_dir}"
        ARCHIVE_OUTPUT_DIRECTORY "${slang_archive_dir}"
    )
    
    # Install the proxy DLL alongside slang-compiler.dll
    install(
        TARGETS slang-proxy
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )
    
    message(STATUS "Windows proxy DLL (slang.dll) will be generated to forward to slang-compiler.dll")
endif()

#
# Unix backward compatibility symlink (libslang -> libslang-compiler)
#
if(UNIX AND SLANG_LIB_TYPE STREQUAL "SHARED")
    # Create symlink in build directory
    add_custom_command(
        TARGET slang POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            $<TARGET_FILE_NAME:slang>
            $<TARGET_FILE_DIR:slang>/libslang.$<IF:$<PLATFORM_ID:Darwin>,dylib,so>
        COMMENT "Creating backward compatibility symlink: libslang -> libslang-compiler"
        VERBATIM
    )

    # Install the symlink by copying it from the build directory
    install(
        FILES $<TARGET_FILE_DIR:slang>/libslang.$<IF:$<PLATFORM_ID:Darwin>,dylib,so>
        DESTINATION lib
    )

    message(STATUS "Unix backward compatibility symlink (libslang -> libslang-compiler) will be created during build and installation")
endif()
