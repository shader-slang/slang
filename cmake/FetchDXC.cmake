# FetchDXC.cmake
#
# Fetches DXC (DirectXShaderCompiler) prebuilt binaries via FetchContent and
# copies them to the build output directory.
#
# On Linux, if the system GLIBC version is older than the minimum required by the
# prebuilt binaries, DXC is built from source via ExternalProject instead.
#
# Using FetchContent with the name 'dxc' ensures that slang-rhi (which uses the
# same FetchContent name) will see dxc_POPULATED=TRUE and skip its own download.
#
# Variables:
#   SLANG_DXC_BINARY_URL        - Override the prebuilt binary download URL (optional)
#   SLANG_DXC_BUILD_FROM_SOURCE - ON: always build from source; OFF: always use
#                                 prebuilt (skips auto-detection); unset: auto-detect
#                                 on Linux by checking the system GLIBC version
#   SLANG_GITHUB_TOKEN          - GitHub token for authenticated downloads (optional)
#
# Requires the following variables to be set by the caller (set in SlangTarget.cmake):
#   runtime_subdir   - Destination for Windows DLLs (e.g. "bin")
#   library_subdir   - Destination for Linux .so files (e.g. "lib")

include(FetchContent)

# DXC version used for both the prebuilt binary URLs and the source build Git tag.
set(_dxc_version_tag "v1.9.2602")

# Minimum GLIBC version required by the official prebuilt DXC binaries.
# Bump this when the prebuilt binaries start requiring a newer GLIBC.
set(_dxc_min_glibc "2.38")
string(REPLACE "." ";" _dxc_min_glibc_parts "${_dxc_min_glibc}")
list(GET _dxc_min_glibc_parts 0 _dxc_min_glibc_major)
list(GET _dxc_min_glibc_parts 1 _dxc_min_glibc_minor)

# ---------------------------------------------------------------------------
# Decide whether to build DXC from source.
# ---------------------------------------------------------------------------

set(_dxc_build_from_source OFF)

if(SLANG_DXC_BUILD_FROM_SOURCE)
    # User explicitly requested a source build.
    set(_dxc_build_from_source ON)
