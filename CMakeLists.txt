cmake_minimum_required(VERSION 3.20)

# Our module dir, include that now so that we can get the version automatically
# from git describe
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(GitVersion)
get_git_version(SLANG_VERSION "${CMAKE_CURRENT_LIST_DIR}")

#
# Our project
#
project(slang VERSION "${SLANG_VERSION}" LANGUAGES)

#
# Global CMake options
#
cmake_policy(SET CMP0135 OLD)
cmake_policy(SET CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.25")
    cmake_policy(SET CMP0141 NEW)
endif()

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
include(FetchContent)
include(GNUInstallDirs)

include(CCacheDebugInfoWorkaround)
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
option(SLANG_ENABLE_DX_ON_VK "Use dxvk and vkd3d-proton for DirectX support")
mark_as_advanced(SLANG_ENABLE_DX_ON_VK)

option(SLANG_EMBED_STDLIB_SOURCE "Embed stdlib source in the binary" ON)
option(SLANG_EMBED_STDLIB "Build slang with an embedded version of the stdlib")

option(SLANG_ENABLE_FULL_IR_VALIDATION "Enable full IR validation (SLOW!)")
option(SLANG_ENABLE_ASAN "Enable ASAN (address sanitizer)")

option(SLANG_ENABLE_GFX "Enable gfx targets" ON)
option(SLANG_ENABLE_SLANGD "Enable language server target" ON)
option(SLANG_ENABLE_SLANGC "Enable standalone compiler target" ON)
option(SLANG_ENABLE_SLANGRT "Enable runtime target" ON)
option(SLANG_ENABLE_SLANG_GLSLANG "Enable glslang dependency and slang-glslang wrapper target" ON)
option(SLANG_ENABLE_TESTS "Enable test targets, requires SLANG_ENABLE_GFX, SLANG_ENABLE_SLANGD and SLANG_ENABLE_SLANGRT" ON)
option(SLANG_ENABLE_EXAMPLES "Enable example targets, requires SLANG_ENABLE_GFX" ON)

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
    FETCH_BINARY
    "How to get or build slang-llvm:"
    # Options
    FETCH_BINARY
    "Use a binary distribution of the slang-llvm library instead of building or using LLVM (default for Windows)"
    USE_SYSTEM_LLVM
    "Build slang-llvm using system-provided LLVM and Clang binaries (default for non-Windows hosts)"
    DISABLE
    "Do not build llvm or fetch slang-llvm"
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

if(SLANG_ENABLE_SLANGRT)
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
endif()

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
    all-generators
    COMMENT "meta target which depends on all generators"
)
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
                    "${SLANG_GENERATORS_PATH}/${target}${CMAKE_EXECUTABLE_SUFFIX}"
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
    LINK_WITH_PRIVATE prelude slang-no-embedded-stdlib slang-capability-lookup slang-lookup-tables Threads::Threads
)

#
# The compiler itself
#

# keep these non-trivial targets in their own directories so as not to clutter
# this file
add_subdirectory(prelude)
add_subdirectory(source/slang)

if(SLANG_ENABLE_SLANGD)
    slang_add_target(
        tools/slangd
        EXECUTABLE
        LINK_WITH_PRIVATE
            core
            slang
            slang-reflect-headers
            slang-capability-defs
            Threads::Threads
        INSTALL
    )
endif()
if(SLANG_ENABLE_SLANGC)
    slang_add_target(
        source/slangc
        EXECUTABLE
        USE_FEWER_WARNINGS
        LINK_WITH_PRIVATE core slang Threads::Threads
        INSTALL
    )
endif()

#
# Our wrappers for glslang and llvm
#
if(SLANG_ENABLE_SLANG_GLSLANG)
    slang_add_target(
        source/slang-glslang
        MODULE
        USE_EXTRA_WARNINGS
        LINK_WITH_PRIVATE glslang SPIRV SPIRV-Tools-opt
        INSTALL
    )
    # Our only interface is through what we define in source/slang-glslang, in the
    # interests of hygiene, hide anything else we link in.
    add_supported_cxx_linker_flags(slang-glslang PRIVATE "-Wl,--exclude-libs,ALL")
endif()


