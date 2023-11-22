cmake_minimum_required(VERSION 3.20)

# Our module dir, include that now so that we can get the version automatically
# from git describe
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(GitVersion)
get_git_version(SLANG_VERSION "${CMAKE_CURRENT_LIST_DIR}")

#
# And our project
#
project(slang VERSION "${SLANG_VERSION}" LANGUAGES)

#
# Global CMake options
#

cmake_policy(SET CMP0135 OLD)
cmake_policy(SET CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# Don't use absolute paths to the build tree in RPATH, this makes the build
# tree relocatable
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

include(CCacheDebugInfoWorkaround)

#
# CMake-supplied modules and our utils
#

enable_language(C CXX)

include(CMakeDependentOption)
include(FetchContent)

include(CompilerFlags)
include(Glob)
include(LLVM)
include(SlangTarget)
include(AutoOption)
include(Nested)

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

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(slang_llvm_flavor_def FETCH_BINARY)
else()
    set(slang_llvm_flavor_def USE_SYSTEM_LLVM)
endif()
enum_option(SLANG_SLANG_LLVM_FLAVOR ${slang_llvm_flavor_def}
    "How to get or build slang-llvm:"
    FETCH_BINARY "Use a binary distribution of the slang-llvm library instead of building or using LLVM (default for Windows)"
    USE_SYSTEM_LLVM "Build slang-llvm using system-provided LLVM and Clang binaries (default for non-Windows hosts)"
    BUILD_LLVM "Immediately fetch and build a LLVM distribution suitable for compiling a redistributable slang-llvm"
    DISABLE "Do not build llvm or fetch slang-llvm"
)
macro(slang_llvm_binary_url_option version filename)
    set(SLANG_SLANG_LLVM_BINARY_URL
        "https://github.com/shader-slang/slang-llvm/releases/download/${version}/${filename}"
        CACHE STRING
        "URL specifying the location of the slang-llvm prebuilt library"
    )
endmacro()
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    slang_llvm_binary_url_option("v13.x-42" "slang-llvm-13.x-42-win64.zip")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    slang_llvm_binary_url_option("v13.x-42" "slang-llvm-v13.x-42-macosx-x86_64-release.zip")
else()
    slang_llvm_binary_url_option("v13.x-42" "slang-llvm-v13.x-42-linux-x86_64-release.zip")
endif()

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

if(SLANG_SLANG_LLVM_FLAVOR STREQUAL "BUILD_LLVM")
    build_llvm()
elseif(SLANG_SLANG_LLVM_FLAVOR STREQUAL "USE_SYSTEM_LLVM")
    find_package(LLVM 13.0 REQUIRED CONFIG)
    find_package(Clang REQUIRED CONFIG)
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
    LINK_WITH_PRIVATE miniz lz4_static Threads::Threads ${CMAKE_DL_LIBS}
    INCLUDE_DIRECTORIES_PUBLIC source
)

