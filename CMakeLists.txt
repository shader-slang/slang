cmake_minimum_required(VERSION 3.20)

project(slang VERSION 2023.4.0 LANGUAGES C CXX)

#
# Global CMake options, generally to be avoided
#

# Don't use absolute paths to the build tree in RPATH
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
# Our module dir
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Squash CMake dev warnings
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

#
# CMake-supplied modules and our utils
#

include(CMakeDependentOption)
include(FetchContent)

include(CompilerFlags)
include(Glob)
include(LLVM)
include(SlangTarget)
include(AutoOption)

#
# Options
#

auto_option(
    SLANG_ENABLE_CUDA
    CUDAToolkit
    "Enable CUDA tests using CUDA found in CUDA_PATH"
)
auto_option(
    SLANG_ENABLE_OPTIX
    OptiX
    "Enable OptiX build/tests, requires enable-cuda"
)
auto_option(
    SLANG_ENABLE_NVAPI
    NVAPI
    "Enable NVAPI usage (Only available for builds targeting Windows)"
)
auto_option(
    SLANG_ENABLE_XLIB
    X11
    "Build gfx and platform with Xlib to support windowed apps on Linux"
)
auto_option(
    SLANG_ENABLE_AFTERMATH
    Aftermath
    "Enable Aftermath in GFX, and add aftermath crash example to project"
)
option(SLANG_ENABLE_DX_ON_VK "Use dxvk and vkd3d-proton for DirectX support")

option(SLANG_EMBED_STDLIB_SOURCE "Embed stdlib source in the binary" ON)
option(SLANG_EMBED_STDLIB "Build slang with an embedded version of the stdlib")

option(SLANG_ENABLE_FULL_IR_VALIDATION "Enable full IR validation (SLOW!)")
option(SLANG_ENABLE_ASAN "Enable ASAN (address sanitizer)")

option(
    SLANG_USE_BINARY_SLANG_LLVM
    "Use a binary distribution of the slang-llvm library instead of building or using LLVM"
)
# If we're not using a binary slang-llvm distribution, add an option for
# building with the system's LLVM
cmake_dependent_option(
    SLANG_USE_SYSTEM_LLVM
    "Use a system-provided LLVM and Clang distribution"
    ON
    "NOT SLANG_USE_BINARY_SLANG_LLVM"
    OFF
)

#
# Option validation
#

if(NOT SLANG_EMBED_STDLIB AND NOT SLANG_EMBED_STDLIB_SOURCE)
    message(
        SEND_ERROR
        "One of SLANG_EMBED_STDLIB and SLANG_EMBED_STDLIB_SOURCE must be enabled"
    )
endif()

if(SLANG_ENABLE_OPTIX AND NOT SLANG_ENABLE_CUDA)
    message(
        SEND_ERROR
        "SLANG_ENABLE_OPTIX is not supported without SLANG_ENABLE_CUDA"
    )
endif()

#
# Dependencies, most of these are however handled inside the "auto_option"
# calls above
#

find_package(Threads REQUIRED)

if(NOT SLANG_USE_BINARY_SLANG_LLVM)
    find_llvm()
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
    # This compiles 'core' again with the SLANG_RT_DYNAMIC_EXPORT macro defined
    EXTRA_SOURCE_DIRS source/core
    USE_EXTRA_WARNINGS
    LINK_WITH_PRIVATE miniz lz4_static Threads::Threads
    EXPORT_MACRO_PREFIX SLANG_RT
    INSTALL
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
        DIRECTORY ${slang_SOURCE_DIR}
    )
endif()

#
# Tools used to generate source during the build:
#

add_custom_target(
    generators
    COMMENT "meta target which depends on all generators"
)
slang_add_target(
    tools/slang-cpp-extractor
    EXECUTABLE
    EXCLUDE_FROM_ALL
    USE_FEWER_WARNINGS
    OUTPUT_DIR generators
    LINK_WITH_PRIVATE core compiler-core
    REQUIRED_BY generators
)
slang_add_target(
    tools/slang-embed
    EXECUTABLE
    EXCLUDE_FROM_ALL
    OUTPUT_DIR generators
    LINK_WITH_PRIVATE core
    REQUIRED_BY generators
)
slang_add_target(
    tools/slang-generate
    EXECUTABLE
    EXCLUDE_FROM_ALL
    OUTPUT_DIR generators
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core
    REQUIRED_BY generators
)
slang_add_target(
    tools/slang-lookup-generator
    EXECUTABLE
    EXCLUDE_FROM_ALL
    OUTPUT_DIR generators
    LINK_WITH_PRIVATE core compiler-core
    REQUIRED_BY generators
)
slang_add_target(
    tools/slang-spirv-embed-generator
    EXECUTABLE
    EXCLUDE_FROM_ALL
    OUTPUT_DIR generators
    LINK_WITH_PRIVATE core compiler-core
    REQUIRED_BY generators
)
slang_add_target(
    source/slangc
    EXECUTABLE
    USE_FEWER_WARNINGS
    OUTPUT_DIR generators
    TARGET_NAME slang-bootstrap
    EXCLUDE_FROM_ALL
    LINK_WITH_PRIVATE core slang-no-embedded-stdlib Threads::Threads
    REQUIRED_BY generators
)