if(SLANG_SLANG_LLVM_FLAVOR STREQUAL "FETCH_BINARY")
    #
    # Do some stupid little dance to put everything in the right shape with
    # correct dependencies
    #

    set(slang_llvm_filename
        "${CMAKE_SHARED_LIBRARY_PREFIX}slang-llvm${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )
    macro(from_glob dir)
        # A little helper function
        file(
            GLOB_RECURSE slang_llvm_source_object
            "${dir}/${slang_llvm_filename}"
        )
        list(LENGTH slang_llvm_source_object nmatches)
        if(nmatches EQUAL 0)
            message(
                SEND_ERROR
                "Unable to find ${slang_llvm_filename} in ${SLANG_SLANG_LLVM_BINARY_URL}"
            )
        elseif(nmatches GREATER 1)
            message(
                SEND_ERROR
                "Found multiple files named ${slang_llvm_filename} in ${SLANG_SLANG_LLVM_BINARY_URL}"
            )
        endif()
    endmacro()

    if(IS_DIRECTORY "${SLANG_SLANG_LLVM_BINARY_URL}")
        # Just glob directly from a local directory
        from_glob("${SLANG_SLANG_LLVM_BINARY_URL}")
    elseif(
        SLANG_SLANG_LLVM_BINARY_URL
            MATCHES
            "${CMAKE_SHARED_LIBRARY_PREFIX}.+${CMAKE_SHARED_LIBRARY_SUFFIX}$"
        AND EXISTS "${SLANG_SLANG_LLVM_BINARY_URL}"
    )
        # Otherwise, if it's a direct path to a shared object, use that
        set(slang_llvm_source_object "${SLANG_SLANG_LLVM_BINARY_URL}")
    else()
        # Otherwise, download and extract from whatever URL we have
        fetchcontent_declare(slang-llvm URL "${SLANG_SLANG_LLVM_BINARY_URL}")
        fetchcontent_populate(slang-llvm)
        from_glob("${slang-llvm_SOURCE_DIR}")
    endif()

    set(slang_llvm_dest_object
        ${CMAKE_BINARY_DIR}/$<CONFIG>/${library_subdir}/${slang_llvm_filename}
    )
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
    set_target_properties(copy-slang-llvm PROPERTIES FOLDER generated)

    # Put this into a library target
    add_library(slang-llvm MODULE IMPORTED GLOBAL)
    add_dependencies(slang-llvm copy-slang-llvm)
    set_property(
        TARGET slang-llvm
        PROPERTY IMPORTED_LOCATION ${slang_llvm_dest_object}
    )
    install(PROGRAMS ${slang_llvm_dest_object} DESTINATION ${module_subdir})
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

if(SLANG_ENABLE_GFX)
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
            LINK_WITH_FRAMEWORK
            Cocoa
            QuartzCore
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
            $<$<BOOL:${SLANG_ENABLE_CUDA}>:CUDA::cuda_driver>
        LINK_WITH_FRAMEWORK
            Cocoa
            QuartzCore
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
        FOLDER gfx
    )
    set(modules_dest_dir $<TARGET_FILE_DIR:slang-test>)
    add_custom_target(
        copy-gfx-slang-modules
        COMMAND ${CMAKE_COMMAND} -E make_directory ${modules_dest_dir}
        COMMAND
            ${CMAKE_COMMAND} -E copy tools/gfx/gfx.slang
            ${modules_dest_dir}/gfx.slang
        COMMAND
            ${CMAKE_COMMAND} -E copy tools/gfx/slang.slang
            ${modules_dest_dir}/slang.slang
        WORKING_DIRECTORY ${slang_SOURCE_DIR}
        VERBATIM
    )
    set_target_properties(copy-gfx-slang-modules PROPERTIES FOLDER generators)
    install(
        FILES ${modules_dest_dir}/gfx.slang ${modules_dest_dir}/slang.slang
        DESTINATION ${runtime_subdir}
    )

    slang_add_target(
        tools/gfx-util
        STATIC
        LINK_WITH_PRIVATE core
        INCLUDE_FROM_PRIVATE gfx
        # The headers are included with 'include "gfx-util/blah.h"' which is found
        # in the tools directory
        INCLUDE_DIRECTORIES_PUBLIC tools
        FOLDER gfx
    )
endif()

#
# Installing any documentation
#

