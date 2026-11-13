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

# The lexer consumes a checked-in copy of its generated kind definitions because `slang-fiddle`
# itself links `compiler-core`. These explicit maintenance targets run after fiddle is built;
# making either target a prerequisite of `compiler-core` would introduce a dependency cycle.
set(SLANG_LEXEME_DEFINITION_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/slang-lexeme-defs.h")
set(SLANG_LEXEME_DEFINITION_LUA "${CMAKE_CURRENT_SOURCE_DIR}/slang-lexeme-defs.lua")
set(SLANG_LEXEME_DEFINITION_OUTPUT "${SLANG_LEXEME_DEFINITION_HEADER}.fiddle")

add_custom_target(
    regenerate-lexeme-definitions
    COMMAND
        $<TARGET_FILE:slang-fiddle> -i "${CMAKE_CURRENT_SOURCE_DIR}/" -o
        "${CMAKE_CURRENT_SOURCE_DIR}/" slang-lexeme-defs.h
    DEPENDS
        slang-fiddle
        ${SLANG_LEXEME_DEFINITION_HEADER}
        ${SLANG_LEXEME_DEFINITION_LUA}
    WORKING_DIRECTORY ${slang_SOURCE_DIR}
    COMMENT "Regenerating the checked-in lexeme definitions"
    VERBATIM
)

# Fiddle can rewrite its input to update template markers. We give the check a build-only
# input copy so checking a stale snapshot never edits the checkout being validated.
set(SLANG_LEXEME_CHECK_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lexeme-definitions-check/$<CONFIG>")
add_custom_target(
    check-lexeme-definitions
    COMMAND
        ${CMAKE_COMMAND} -E make_directory
        "${SLANG_LEXEME_CHECK_DIRECTORY}/input"
        "${SLANG_LEXEME_CHECK_DIRECTORY}/output"
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        ${SLANG_LEXEME_DEFINITION_HEADER}
        "${SLANG_LEXEME_CHECK_DIRECTORY}/input/slang-lexeme-defs.h"
    COMMAND
        $<TARGET_FILE:slang-fiddle> -i "${SLANG_LEXEME_CHECK_DIRECTORY}/input/" -o
        "${SLANG_LEXEME_CHECK_DIRECTORY}/output/" slang-lexeme-defs.h
    COMMAND
        ${CMAKE_COMMAND} -E compare_files
        ${SLANG_LEXEME_DEFINITION_OUTPUT}
        "${SLANG_LEXEME_CHECK_DIRECTORY}/output/slang-lexeme-defs.h.fiddle"
    DEPENDS
        slang-fiddle
        ${SLANG_LEXEME_DEFINITION_HEADER}
        ${SLANG_LEXEME_DEFINITION_LUA}
        ${SLANG_LEXEME_DEFINITION_OUTPUT}
    WORKING_DIRECTORY ${slang_SOURCE_DIR}
    COMMENT "Checking the checked-in lexeme definitions"
    VERBATIM
)
set_target_properties(
    regenerate-lexeme-definitions check-lexeme-definitions
    PROPERTIES FOLDER generated
)
