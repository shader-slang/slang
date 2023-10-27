cmake_minimum_required(VERSION 3.17)

project(slang VERSION 2023.4.0 LANGUAGES C CXX)

#
# Our utils
#

set(SLANG_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(CompilerFlags)
include(Glob)
include(SlangTarget)

#
# Options
#

option(SLANG_ENABLE_CUDA "Enable CUDA tests using CUDA found in CUDA_PATH")
option(SLANG_ENABLE_OPTIX "Enable OptiX build/tests, requires enable-cuda")
set(SLANG_OPTIX_SDK_PATH CACHE STRING "Path to the OptiX SDK")
option(SLANG_ENABLE_NVAPI "Use NVAPI as found in external/nvapi")
option(
    SLANG_ENABLE_XLIB
    "Build gfx and platform with Xlib to support windowed apps on Linux"
    ON
)
option(
    SLANG_ENABLE_AFTERMATH
    "Enable Aftermath in GFX, and add aftermath crash example to project"
)
option(SLANG_ENABLE_DX_ON_VK "Use dxvk and vkd3d-proton for DirectX support")

option(SLANG_EMBED_STDLIB "Build slang with an embedded version of the stdlib")
option(SLANG_EMBED_STDLIB_SOURCE "Do not include stdlib source in the binary")

option(SLANG_ENABLE_FULL_IR_VALIDATION "Enable full IR validation (SLOW!)")
option(SLANG_ENABLE_ASAN "Enable ASAN (address sanitizer)")

#
# Dependencies
#

find_package(Threads REQUIRED)

find_package(X11 REQUIRED)

if(SLANG_ENABLE_CUDA)
    find_package(CUDAToolkit REQUIRED)
endif()

if(SLANG_ENABLE_OPTIX)
    find_path(
        OPTIX_INCLUDE
        optix.h
        REQUIRED
        PATHS ${SLANG_OPTIX_SDK_PATH}/include
    )
endif()

add_subdirectory(external)

#
# Our targets
#

slang_add_target(
    source/core
    STATIC
    EXCLUDE_FROM_ALL
    USE_EXTRA_WARNINGS
    LINK_WITH_PRIVATE miniz lz4_static
    INCLUDE_DIRECTORIES_PUBLIC source
)

slang_add_target(
    source/slang-rt
    SHARED
    EXTRA_SOURCE_DIRS source/core
    LINK_WITH_PRIVATE miniz lz4_static Threads::Threads
    EXPORT_MACRO_PREFIX SLANG_RT
)

slang_add_target(
    source/compiler-core
    STATIC
    EXCLUDE_FROM_ALL
    USE_EXTRA_WARNINGS
    LINK_WITH_PRIVATE core
    INCLUDE_DIRECTORIES_PUBLIC source
)
if(NOT MSVC)
    # This is necessary to compile the DXC headers
    set_source_files_properties(
        source/compiler-core/slang-dxc-compiler.cpp
        PROPERTIES COMPILE_OPTIONS "-fms-extensions"
    )
endif()

#
# Tools used to generate source during the build:
#

slang_add_target(
    tools/slang-cpp-extractor
    EXECUTABLE
    EXCLUDE_FROM_ALL
    GENERATOR_TOOL
    LINK_WITH_PRIVATE core compiler-core
)
slang_add_target(
    tools/slang-embed
    EXECUTABLE
    EXCLUDE_FROM_ALL
    GENERATOR_TOOL
    LINK_WITH_PRIVATE core
)
slang_add_target(
    tools/slang-generate
    EXECUTABLE
    EXCLUDE_FROM_ALL
    GENERATOR_TOOL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core
)
slang_add_target(
    tools/slang-lookup-generator
    EXECUTABLE
    EXCLUDE_FROM_ALL
    GENERATOR_TOOL
    LINK_WITH_PRIVATE core compiler-core
)
slang_add_target(
    tools/slang-spirv-embed-generator
    EXECUTABLE
    EXCLUDE_FROM_ALL
    GENERATOR_TOOL
    LINK_WITH_PRIVATE core compiler-core
)
slang_add_target(
    source/slangc
    EXECUTABLE
    USE_FEWER_WARNINGS
    GENERATOR_TOOL
    TARGET_NAME slang-bootstrap
    EXCLUDE_FROM_ALL
    LINK_WITH_PRIVATE core slang-static Threads::Threads
)

#
# The compiler itself
#

add_subdirectory(prelude)
add_subdirectory(source/slang)
slang_add_target(tools/slangd EXECUTABLE LINK_WITH_PRIVATE core slang)
slang_add_target(
    source/slangc
    EXECUTABLE
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core slang Threads::Threads
)

#
# `platform` contains all the platform abstractions for a GUI application.
#
slang_add_target(
    tools/platform
    SHARED
    EXCLUDE_FROM_ALL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core $<$<BOOL:${SLANG_ENABLE_XLIB}>:X11::X11>
    INCLUDE_FROM_PRIVATE gfx
    INCLUDE_DIRECTORIES_PUBLIC tools/platform
    EXPORT_MACRO_PREFIX SLANG_PLATFORM
)

#
# GFX
#

slang_add_target(
    tools/gfx
    SHARED
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE
        core
        slang
        Vulkan-Headers
        $<$<BOOL:${SLANG_ENABLE_XLIB}>:X11::X11>
    # This is a shared library, so we need to set a preprocessor macro to mark
    # exported symbols
    EXPORT_MACRO_PREFIX SLANG_GFX
    # slang-gfx is in this directory, anything which depends on gfx should include
    # this
    INCLUDE_DIRECTORIES_PUBLIC .
)

slang_add_target(
    tools/gfx-util
    STATIC
    LINK_WITH_PRIVATE core
    INCLUDE_FROM_PRIVATE gfx
)

#
# The test executables and runtime-loaded modules
#

slang_add_target(
    tools/slang-test
    EXECUTABLE
    USE_FEWER_WARNINGS
    TEST_TOOL
    LINK_WITH_PRIVATE core compiler-core slang ${CMAKE_DL_LIBS} Threads::Threads
    MODULE_DEPENDS
        slang-reflection-test
        render-test
        slang-unit-test
        gfx-unit-test
)
slang_add_target(
    tools/test-server
    EXECUTABLE
    EXCLUDE_FROM_ALL
    TEST_TOOL
    LINK_WITH_PRIVATE core compiler-core slang
)
slang_add_target(
    tools/test-process
    EXECUTABLE
    EXCLUDE_FROM_ALL
    TEST_TOOL
    LINK_WITH_PRIVATE core compiler-core
)
slang_add_target(
    tools/slang-profile
    EXECUTABLE
    EXCLUDE_FROM_ALL
    TEST_TOOL
    LINK_WITH_PRIVATE slang-static
)

slang_add_target(
    tools/unit-test
    OBJECT
    EXCLUDE_FROM_ALL
    TEST_TOOL
    INCLUDE_FROM_PRIVATE slang
)

# These are libraries loaded at runtime from the test executable:
slang_add_target(
    tools/gfx-unit-test
    MODULE
    EXCLUDE_FROM_ALL
    SHARED_LIBRARY_TOOL
    TEST_TOOL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core slang unit-test gfx gfx-util platform
    OUTPUT_NAME gfx-unit-test-tool
)
slang_add_target(
    tools/slang-unit-test
    MODULE
    EXCLUDE_FROM_ALL
    SHARED_LIBRARY_TOOL
    TEST_TOOL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core compiler-core unit-test slang Threads::Threads
    OUTPUT_NAME slang-unit-test-tool
)
slang_add_target(
    tools/slang-reflection-test
    MODULE
    EXCLUDE_FROM_ALL
    SHARED_LIBRARY_TOOL
    TEST_TOOL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core slang
    OUTPUT_NAME slang-reflection-test-tool
)
slang_add_target(
    tools/render-test
    MODULE
    EXCLUDE_FROM_ALL
    SHARED_LIBRARY_TOOL
    TEST_TOOL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core compiler-core slang gfx gfx-util platform
    OUTPUT_NAME render-test-tool
)