file(GLOB_RECURSE docs CONFIGURE_DEPENDS docs/*.md)
if(docs)
    install(FILES ${docs} DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()

#
# The test executables and runtime-loaded modules
#
if(SLANG_ENABLE_TESTS AND SLANG_ENABLE_GFX AND SLANG_ENABLE_SLANGD AND SLANG_ENABLE_SLANGRT)
    slang_add_target(
        tools/test-server
        EXECUTABLE
        EXCLUDE_FROM_ALL
        LINK_WITH_PRIVATE core compiler-core slang
        FOLDER test
    )
    slang_add_target(
        tools/test-process
        EXECUTABLE
        EXCLUDE_FROM_ALL
        LINK_WITH_PRIVATE core compiler-core
        FOLDER test
    )

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
            # Used by some tests when they run
            slangd
            slang-rt
            gfx
            test-server
            test-process
        FOLDER test
        DEBUG_DIR ${slang_SOURCE_DIR}
    )
    if(TARGET slang-glslang)
        # Only add a dependency here if slang-glslang exists
        add_dependencies(slang-test slang-glslang)
    endif()
    if(TARGET slang-llvm)
        # Only add a dependency here if slang-llvm exists
        # Pending https://gitlab.kitware.com/cmake/cmake/-/issues/19467 to be able
        # to use a generator expression in REQUIRES just above.
        add_dependencies(slang-test slang-llvm)
    endif()
    set_property(
        DIRECTORY ${slang_SOURCE_DIR}
        PROPERTY VS_STARTUP_PROJECT slang-test
    )

    slang_add_target(
        tools/unit-test
        OBJECT
        EXCLUDE_FROM_ALL
        INCLUDE_FROM_PRIVATE slang
        FOLDER test
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
        FOLDER test/tools
    )
    slang_add_target(
        tools/slang-unit-test
        MODULE
        EXCLUDE_FROM_ALL
        EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_SHARED_LIBRARY_TOOL
        USE_FEWER_WARNINGS
        LINK_WITH_PRIVATE core compiler-core unit-test slang Threads::Threads
        OUTPUT_NAME slang-unit-test-tool
        FOLDER test/tools
    )
    slang_add_target(
        tools/slang-reflection-test
        MODULE
        EXCLUDE_FROM_ALL
        EXTRA_COMPILE_DEFINITIONS_PRIVATE SLANG_SHARED_LIBRARY_TOOL
        USE_FEWER_WARNINGS
        LINK_WITH_PRIVATE core slang Threads::Threads
        OUTPUT_NAME slang-reflection-test-tool
        FOLDER test/tools
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
            $<$<BOOL:${SLANG_ENABLE_CUDA}>:CUDA::cuda_driver>
        EXTRA_COMPILE_DEFINITIONS_PRIVATE
            $<$<BOOL:${SLANG_ENABLE_CUDA}>:RENDER_TEST_CUDA>
            $<$<BOOL:${SLANG_ENABLE_OPTIX}>:RENDER_TEST_OPTIX>
        OUTPUT_NAME render-test-tool
        FOLDER test/tools
    )

    slang_add_target(
        tools/slang-profile
        EXECUTABLE
        EXCLUDE_FROM_ALL
        LINK_WITH_PRIVATE core slang
        FOLDER test
    )
endif()

if (SLANG_ENABLE_EXAMPLES AND SLANG_ENABLE_GFX)
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
            $<$<BOOL:${SLANG_ENABLE_CUDA}>:CUDA::cuda_driver>
        FOLDER examples
    )

    add_custom_target(
        all-examples
        COMMENT "meta target which depends on all examples"
    )
    set_target_properties(all-examples PROPERTIES FOLDER examples)
    function(example dir)
        slang_add_target(
            ${dir}
            EXECUTABLE
            EXCLUDE_FROM_ALL
            USE_FEWER_WARNINGS
            LINK_WITH_PRIVATE
                core
                example-base
                slang
                gfx
                gfx-util
                platform
                $<$<BOOL:${SLANG_ENABLE_CUDA}>:CUDA::cuda_driver>
            EXTRA_COMPILE_DEFINITIONS_PRIVATE
                $<$<BOOL:${SLANG_ENABLE_XLIB}>:SLANG_ENABLE_XLIB>
            REQUIRED_BY all-examples
            FOLDER examples
            DEBUG_DIR ${dir}
            ${ARGN}
        )
    endfunction()

    example(examples/autodiff-texture WIN32_EXECUTABLE)
    example(examples/cpu-com-example )
    example(examples/cpu-hello-world )
    example(examples/gpu-printing )
    example(examples/hello-world LINK_WITH_PRIVATE Vulkan-Headers)
    example(examples/model-viewer WIN32_EXECUTABLE)
    example(examples/platform-test WIN32_EXECUTABLE)
    example(examples/ray-tracing WIN32_EXECUTABLE)
    example(examples/ray-tracing-pipeline WIN32_EXECUTABLE)
    example(examples/shader-object )
    example(examples/shader-toy WIN32_EXECUTABLE)
    example(examples/triangle WIN32_EXECUTABLE)
    if(SLANG_ENABLE_AFTERMATH)
        example(examples/nv-aftermath-example WIN32_EXECUTABLE)
    endif()
endif()

#
# Testing
#
if(SLANG_ENABLE_TESTS)
    include(CTest)
    add_test(
        NAME slang-test
        COMMAND
            slang-test -bindir ${slang_SOURCE_DIR}/build/$<CONFIG>/${runtime_subdir}
            -expected-failure-list ${slang_SOURCE_DIR}/tests/expected-failure.txt
            -expected-failure-list
            ${slang_SOURCE_DIR}/tests/expected-failure-github.txt
        WORKING_DIRECTORY ${slang_SOURCE_DIR}
    )
endif()

#
# Packaging
#
include(CPack)