elseif(
    NOT DEFINED SLANG_DXC_BUILD_FROM_SOURCE
    AND CMAKE_SYSTEM_NAME STREQUAL "Linux"
    AND NOT DEFINED SLANG_DXC_BINARY_URL
)
    # Auto-detect: check GLIBC version only when the user has not explicitly
    # set SLANG_DXC_BUILD_FROM_SOURCE or provided a custom SLANG_DXC_BINARY_URL.
    execute_process(
        COMMAND ldd --version
        OUTPUT_VARIABLE _ldd_output
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    # First line is typically: "ldd (Ubuntu GLIBC 2.35-0ubuntu3.6) 2.35"
    string(REGEX MATCH "([0-9]+)\\.([0-9]+)" _glibc_match "${_ldd_output}")
    if(_glibc_match)
        set(_glibc_major ${CMAKE_MATCH_1})
        set(_glibc_minor ${CMAKE_MATCH_2})
        message(
            STATUS
            "Detected GLIBC version: ${_glibc_major}.${_glibc_minor}"
        )
        if(
            _glibc_major LESS _dxc_min_glibc_major
            OR (
                _glibc_major EQUAL _dxc_min_glibc_major
                AND _glibc_minor LESS _dxc_min_glibc_minor
            )
        )
            message(
                STATUS
                "GLIBC ${_glibc_major}.${_glibc_minor} < ${_dxc_min_glibc}: prebuilt DXC binaries "
                "require GLIBC ${_dxc_min_glibc}+, building DXC from source (${_dxc_version_tag})"
            )
            set(_dxc_build_from_source ON)
        endif()
    endif()
endif()

# ---------------------------------------------------------------------------
# Source build path.
# ---------------------------------------------------------------------------

if(_dxc_build_from_source)
    include(ExternalProject)

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(_dxc_src_byproducts
            <BINARY_DIR>/bin/dxcompiler.dll
            <BINARY_DIR>/bin/dxil.dll
        )
        set(_dxc_warning_flags "")
    else()
        set(_dxc_src_byproducts
            <BINARY_DIR>/bin/dxc
            <BINARY_DIR>/lib/libdxcompiler.so
            <BINARY_DIR>/lib/libdxil.so
        )
        # LLVM_ENABLE_WARNINGS=OFF prevents adding warning flags but does not
        # actively suppress existing ones. Pass -w via CMAKE_*_FLAGS to silence
        # all GCC/Clang warnings from DXC's source.
        set(_dxc_warning_flags "-DCMAKE_C_FLAGS=-w" "-DCMAKE_CXX_FLAGS=-w")
    endif()

    # DXC's build runs clang-format on generated files inside its build directory.
    # Clang-format walks up from there and finds Slang's root .clang-format, which
    # uses options unsupported by older clang-format versions (e.g.
    # PackConstructorInitializers: NextLineOnly on clang-format 14).
    # Placing DXC's own style (BasedOnStyle: LLVM) in the DXC build directory
    # stops the upward search at the right level.
    #
    set(_dxc_build_dir
        "${CMAKE_BINARY_DIR}/dxc_from_source-prefix/src/dxc_from_source-build"
    )
    file(MAKE_DIRECTORY "${_dxc_build_dir}")
    file(WRITE "${_dxc_build_dir}/.clang-format" "BasedOnStyle: LLVM\n")

    ExternalProject_Add(
        dxc_from_source
        GIT_REPOSITORY "https://github.com/microsoft/DirectXShaderCompiler.git"
        GIT_TAG "${_dxc_version_tag}"
        GIT_SUBMODULES_RECURSE ON
        GIT_PROGRESS ON
        UPDATE_DISCONNECTED ON
        CONFIGURE_COMMAND
            ${CMAKE_COMMAND} -B <BINARY_DIR> -S <SOURCE_DIR> -G Ninja -C
            <SOURCE_DIR>/cmake/caches/PredefinedParams.cmake
            -DCMAKE_BUILD_TYPE=MinSizeRel -DHLSL_COPY_GENERATED_SOURCES=ON
            -DLLVM_INCLUDE_TESTS=OFF -DCLANG_INCLUDE_TESTS=OFF
            -DLLVM_ENABLE_WARNINGS=OFF ${_dxc_warning_flags} -Wno-dev
        BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR>
        INSTALL_COMMAND ""
        BUILD_BYPRODUCTS ${_dxc_src_byproducts}
    )

    ExternalProject_Get_Property(dxc_from_source BINARY_DIR)

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        foreach(_dll dxcompiler dxil)
            set(_src "${BINARY_DIR}/bin/${_dll}.dll")
            set(_dst
                "${CMAKE_BINARY_DIR}/$<CONFIG>/${runtime_subdir}/${_dll}.dll"
            )
            add_custom_target(
                copy-${_dll}
                COMMAND
                    ${CMAKE_COMMAND} -E copy_if_different "${_src}" "${_dst}"
                VERBATIM
            )
            add_dependencies(copy-${_dll} dxc_from_source)
            set_target_properties(copy-${_dll} PROPERTIES FOLDER generated)
        endforeach()
    else()
        foreach(_lib dxcompiler dxil)
            set(_src "${BINARY_DIR}/lib/lib${_lib}.so")
            set(_dst
                "${CMAKE_BINARY_DIR}/$<CONFIG>/${library_subdir}/lib${_lib}.so"
            )
            add_custom_target(
                copy-${_lib}
                COMMAND
                    ${CMAKE_COMMAND} -E copy_if_different "${_src}" "${_dst}"
                VERBATIM
            )
            add_dependencies(copy-${_lib} dxc_from_source)
            set_target_properties(copy-${_lib} PROPERTIES FOLDER generated)
        endforeach()
    endif()

    return()
endif()

# ---------------------------------------------------------------------------
# Prebuilt binary download path.
# ---------------------------------------------------------------------------

if(NOT DEFINED SLANG_DXC_BINARY_URL)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(SLANG_DXC_BINARY_URL
            "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2602/dxc_2026_02_20.zip"
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
            set(SLANG_DXC_BINARY_URL
                "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2602/linux_dxc_2026_02_20.x86_64.tar.gz"
            )
        endif()
    endif()
endif()

if(NOT DEFINED SLANG_DXC_BINARY_URL)
    return()
endif()

message(STATUS "Downloading DXC from: ${SLANG_DXC_BINARY_URL} ...")
set(_dxc_fetch_args
    dxc
    URL
    "${SLANG_DXC_BINARY_URL}"
    SOURCE_SUBDIR
    _does_not_exist_
)
if(SLANG_GITHUB_TOKEN)
    list(
        APPEND
        _dxc_fetch_args
        HTTP_HEADER
        "Authorization: token ${SLANG_GITHUB_TOKEN}"
    )
endif()

FetchContent_Declare(${_dxc_fetch_args})

FetchContent_GetProperties(dxc)
if(NOT dxc_POPULATED)
    FetchContent_MakeAvailable(dxc)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
        set(_dxc_arch arm64)
    else()
        set(_dxc_arch x64)
    endif()
    foreach(_dll dxcompiler dxil)
        set(_src "${dxc_SOURCE_DIR}/bin/${_dxc_arch}/${_dll}.dll")
        set(_dst "${CMAKE_BINARY_DIR}/$<CONFIG>/${runtime_subdir}/${_dll}.dll")
        add_custom_command(
            OUTPUT "${_dst}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_src}" "${_dst}"
            DEPENDS "${_src}"
            VERBATIM
        )
        add_custom_target(copy-${_dll} DEPENDS "${_dst}")
        set_target_properties(copy-${_dll} PROPERTIES FOLDER generated)
    endforeach()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    foreach(_lib dxcompiler dxil)
        set(_src "${dxc_SOURCE_DIR}/lib/lib${_lib}.so")
        set(_dst
            "${CMAKE_BINARY_DIR}/$<CONFIG>/${library_subdir}/lib${_lib}.so"
        )
        add_custom_command(
            OUTPUT "${_dst}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_src}" "${_dst}"
            DEPENDS "${_src}"
            VERBATIM
        )
        add_custom_target(copy-${_lib} DEPENDS "${_dst}")
        set_target_properties(copy-${_lib} PROPERTIES FOLDER generated)
    endforeach()
endif()
