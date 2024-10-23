cmake_minimum_required(VERSION 3.22)

# Our module dir, include that now so that we can get the version automatically
# from git describe
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(GitVersion)
get_git_version(SLANG_VERSION_NUMERIC SLANG_VERSION_FULL "${CMAKE_CURRENT_LIST_DIR}")

#
# Our project
#
project(slang VERSION "${SLANG_VERSION_NUMERIC}" LANGUAGES)
set(PROJECT_VERSION "${SLANG_VERSION_FULL}")

#
# Global CMake options
#
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24")
    cmake_policy(SET CMP0135 OLD)
endif()
cmake_policy(SET CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.25")
    cmake_policy(SET CMP0141 NEW)
endif()
cmake_policy(SET CMP0091 NEW)

# Don't use absolute paths to the build tree in RPATH, this makes the build
# tree relocatable
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

# Enable placing targets into a hierarchy for IDE generators
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#
# CMake-supplied modules and our utils
#
enable_language(C CXX)

include(FindPackageHandleStandardArgs)
include(CMakeDependentOption)
include(GNUInstallDirs)

include(CCacheDebugInfoWorkaround)
include(CompilerFlags)
include(Glob)
include(LLVM)
include(SlangTarget)
include(AutoOption)
include(GitHubRelease)
include(FetchedSharedLibrary)

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
    "Enable OptiX build/tests, requires SLANG_ENABLE_CUDA"
)
auto_option(
    SLANG_ENABLE_NVAPI
    NVAPI
    "Enable NVAPI usage (Only available for builds targeting Windows)"
)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    auto_option(
        SLANG_ENABLE_XLIB
        X11
        "Build gfx and platform with Xlib to support windowed apps on Linux"
    )
else()
    set(SLANG_ENABLE_XLIB OFF)
endif()
auto_option(
    SLANG_ENABLE_AFTERMATH
    Aftermath
    "Enable Aftermath in GFX, and add aftermath crash example to project"
)
advanced_option(SLANG_ENABLE_DX_ON_VK "Use dxvk and vkd3d-proton for DirectX support" OFF)
advanced_option(SLANG_ENABLE_SLANG_RHI "Use slang-rhi as dependency" ON)

option(SLANG_EMBED_STDLIB_SOURCE "Embed stdlib source in the binary" ON)
option(SLANG_EMBED_STDLIB "Build slang with an embedded version of the stdlib")

option(SLANG_ENABLE_FULL_IR_VALIDATION "Enable full IR validation (SLOW!)")
option(SLANG_ENABLE_IR_BREAK_ALLOC, "Enable _debugUID on IR allocation")
option(SLANG_ENABLE_ASAN "Enable ASAN (address sanitizer)")

option(SLANG_ENABLE_PREBUILT_BINARIES "Enable using prebuilt binaries" ON)
option(SLANG_ENABLE_GFX "Enable gfx targets" ON)
option(SLANG_ENABLE_SLANGD "Enable language server target" ON)
option(SLANG_ENABLE_SLANGC "Enable standalone compiler target" ON)
option(SLANG_ENABLE_SLANGRT "Enable runtime target" ON)
option(SLANG_ENABLE_SLANG_GLSLANG "Enable glslang dependency and slang-glslang wrapper target" ON)
option(SLANG_ENABLE_TESTS "Enable test targets, some tests may require SLANG_ENABLE_GFX, SLANG_ENABLE_SLANGD or SLANG_ENABLE_SLANGRT" ON)
option(SLANG_ENABLE_EXAMPLES "Enable example targets, requires SLANG_ENABLE_GFX" ON)
option(SLANG_ENABLE_REPLAYER "Enable slang-replay tool" ON)

option(SLANG_GITHUB_TOKEN "Use a given token value for accessing Github REST API" "")

advanced_option(SLANG_USE_SYSTEM_MINIZ "Build using system Miniz library" OFF)
advanced_option(SLANG_USE_SYSTEM_LZ4 "Build using system LZ4 library" OFF)
advanced_option(SLANG_USE_SYSTEM_VULKAN_HEADERS "Build using system Vulkan headers" OFF)
advanced_option(SLANG_USE_SYSTEM_SPIRV_HEADERS "Build using system SPIR-V headers" OFF)
advanced_option(SLANG_USE_SYSTEM_UNORDERED_DENSE "Build using system unordered dense" OFF)