#
# The compiler itself
#

# keep these non-trivial targets in their own directories so as not to clutter
# this file
add_subdirectory(prelude)
add_subdirectory(source/slang)

slang_add_target(
    tools/slangd
    EXECUTABLE
    LINK_WITH_PRIVATE core slang slang-reflect-headers
    INSTALL
)
slang_add_target(
    source/slangc
    EXECUTABLE
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core slang Threads::Threads
    INSTALL
)

#
# Our wrappers for glslang and llvm
#
slang_add_target(
    source/slang-glslang
    MODULE
    USE_EXTRA_WARNINGS
    LINK_WITH_PRIVATE glslang SPIRV SPIRV-Tools-opt
    INSTALL
)

if(SLANG_USE_BINARY_SLANG_LLVM)
    #
    # Do some stupid little dance to put everything in the right shape with
    # correct dependencies
    #

    # Download the binary
    # TODO: correct versioning and platform handling
    fetchcontent_declare(
        slang-llvm
        URL
            https://github.com/shader-slang/slang-llvm/releases/download/v13.x-42/slang-llvm-v13.x-42-linux-x86_64-release.zip
    )
    fetchcontent_populate(slang-llvm)

    # Copy it to the lib/ directory
    set(slang_llvm_source_object
        "${slang-llvm_SOURCE_DIR}/bin/linux-x64/release/libslang-llvm.so"
    )
    set(slang_llvm_object ${CMAKE_BINARY_DIR}/$<CONFIG>/lib/libslang-llvm.so)
    add_custom_command(
        OUTPUT ${slang_llvm_object}
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${slang_llvm_source_object}
            ${slang_llvm_object}
        DEPENDS ${slang_llvm_source_object}
        VERBATIM
    )
    # Give this copying action a name
    add_custom_target(copy-slang-llvm DEPENDS ${slang_llvm_object})

    # Put this into a library
    add_library(slang-llvm MODULE IMPORTED GLOBAL)
    add_dependencies(slang-llvm copy-slang-llvm)
    set_property(
        TARGET slang-llvm
        PROPERTY IMPORTED_LOCATION ${slang_llvm_object}
    )
    install(PROGRAMS ${slang_llvm_object} DESTINATION module_subdir)
else()
    llvm_target_from_components(llvm-dep filecheck native orcjit)
    clang_target_from_libs(
        clang-dep
        clangBasic
        clangCodeGen
        clangDriver
        clangLex
        clangFrontend
        clangFrontendTool
    )
    slang_add_target(
        source/slang-llvm
        MODULE
        LINK_WITH_PRIVATE core compiler-core llvm-dep clang-dep
        # We include slang.h, but don't need to link with it
        INCLUDE_FROM_PRIVATE slang
        # This uses the SLANG_DLL_EXPORT macro from slang.h, so make sure to set
        # SLANG_DYNAMIC and SLANG_DYNAMIC_EXPORT
        EXPORT_MACRO_PREFIX SLANG
        INSTALL
    )
    # If we don't include this, then the symbols in the LLVM linked here may
    # conflict with those of other LLVMs linked at runtime, for instance in mesa.
    add_supported_cxx_linker_flags(slang-llvm PRIVATE "-Wl,--exclude-libs,all")
endif()

#
# `platform` contains all the platform abstractions for a GUI application.
#
slang_add_target(
    tools/platform
    SHARED
    EXCLUDE_FROM_ALL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core $<$<BOOL:${SLANG_ENABLE_XLIB}>:X11::X11>
    EXTRA_COMPILE_DEFINITIONS_PRIVATE
        $<$<BOOL:${SLANG_ENABLE_XLIB}>:SLANG_ENABLE_XLIB>
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
        $<$<BOOL:${SLANG_ENABLE_CUDA}>:CUDA::cudart;CUDA::cuda_driver>
    EXTRA_COMPILE_DEFINITIONS_PRIVATE
        $<$<BOOL:${SLANG_ENABLE_CUDA}>:GFX_ENABLE_CUDA>
        $<$<BOOL:${SLANG_ENABLE_OPTIX}>:GFX_OPTIX>
        $<$<BOOL:${SLANG_ENABLE_NVAPI}>:GFX_NVAPI>
        $<$<BOOL:${SLANG_ENABLE_XLIB}>:SLANG_ENABLE_XLIB>
    # This is a shared library, so we need to set a preprocessor macro to mark
    # exported symbols
    EXPORT_MACRO_PREFIX SLANG_GFX
    # slang-gfx is in this directory, anything which depends on gfx should include
    # this
    INCLUDE_DIRECTORIES_PUBLIC .
    REQUIRES copy-gfx-slang-modules
    INSTALL
)
set(modules_dest_dir $<TARGET_FILE_DIR:slang-test>)
add_custom_target(
    copy-gfx-slang-modules
    COMMAND ${CMAKE_COMMAND} -E copy tools/gfx/gfx.slang ${modules_dest_dir}
    COMMAND ${CMAKE_COMMAND} -E copy tools/gfx/slang.slang ${modules_dest_dir}
    WORKING_DIRECTORY ${slang_SOURCE_DIR}
    VERBATIM
)
install(
    FILES ${modules_dest_dir}/gfx.slang ${modules_dest_dir}/slang.slang
    DESTINATION bin
)

