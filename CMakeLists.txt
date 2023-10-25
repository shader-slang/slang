cmake_minimum_required(VERSION 3.17)

project(
  slang
  VERSION 2023.4.0
  LANGUAGES C CXX)

#
# Our utils
#

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(CompilerFlags)
set(SLANG_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#
# Dependencies
#

find_package(Threads REQUIRED)
add_subdirectory(external)

#
# Options
#

option(SLANG_ENABLE_CUDA "Enable CUDA tests using CUDA found in CUDA_PATH")
option(SLANG_ENABLE_OPTIX "Enable OptiX build/tests, requires enable-cuda")
set(SLANG_OPTIX_SDK_PATH CACHE STRING "Path to the OptiX SDK")
option(SLANG_ENABLE_NVAPI "Use NVAPI as found in external/nvapi")
option(SLANG_ENABLE_XLIB
       "Build gfx and platform with Xlib to support windowed apps on Linux")
option(SLANG_ENABLE_DX_ON_VK "Use dxvk and vkd3d-proton for DirectX support")
option(SLANG_ENABLE_AFTERMATH
       "Enable Aftermath in GFX, and add aftermath crash example to project")

option(SLANG_EMBED_STDLIB "Build slang with an embedded version of the stdlib")
option(SLANG_EMBED_STDLIB_SOURCE "Do not include stdlib source in the binary")

option(SLANG_ENABLE_FULL_IR_VALIDATION "Enable full IR validation (SLOW!)")
option(SLANG_ENABLE_ASAN "Enable ASAN (address sanitizer)")

#
# Our targets
#

add_subdirectory(source/core)
add_subdirectory(source/slang-rt)
add_subdirectory(source/compiler-core)
add_subdirectory(tools/slang-cpp-extractor)
add_subdirectory(tools/slang-embed)
add_subdirectory(tools/slang-generate)
add_subdirectory(tools/slang-lookup-generator)
add_subdirectory(tools/slang-spirv-embed-generator)
add_subdirectory(tools/test-process)
add_subdirectory(prelude)
add_subdirectory(source/slang)
add_subdirectory(source/slangc)
add_subdirectory(tools/slangd)
add_subdirectory(tools/platform)
add_subdirectory(tools/gfx)
add_subdirectory(tools/gfx-util)
add_subdirectory(tools/slang-test)
add_subdirectory(tools/unit-test)
add_subdirectory(tools/gfx-unit-test)
add_subdirectory(tools/slang-unit-test)
add_subdirectory(tools/slang-reflection-test)
add_subdirectory(tools/render-test)
add_subdirectory(tools/test-server)
add_subdirectory(tools/slang-profile)