option(SLANG_SPIRV_HEADERS_INCLUDE_DIR "Provide a specific path for the SPIR-V headers and grammar files")
mark_as_advanced(SLANG_SPIRV_HEADERS_INCLUDE_DIR)

if (${SLANG_USE_SYSTEM_LZ4})
    add_compile_definitions(SLANG_USE_SYSTEM_LZ4_HEADER)
endif()

if (${SLANG_USE_SYSTEM_SPIRV_HEADERS})
    add_compile_definitions(SLANG_USE_SYSTEM_SPIRV_HEADER)
endif()

if (${SLANG_USE_SYSTEM_UNORDERED_DENSE})
    add_compile_definitions(SLANG_USE_SYSTEM_UNORDERED_DENSE_HEADER)
endif()

enum_option(
    SLANG_LIB_TYPE
    # Default
    SHARED
    "How to build the slang lib:"
    # Options
    SHARED
    "Build slang as a shared library (default)"
    STATIC
    "Build slang as a static library"
)

set(SLANG_GENERATORS_PATH
    ""
    CACHE PATH
    "An optional path to the outputs of the all-generators target compiled for the build platform, used when cross-compiling"
)

enum_option(
    SLANG_SLANG_LLVM_FLAVOR
    # Default
    FETCH_BINARY_IF_POSSIBLE
    "How to get or build slang-llvm:"
    # Options
    FETCH_BINARY
    "Use a binary distribution of the slang-llvm library instead of building or using LLVM"
    FETCH_BINARY_IF_POSSIBLE
    "Like FETCH_BINARY, except falls back to DISABLE if a prebuilt slang-llvm can't be downloaded"
    USE_SYSTEM_LLVM
    "Build slang-llvm using system-provided LLVM and Clang binaries"
    DISABLE
    "Do not build llvm or fetch slang-llvm"
)

if(SLANG_SLANG_LLVM_FLAVOR MATCHES FETCH_BINARY)
    # If the user didn't specify a URL, find the best one now
    if(NOT SLANG_SLANG_LLVM_BINARY_URL)
        get_best_slang_binary_release_url("${SLANG_GITHUB_TOKEN}" url)
        if(NOT DEFINED url)
            if(SLANG_SLANG_LLVM_FLAVOR STREQUAL FETCH_BINARY_IF_POSSIBLE)
                message(WARNING "Unable to find a prebuilt binary for slang-llvm, Slang will be built without LLVM support. Please consider setting SLANG_SLANG_LLVM_BINARY_URL manually")
            else()
                message(FATAL_ERROR "Unable to find binary release for slang-llvm, please set a different SLANG_SLANG_LLVM_FLAVOR or set SLANG_SLANG_LLVM_BINARY_URL manually")
            endif()
        endif()
    endif()
    set(SLANG_SLANG_LLVM_BINARY_URL
        ${url}
        CACHE STRING
        "URL specifying the location of the slang-llvm prebuilt library"
    )
endif()