slang_add_target(
    source/slang-rt
    SHARED
    # This compiles 'core' again with the SLANG_RT_DYNAMIC_EXPORT macro defined
    EXTRA_SOURCE_DIRS source/core
    USE_EXTRA_WARNINGS
    LINK_WITH_PRIVATE miniz lz4_static Threads::Threads ${CMAKE_DL_LIBS}
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
    LINK_WITH_PRIVATE core slang slang-reflect-headers Threads::Threads
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

if(SLANG_SLANG_LLVM_FLAVOR STREQUAL "FETCH_BINARY")
    #
    # Do some stupid little dance to put everything in the right shape with
    # correct dependencies
    #

    set(slang_llvm_filename "${CMAKE_SHARED_LIBRARY_PREFIX}slang-llvm${CMAKE_SHARED_LIBRARY_SUFFIX}")
    macro(from_glob dir)
        # A little helper function
        file(GLOB_RECURSE slang_llvm_source_object "${dir}/${slang_llvm_filename}")
        list(LENGTH slang_llvm_source_object nmatches)
        if(nmatches EQUAL 0)
            message(SEND_ERROR "Unable to find ${slang_llvm_filename} in ${SLANG_SLANG_LLVM_BINARY_URL}")
        elseif(nmatches GREATER 1)
            message(SEND_ERROR "Found multiple files named ${slang_llvm_filename} in ${SLANG_SLANG_LLVM_BINARY_URL}")
        endif()
    endmacro()

    if(IS_DIRECTORY "${SLANG_SLANG_LLVM_BINARY_URL}")
        # Just glob directly from a local directory
        from_glob("${SLANG_SLANG_LLVM_BINARY_URL}")
    elseif(SLANG_SLANG_LLVM_BINARY_URL MATCHES "${CMAKE_SHARED_LIBRARY_PREFIX}.+${CMAKE_SHARED_LIBRARY_SUFFIX}$"
        AND EXISTS "${SLANG_SLANG_LLVM_BINARY_URL}")
        # Otherwise, if it's a direct path to a shared object, use that
        set(slang_llvm_source_object "${SLANG_SLANG_LLVM_BINARY_URL}")
    else()
        # Otherwise, download and extract from whatever URL we have
        fetchcontent_declare(slang-llvm URL "${SLANG_SLANG_LLVM_BINARY_URL}")
        fetchcontent_populate(slang-llvm)
        from_glob("${slang-llvm_SOURCE_DIR}")
    endif()

    set(slang_llvm_dest_object ${CMAKE_BINARY_DIR}/$<CONFIG>/lib/${slang_llvm_filename})
    add_custom_command(
        OUTPUT ${slang_llvm_dest_object}
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${slang_llvm_source_object}
            ${slang_llvm_dest_object}
        DEPENDS ${slang_llvm_source_object}
        VERBATIM
    )
    # Give this copying action a name
    add_custom_target(copy-slang-llvm DEPENDS ${slang_llvm_dest_object})

    # Put this into a library target
    add_library(slang-llvm MODULE IMPORTED GLOBAL)
    add_dependencies(slang-llvm copy-slang-llvm)
    set_property(
        TARGET slang-llvm
        PROPERTY IMPORTED_LOCATION ${slang_llvm_dest_object}
    )
    install(PROGRAMS ${slang_llvm_dest_object} DESTINATION module_subdir)
elseif(SLANG_SLANG_LLVM_FLAVOR STREQUAL "USE_SYSTEM_LLVM" OR SLANG_SLANG_LLVM_FLAVOR STREQUAL "BUILD_LLVM")
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
    add_supported_cxx_linker_flags(slang-llvm PRIVATE "-Wl,--exclude-libs,ALL")

    # TODO: Put a check here that libslang-llvm.so doesn't have a 'NEEDED'
    # directive for libLLVM-13.so, it's almost certainly going to break at
    # runtime in surprising ways when linked alongside Mesa (or anything else
    # pulling in libLLVM.so)
endif()

#
# `platform` contains all the platform abstractions for a GUI application.
#
slang_add_target(
    tools/platform
    SHARED
    EXCLUDE_FROM_ALL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE
        core
        imgui
        $<$<BOOL:${SLANG_ENABLE_XLIB}>:X11::X11>
        ${CMAKE_DL_LIBS}
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
        slang-glslang
        # Used by some tests when they run
        slangd
        slang-rt
        gfx
)
if(TARGET slang-llvm)
    # Only add a dependency here if slang-llvm exists
    # Pending https://gitlab.kitware.com/cmake/cmake/-/issues/19467 to be able
    # to use a generator expression in REQUIRES just above.
    add_dependencies(slang-test slang-llvm)
endif()

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
    LINK_WITH_PRIVATE core slang Threads::Threads
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

#
# Testing
#
include(CTest)
add_test(
    NAME slang-test
    COMMAND
        slang-test
        -bindir ${slang_SOURCE_DIR}/build/Release/bin
        -expected-failure-list ${slang_SOURCE_DIR}/tests/expected-failure.txt
        -expected-failure-list ${slang_SOURCE_DIR}/tests/expected-failure-github.txt
    WORKING_DIRECTORY ${slang_SOURCE_DIR}
)

#
# Packaging
#
include(CPack)