slang_add_target(
    tools/gfx-util
    STATIC
    LINK_WITH_PRIVATE core
    INCLUDE_FROM_PRIVATE gfx
    # The headers are included with 'include "gfx-util/blah.h"' which is found
    # in the tools directory
    INCLUDE_DIRECTORIES_PUBLIC tools
)

#
# The test executables and runtime-loaded modules
#

slang_add_target(
    tools/slang-test
    EXECUTABLE
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core compiler-core slang ${CMAKE_DL_LIBS} Threads::Threads
    REQUIRES
        # Shared libraries dlopened by slang-test
        slang-reflection-test
        render-test
        slang-unit-test
        gfx-unit-test
        slang-llvm
        slang-glslang
        # Used by some tests when they run
        slangd
        slang-rt
        gfx
)
slang_add_target(
    tools/test-server
    EXECUTABLE
    EXCLUDE_FROM_ALL
    LINK_WITH_PRIVATE core compiler-core slang
)
slang_add_target(
    tools/test-process
    EXECUTABLE
    EXCLUDE_FROM_ALL
    LINK_WITH_PRIVATE core compiler-core
)
slang_add_target(
    tools/slang-profile
    EXECUTABLE
    EXCLUDE_FROM_ALL
    LINK_WITH_PRIVATE core slang
)

slang_add_target(
    tools/unit-test
    OBJECT
    EXCLUDE_FROM_ALL
    INCLUDE_FROM_PRIVATE slang
)

# These are libraries loaded at runtime from the test executable:
slang_add_target(
    tools/gfx-unit-test
    MODULE
    EXCLUDE_FROM_ALL
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_SHARED_LIBRARY_TOOL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core slang unit-test gfx gfx-util platform
    OUTPUT_NAME gfx-unit-test-tool
)
slang_add_target(
    tools/slang-unit-test
    MODULE
    EXCLUDE_FROM_ALL
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_SHARED_LIBRARY_TOOL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core compiler-core unit-test slang Threads::Threads
    OUTPUT_NAME slang-unit-test-tool
)
slang_add_target(
    tools/slang-reflection-test
    MODULE
    EXCLUDE_FROM_ALL
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_SHARED_LIBRARY_TOOL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE core slang
    OUTPUT_NAME slang-reflection-test-tool
)
slang_add_target(
    tools/render-test
    MODULE
    EXCLUDE_FROM_ALL
    EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_SHARED_LIBRARY_TOOL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE
        core
        compiler-core
        slang
        gfx
        gfx-util
        platform
        $<$<BOOL:${SLANG_ENABLE_CUDA}>:CUDA::cudart;CUDA::cuda_driver>
    EXTRA_COMPILE_DEFINITIONS_PRIVATE
        $<$<BOOL:${SLANG_ENABLE_CUDA}>:RENDER_TEST_CUDA>
        $<$<BOOL:${SLANG_ENABLE_OPTIX}>:RENDER_TEST_OPTIX>
    OUTPUT_NAME render-test-tool
)

#
# Examples
#

slang_add_target(
    examples/example-base
    STATIC
    EXCLUDE_FROM_ALL
    LINK_WITH_PRIVATE
        core
        slang
        gfx
        platform
        $<$<BOOL:${SLANG_ENABLE_CUDA}>:CUDA::cudart;CUDA::cuda_driver>
)

add_custom_target(examples COMMENT "meta target which depends on all examples")
function(example dir)
    slang_add_target(
        ${dir}
        EXECUTABLE
        ${ARGN}
        EXCLUDE_FROM_ALL
        USE_FEWER_WARNINGS
        LINK_WITH_PRIVATE
            core
            example-base
            slang
            gfx
            gfx-util
            platform
            $<$<BOOL:${SLANG_ENABLE_CUDA}>:CUDA::cudart;CUDA::cuda_driver>
        EXTRA_COMPILE_DEFINITIONS_PRIVATE
            $<$<BOOL:${SLANG_ENABLE_XLIB}>:SLANG_ENABLE_XLIB>
        REQUIRED_BY examples
    )
endfunction()

example(examples/autodiff-texture WIN32_EXECUTABLE)
example(examples/cpu-com-example )
example(examples/cpu-hello-world )
example(examples/gpu-printing )
example(examples/hello-world LINK_WITH_PRIVATE Vulkan-Headers)
example(examples/model-viewer WIN32_EXECUTABLE)
example(examples/ray-tracing WIN32_EXECUTABLE)
example(examples/ray-tracing-pipeline WIN32_EXECUTABLE)
example(examples/shader-object )
example(examples/shader-toy WIN32_EXECUTABLE)
example(examples/triangle WIN32_EXECUTABLE)
if(SLANG_ENABLE_AFTERMATH)
    example(examples/nv-aftermath-example WIN32_EXECUTABLE)
endif()