set(webgpu_dawn_release_tag "webgpu_dawn-0")
if(CMAKE_SYSTEM_NAME MATCHES "Windows" AND
    CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
  set(
    SLANG_WEBGPU_DAWN_BINARY_URL
    "https://github.com/shader-slang/dawn/releases/download/${webgpu_dawn_release_tag}/webgpu_dawn-windows-x64.zip"
  )
endif()

set(slang_tint_release_tag "slang-tint-0")
if(CMAKE_SYSTEM_NAME MATCHES "Windows" AND
    CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
  set(
    SLANG_SLANG_TINT_BINARY_URL
    "https://github.com/shader-slang/dawn/releases/download/${slang_tint_release_tag}/slang-tint-windows-x64.zip"
  )
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

if(SLANG_ENABLE_NVAPI AND NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    message(SEND_ERROR "SLANG_ENABLE_NVAPI is only supported on Windows")
endif()

#
# Clean files from premake
#
# The premake builds places generated files in the source tree, make sure these
# aren't present
glob_append(
    premake_generated_files
    "source/slang/*.meta.slang.h"
    "source/slang/*-generated-*.h"
    "source/slang/slang-lookup-*.cpp"
)
if(premake_generated_files)
    file(REMOVE ${premake_generated_files})
endif()
file(REMOVE external/miniz/miniz_export.h)

#
# Dependencies, most of these are however handled inside the "auto_option"
# calls above
#

find_package(Threads REQUIRED)

if(SLANG_SLANG_LLVM_FLAVOR STREQUAL "USE_SYSTEM_LLVM")
    find_package(LLVM 13.0 REQUIRED CONFIG)
    find_package(Clang REQUIRED CONFIG)
endif()

if (${SLANG_USE_SYSTEM_UNORDERED_DENSE})
    find_package(unordered_dense CONFIG QUIET)
endif()

add_subdirectory(external)

set(SLANG_CORE_LIBRARIES "")
list(APPEND SLANG_CORE_LIBRARIES miniz lz4_static Threads::Threads ${CMAKE_DL_LIBS})

#
# Our targets
#

slang_add_target(
    source/core
    STATIC
    EXCLUDE_FROM_ALL
    USE_EXTRA_WARNINGS
    LINK_WITH_PRIVATE ${SLANG_CORE_LIBRARIES}
    LINK_WITH_PUBLIC unordered_dense::unordered_dense
    INCLUDE_DIRECTORIES_PUBLIC source include
)

if(SLANG_ENABLE_SLANGRT)
    slang_add_target(
        source/slang-rt
        SHARED
        # This compiles 'core' again with the SLANG_RT_DYNAMIC_EXPORT macro defined
        EXTRA_SOURCE_DIRS source/core
        USE_EXTRA_WARNINGS
        LINK_WITH_PRIVATE ${SLANG_CORE_LIBRARIES}
        LINK_WITH_PUBLIC unordered_dense::unordered_dense
        EXPORT_MACRO_PREFIX SLANG_RT
        INCLUDE_DIRECTORIES_PUBLIC include
        INSTALL
    )
endif()

slang_add_target(
    source/compiler-core
    STATIC
    EXCLUDE_FROM_ALL
    USE_EXTRA_WARNINGS
    LINK_WITH_PRIVATE core
    INCLUDE_FROM_PUBLIC SPIRV-Headers
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
    all-generators
    COMMENT "meta target which depends on all generators"
)

if(CMAKE_HOST_WIN32)
  set(CMAKE_HOST_EXECUTABLE_SUFFIX ".exe")
else()
  set(CMAKE_HOST_EXECUTABLE_SUFFIX "")
endif()

set_target_properties(all-generators PROPERTIES FOLDER generators)
function(generator dir)
    if(SLANG_GENERATORS_PATH)
        cmake_parse_arguments(ARG "" "TARGET_NAME" "" ${ARGN})
        if(ARG_TARGET_NAME)
            set(target ${ARG_TARGET_NAME})
        else()
            get_filename_component(target ${dir} NAME)
        endif()
        add_executable(${target} IMPORTED)
        set_property(
            TARGET ${target}
            PROPERTY
                IMPORTED_LOCATION
                    "${SLANG_GENERATORS_PATH}/${target}${CMAKE_HOST_EXECUTABLE_SUFFIX}"
        )
    else()
        slang_add_target(
            ${dir}
            EXECUTABLE
            EXCLUDE_FROM_ALL
            USE_FEWER_WARNINGS
            LINK_WITH_PRIVATE core
            OUTPUT_DIR generators
            REQUIRED_BY all-generators
            FOLDER generators
            INSTALL_COMPONENT generators
            ${ARGN}
        )
    endif()
endfunction()

generator(tools/slang-cpp-extractor USE_FEWER_WARNINGS LINK_WITH_PRIVATE compiler-core)
generator(tools/slang-embed)
generator(tools/slang-generate USE_FEWER_WARNINGS)
generator(tools/slang-lookup-generator LINK_WITH_PRIVATE compiler-core)
generator(tools/slang-capability-generator LINK_WITH_PRIVATE compiler-core)
generator(tools/slang-spirv-embed-generator LINK_WITH_PRIVATE compiler-core)

generator(
    source/slangc
    TARGET_NAME slang-bootstrap
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE 
        prelude 
        slang-without-embedded-stdlib 
        slang-capability-lookup 
        slang-lookup-tables 
        Threads::Threads
)

# webgpu_dawn is only available as a fetched shared library, since Dawn's nested source
# trees are too large and onerous for us to depend on.
if(SLANG_WEBGPU_DAWN_BINARY_URL)
  copy_fetched_shared_library("webgpu_dawn" "${SLANG_WEBGPU_DAWN_BINARY_URL}")
endif()

# slang-tint is only available as a fetched shared library, since it's hosted in the Dawn
# repository, and Dawn's nested source trees are too large and onerous for us to depend
# on.
if(SLANG_SLANG_TINT_BINARY_URL)
  copy_fetched_shared_library("slang-tint" "${SLANG_SLANG_TINT_BINARY_URL}")
endif()

if(SLANG_SLANG_LLVM_FLAVOR STREQUAL "FETCH_BINARY")
    install_fetched_shared_library("slang-llvm" "${SLANG_SLANG_LLVM_BINARY_URL}")
elseif(SLANG_SLANG_LLVM_FLAVOR STREQUAL "FETCH_BINARY_IF_POSSIBLE")
    if(SLANG_SLANG_LLVM_BINARY_URL)
        install_fetched_shared_library("slang-llvm" "${SLANG_SLANG_LLVM_BINARY_URL}" IGNORE_FAILURE)
        if(NOT TARGET slang-llvm)
            message(WARNING "Unable to fetch slang-llvm prebuilt binary, configuring without LLVM support")
        endif()
    endif()
elseif(SLANG_SLANG_LLVM_FLAVOR STREQUAL "USE_SYSTEM_LLVM")
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
        INSTALL_COMPONENT slang-llvm
    )
    # If we don't include this, then the symbols in the LLVM linked here may
    # conflict with those of other LLVMs linked at runtime, for instance in mesa.
    add_supported_cxx_linker_flags(slang-llvm PRIVATE "-Wl,--exclude-libs,ALL")

    # The LLVM headers need a warning disabling, which somehow slips through \external
    if(MSVC)
        target_compile_options(slang-llvm PRIVATE -wd4244)
    endif()

    # TODO: Put a check here that libslang-llvm.so doesn't have a 'NEEDED'
    # directive for libLLVM-13.so, it's almost certainly going to break at
    # runtime in surprising ways when linked alongside Mesa (or anything else
    # pulling in libLLVM.so)
endif()

if(SLANG_ENABLE_PREBUILT_BINARIES)
    if(CMAKE_SYSTEM_NAME MATCHES "Windows")
        # DX Agility SDK requires the D3D12*.DLL files to be placed under a sub-directory, "D3D12".
        # https://devblogs.microsoft.com/directx/gettingstarted-dx12agility/#d3d12sdkpath-should-not-be-the-same-directory-as-the-application-exe
        file(GLOB prebuilt_binaries "${CMAKE_SOURCE_DIR}/external/slang-binaries/bin/windows-x64/*")
        file(GLOB prebuilt_d3d12_binaries "${CMAKE_SOURCE_DIR}/external/slang-binaries/bin/windows-x64/[dD]3[dD]12*")
        list(REMOVE_ITEM prebuilt_binaries ${prebuilt_d3d12_binaries})
        add_custom_target(
            copy-prebuilt-binaries ALL
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/$<CONFIG>/${runtime_subdir}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                ${prebuilt_binaries}
                ${CMAKE_BINARY_DIR}/$<CONFIG>/${runtime_subdir}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/$<CONFIG>/${runtime_subdir}/D3D12
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${prebuilt_d3d12_binaries}
                ${CMAKE_BINARY_DIR}/$<CONFIG>/${runtime_subdir}/D3D12
            VERBATIM
        )
    endif()
endif()


#
#
# keep these non-trivial targets in their own directories so as not to clutter
# this file
add_subdirectory(prelude)
add_subdirectory(source/slang-stdlib)
add_subdirectory(source/slang)
add_subdirectory(source/slangc)
add_subdirectory(source/slang-wasm)
add_subdirectory(source/slang-glslang)
add_subdirectory(tools)
add_subdirectory(examples)

#
# Packaging
#
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)
set(CPACK_STRIP_FILES FALSE)
install(FILES
  "${slang_SOURCE_DIR}/README.md"
  "${slang_SOURCE_DIR}/LICENSE"
  DESTINATION .
  COMPONENT metadata
  EXCLUDE_FROM_ALL
)
install(DIRECTORY
  "${slang_SOURCE_DIR}/docs/"
  DESTINATION share/doc/slang
)
install(DIRECTORY
  "${slang_SOURCE_DIR}/include"
  DESTINATION .
)

include(CPack)
