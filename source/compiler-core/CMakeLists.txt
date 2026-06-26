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

# slang-glslang-compiler.cpp defaults SLANG_ENABLE_GLSLANG_SUPPORT to 1 when the build does not
# define it. Define it to 0 (compiling out the in-process registration of the slang-glslang
# downstream compiler and the include of its header) when registration is explicitly disabled,
# or when the slang-glslang module itself is not being built -- registering a compiler that
# dlopen()s a module that was never produced is exactly the missing-module failure (#11662) we
# want to avoid. So a `-DSLANG_ENABLE_SLANG_GLSLANG=OFF` build drops both the module and its
# registration, leaving compiler-core with no reference to or dependency on slang-glslang.
if(NOT SLANG_ENABLE_GLSLANG_SUPPORT OR NOT SLANG_ENABLE_SLANG_GLSLANG)
    target_compile_definitions(compiler-core PRIVATE SLANG_ENABLE_GLSLANG_SUPPORT=0)
endif()
