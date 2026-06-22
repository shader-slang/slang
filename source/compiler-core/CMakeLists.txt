slang_add_target(
    .
    STATIC
    EXPORT_MACRO_PREFIX SLANG
    EXCLUDE_FROM_ALL
    USE_EXTRA_WARNINGS
    LINK_WITH_PRIVATE core
    INCLUDE_FROM_PUBLIC SPIRV-Headers::SPIRV-Headers
    INCLUDE_DIRECTORIES_PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/../slang/slang-version-header
    PRECOMPILE_HEADERS
        ${slang_SOURCE_DIR}/source/core/slang-basic.h
)
if(NOT MSVC)
    # This is necessary to compile the DXC headers.
    # Also skip PCH for this file since -fms-extensions conflicts with PCH.
    set_source_files_properties(
        slang-dxc-compiler.cpp
        PROPERTIES
            COMPILE_OPTIONS "-fms-extensions"
            SKIP_PRECOMPILE_HEADERS ON
        DIRECTORY ${slang_SOURCE_DIR}
    )
endif()

# On the Linux STATIC path, slang-glslang-compiler.cpp binds the wrapper's 7 entry
# points in-process (SLANG_STATIC_SLANG_GLSLANG) instead of dlopen'ing them, so it has
# hard references to those symbols. Link the symbol-localized static archive (built in
# source/slang-glslang) PUBLICly so every consumer of compiler-core — slang,
# slang-bootstrap, and the various tools — resolves them transitively.
if(
    SLANG_LIB_TYPE STREQUAL "STATIC"
    AND CMAKE_SYSTEM_NAME STREQUAL "Linux"
    AND SLANG_ENABLE_SLANG_GLSLANG
)
    target_link_libraries(compiler-core PUBLIC slang-glslang-static)
endif()
