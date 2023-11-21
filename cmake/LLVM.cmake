macro(build_llvm)
    #
    # Settings one might wish to change for this build
    #
    set(llvm_version 13.0.1)
    set(llvm_url
        https://github.com/llvm/llvm-project/releases/download/llvmorg-${llvm_version}/llvm-project-${llvm_version}.src.tar.xz
    )
    set(llvm_sha256
        326335a830f2e32d06d0a36393b5455d17dc73e0bd1211065227ee014f92cbf8
    )
    set(llvm_install_root "${CMAKE_BINARY_DIR}/llvm-project")
    set(llvm_config Release)

    #
    # Some validation
    #
    if(CMAKE_CROSSCOMPILING)
        message(
            FATAL_ERROR
            "Cross building LLVM isn't supported here, please enhance find_llvm() or use set SLANG_USE_SYSTEM_LLVM"
        )
    endif()
    if (CMAKE_SYSTEM_NAME MATCHES "Windows" AND NOT CMAKE_BUILD_TYPE MATCHES "^Release.*")
        message(
            WARNING
            "Building LLVM in a Release configuration for a debug or multi-config Slang build "
            "will likely lead to problems linking a Debug slang-llvm on Windows, where incompatible "
            "runtime libraries may be used.\n"
            "SLANG_BUILD_LLVM is only intended to be used for release creation. On Windows it's "
            "recommended to use SLANG_USE_BINARY_SLANG_LLVM and on other platforms it's "
            "recommended to use SLANG_USE_SYSTEM_LLVM for local development. "
        )
    endif()

    #
    # Fetch LLVM
    #
    fetchcontent_declare(
        llvm-project
        URL ${llvm_url}
        URL_HASH SHA256=${llvm_sha256}
        PREFIX
        llvm-project
    )
    fetchcontent_makeavailable(llvm-project)

    #
    # Configure, build and install a CMake project
    #
    BuildNestedCMakeProject(
      SOURCE_DIR "${llvm-project_SOURCE_DIR}/llvm"
      BINARY_DIR "${llvm-project_BINARY_DIR}"
      BUILD_TYPE ${llvm_config}
      INSTALL_PREFIX ${llvm_install_root}
      EXTRA_CMAKE_ARGS
        # Don't build unnecessary things
        -DLLVM_BUILD_LLVM_C_DYLIB=0
        -DLLVM_INCLUDE_BENCHMARKS=0
        -DLLVM_INCLUDE_DOCS=0
        -DLLVM_INCLUDE_EXAMPLES=0
        -DLLVM_INCLUDE_TESTS=0
        -DLLVM_ENABLE_TERMINFO=0
        -DCLANG_BUILD_TOOLS=0
        -DCLANG_ENABLE_STATIC_ANALYZER=0
        -DCLANG_ENABLE_ARCMT=0
        -DCLANG_INCLUDE_DOCS=0
        -DCLANG_INCLUDE_TESTS=0
        # Requirements for Slang
        -DCMAKE_CXX_VISIBILITY_PRESET=hidden
        -DLLVM_ENABLE_PROJECTS=clang
        "-DLLVM_TARGETS_TO_BUILD=X86\\\;ARM\\\;AArch64"
        -DLLVM_BUILD_TOOLS=1
    )

    #
    # Don't require a specific version here, whatever we've build is
    # clearly what we want
    #
    find_package(
        LLVM
        REQUIRED
        CONFIG
        PATHS ${llvm_install_root}
        NO_DEFAULT_PATH
    )
    find_package(
        Clang
        REQUIRED
        CONFIG
        PATHS ${llvm_install_root}
        NO_DEFAULT_PATH
    )
endmacro()

# A convenience on top of the llvm package's cmake files, this creates a target
# to pass to target_link_libraries which correctly pulls in the llvm include
# dir and other compile dependencies
function(llvm_target_from_components target_name)
    set(components ${ARGN})
    llvm_map_components_to_libnames(llvm_libs
        ${components}
    )
    add_library(${target_name} INTERFACE)
    target_link_libraries(${target_name} INTERFACE ${llvm_libs})
    target_include_directories(
        ${target_name}
        SYSTEM
        INTERFACE ${LLVM_INCLUDE_DIRS}
    )
    target_compile_definitions(${target_name} INTERFACE ${LLVM_DEFINITIONS})
    if(NOT LLVM_ENABLE_RTTI)
        # Make sure that we don't disable rtti if this library wasn't compiled with
        # support
        add_supported_cxx_flags(${target_name} INTERFACE -fno-rtti /GR-)
    endif()
endfunction()

# The same for clang
function(clang_target_from_libs target_name)
    set(clang_libs ${ARGN})
    add_library(${target_name} INTERFACE)
    target_link_libraries(${target_name} INTERFACE ${clang_libs})
    target_include_directories(
        ${target_name}
        SYSTEM
        INTERFACE ${CLANG_INCLUDE_DIRS}
    )
    target_compile_definitions(${target_name} INTERFACE ${CLANG_DEFINITIONS})
    if(NOT LLVM_ENABLE_RTTI)
        # Make sure that we don't disable rtti if this library wasn't compiled with
        # support
        add_supported_cxx_flags(${target_name} INTERFACE -fno-rtti /GR-)
    endif()
endfunction()
