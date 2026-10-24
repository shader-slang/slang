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

if(SLANG_ENABLE_VKD3D_SHADER)
    find_path(
        SLANG_VKD3D_INCLUDE_DIR
        vkd3d_shader.h
        PATH_SUFFIXES vkd3d
        REQUIRED
    )
    include(CheckCXXSourceCompiles)
    include(CMakePushCheckState)
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_INCLUDES ${SLANG_VKD3D_INCLUDE_DIR})
    unset(SLANG_VKD3D_HAS_REQUIRED_API CACHE)
    check_cxx_source_compiles(
        "#include <vkd3d_shader.h>
int main() { return VKD3D_SHADER_API_VERSION_2_1; }"
        SLANG_VKD3D_HAS_REQUIRED_API
    )
    cmake_pop_check_state()
    if(NOT SLANG_VKD3D_HAS_REQUIRED_API)
        message(
            FATAL_ERROR
            "SLANG_ENABLE_VKD3D_SHADER requires upstream vkd3d 2.1 or newer headers"
        )
    endif()
    target_include_directories(compiler-core PRIVATE ${SLANG_VKD3D_INCLUDE_DIR})
endif()
target_compile_definitions(
    compiler-core
    PRIVATE SLANG_ENABLE_VKD3D_SHADER=$<BOOL:${SLANG_ENABLE_VKD3D_SHADER}>
)
