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
)
if(NOT MSVC)
    # This is necessary to compile the DXC headers
    set_source_files_properties(
        slang-dxc-compiler.cpp
        PROPERTIES COMPILE_OPTIONS "-fms-extensions"
        DIRECTORY ${slang_SOURCE_DIR}
    )
endif()
