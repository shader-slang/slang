slang_add_target(
    .
    STATIC
    EXPORT_MACRO_PREFIX SLANG
    EXCLUDE_FROM_ALL
    USE_EXTRA_WARNINGS
    LINK_WITH_PRIVATE core fast_float
    INCLUDE_FROM_PUBLIC SPIRV-Headers::SPIRV-Headers
    INCLUDE_DIRECTORIES_PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/../slang/slang-version-header
        ${slang_SOURCE_DIR}/external
    PRECOMPILE_HEADERS
        ${slang_SOURCE_DIR}/source/core/slang-basic.h
)
if(NOT MSVC)
    # This is necessary to compile the DXC headers.
    # Also skip PCH for this file since -fms-extensions conflicts with PCH.
    set_source_files_properties(
        slang-dxc-compiler.cpp
        PROPERTIES COMPILE_OPTIONS "-fms-extensions" SKIP_PRECOMPILE_HEADERS ON
        DIRECTORY ${slang_SOURCE_DIR}
    )
endif()

# We compile the checked-in kind and type definitions because `slang-fiddle` links
# `compiler-core`. The maintenance targets run after `slang-fiddle` is built; making either
# target a prerequisite of `compiler-core` would introduce a dependency cycle.
set(
    SLANG_LEXEME_DEFINITION_FILES
    slang-lexeme-defs.h
    slang-raw-lexeme-types.h
    slang-token-types.h
)
list(
    TRANSFORM SLANG_LEXEME_DEFINITION_FILES
    PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/"
    OUTPUT_VARIABLE SLANG_LEXEME_DEFINITION_HEADERS
)
list(
    TRANSFORM SLANG_LEXEME_DEFINITION_HEADERS
    APPEND ".fiddle"
    OUTPUT_VARIABLE SLANG_LEXEME_DEFINITION_OUTPUTS
)
set(SLANG_LEXEME_DEFINITION_LUA "${CMAKE_CURRENT_SOURCE_DIR}/lexeme-defs.lua")

add_custom_target(
    regenerate-lexeme-definitions
    COMMAND
        $<TARGET_FILE:slang-fiddle> -i "${CMAKE_CURRENT_SOURCE_DIR}/" -o
        "${CMAKE_CURRENT_SOURCE_DIR}/" ${SLANG_LEXEME_DEFINITION_FILES}
    DEPENDS
        slang-fiddle
        ${SLANG_LEXEME_DEFINITION_HEADERS}
        ${SLANG_LEXEME_DEFINITION_LUA}
    WORKING_DIRECTORY ${slang_SOURCE_DIR}
    COMMENT "Regenerating the checked-in lexeme definitions"
    VERBATIM
)

# Fiddle can rewrite its input to update template markers. We give the check input copies
# in the build directory so checking stale snapshots does not edit the checkout.
set(SLANG_LEXEME_CHECK_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lexeme-definitions-check/$<CONFIG>")
add_custom_target(
    check-lexeme-definitions
    COMMAND
        ${CMAKE_COMMAND} -E make_directory
        "${SLANG_LEXEME_CHECK_DIRECTORY}/input"
        "${SLANG_LEXEME_CHECK_DIRECTORY}/output"
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        ${SLANG_LEXEME_DEFINITION_HEADERS}
        ${SLANG_LEXEME_DEFINITION_LUA}
        "${SLANG_LEXEME_CHECK_DIRECTORY}/input/"
    COMMAND
        $<TARGET_FILE:slang-fiddle> -i "${SLANG_LEXEME_CHECK_DIRECTORY}/input/" -o
        "${SLANG_LEXEME_CHECK_DIRECTORY}/output/" ${SLANG_LEXEME_DEFINITION_FILES}
    COMMAND
        ${CMAKE_COMMAND} -E compare_files
        "${CMAKE_CURRENT_SOURCE_DIR}/slang-lexeme-defs.h.fiddle"
        "${SLANG_LEXEME_CHECK_DIRECTORY}/output/slang-lexeme-defs.h.fiddle"
    COMMAND
        ${CMAKE_COMMAND} -E compare_files
        "${CMAKE_CURRENT_SOURCE_DIR}/slang-raw-lexeme-types.h.fiddle"
        "${SLANG_LEXEME_CHECK_DIRECTORY}/output/slang-raw-lexeme-types.h.fiddle"
    COMMAND
        ${CMAKE_COMMAND} -E compare_files
        "${CMAKE_CURRENT_SOURCE_DIR}/slang-token-types.h.fiddle"
        "${SLANG_LEXEME_CHECK_DIRECTORY}/output/slang-token-types.h.fiddle"
    DEPENDS
        slang-fiddle
        ${SLANG_LEXEME_DEFINITION_HEADERS}
        ${SLANG_LEXEME_DEFINITION_LUA}
        ${SLANG_LEXEME_DEFINITION_OUTPUTS}
    WORKING_DIRECTORY ${slang_SOURCE_DIR}
    COMMENT "Checking the checked-in lexeme definitions"
    VERBATIM
)
set_target_properties(
    regenerate-lexeme-definitions check-lexeme-definitions
    PROPERTIES FOLDER generated
)
