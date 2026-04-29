# FetchDXC.cmake
#
# Fetches DXC (DirectXShaderCompiler) prebuilt binaries via FetchContent and
# copies them to the build output directory.
#
# On Linux, if the system GLIBC version is older than the minimum required by the
# prebuilt binaries, DXC is built from source via ExternalProject instead.
#
# Both paths register the FetchContent 'dxc' name as populated so that slang-rhi
# (which fetches its own DXC under the same name) skips its own download.
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

# DXC version Git tag. Used for the source build GIT_TAG and as the
# release path component in the prebuilt binary URLs below.
# When upgrading DXC, bump this AND update _dxc_release_date below.
set(_dxc_version_tag "v1.9.2602")

# Release date embedded in the prebuilt binary filenames (e.g.
# "dxc_2026_02_20.zip"). DXC releases use date-stamped filenames, so this
# must be updated together with _dxc_version_tag when upgrading.
set(_dxc_release_date "2026_02_20")

# GLIBC threshold below which DXC is built from source instead of using the
# prebuilt binaries. Both libdxcompiler.so and libdxil.so from the v1.9.2602
# release require GLIBC 2.38 to load at runtime. However, this threshold is
# intentionally set to 2.29 — below the real runtime requirement — so that
# the source-build path only activates for the project's oldest release CI
# containers (Ubuntu 18.04 / GLIBC 2.27-2.28). Systems with GLIBC in
# [2.29, 2.38) continue to download prebuilt binaries that may not load at
# runtime (DXC silently unavailable), which matches the behavior before this
# feature was added. Set SLANG_DXC_BUILD_FROM_SOURCE=ON explicitly to force
# a source build on those systems if DXC is needed.
# Bump when upgrading to a DXC release that changes the oldest supported container.
set(_dxc_min_glibc "2.29")

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
    AND NOT CMAKE_CROSSCOMPILING
    AND NOT DEFINED SLANG_DXC_BINARY_URL
)
    # Auto-detect: check GLIBC version only when the user has not explicitly
    # set SLANG_DXC_BUILD_FROM_SOURCE or provided a custom SLANG_DXC_BINARY_URL.
    #
    # Prefer getconf GNU_LIBC_VERSION: it outputs "glibc X.Y" on all glibc
    # systems regardless of distro (Debian/Ubuntu say "GLIBC"; RHEL/Fedora/Arch
    # say "GNU libc" in ldd output, which would not match a "glibc"-anchored
    # regex). Fall back to ldd --version for completeness.
    execute_process(
        COMMAND getconf GNU_LIBC_VERSION
        OUTPUT_VARIABLE _libc_probe
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _getconf_result
    )
    if(NOT _getconf_result EQUAL 0)
        execute_process(
            COMMAND ldd --version
            OUTPUT_VARIABLE _libc_probe
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
    endif()
    # getconf outputs "glibc 2.35"; ldd (Debian/Ubuntu) outputs
    # "ldd (Ubuntu GLIBC 2.35-0ubuntu3.6) 2.35". Both contain "glibc X.Y".
    string(
        REGEX MATCH
        "[Gg][Ll][Ii][Bb][Cc][^0-9]*([0-9]+)\\.([0-9]+)"
        _glibc_match
        "${_libc_probe}"
    )
    if(_glibc_match)
        set(_glibc_version "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}")
        message(STATUS "Detected GLIBC version: ${_glibc_version}")
        if(_glibc_version VERSION_LESS _dxc_min_glibc)
            message(
                STATUS
                "GLIBC ${_glibc_version} < ${_dxc_min_glibc}: prebuilt DXC binaries "
                "require GLIBC ${_dxc_min_glibc}+, building DXC from source (${_dxc_version_tag})"
            )
            set(_dxc_build_from_source ON)
        endif()
    else()
        # Neither getconf nor ldd returned a recognizable GLIBC version (e.g.
        # musl libc). Prebuilt DXC binaries are glibc-linked and may not run.
        # Set -DSLANG_DXC_BUILD_FROM_SOURCE=ON to build from source if needed.
        message(
            WARNING
            "Could not detect GLIBC version; "
            "prebuilt DXC binaries may not run on this system. "
            "Set -DSLANG_DXC_BUILD_FROM_SOURCE=ON to build DXC from source."
        )
    endif()
endif()

# ---------------------------------------------------------------------------
# Source build path.
# ---------------------------------------------------------------------------

if(_dxc_build_from_source)
    include(ExternalProject)

    # DXC's build (PredefinedParams.cmake) is designed for a single-config
    # generator. Normalize any Ninja variant (e.g. "Ninja Multi-Config") to
    # plain "Ninja"; for non-Ninja generators (e.g. Visual Studio on Windows)
    # mirror the parent so the same toolchain is used.
    #
    # LLVM uses CMAKE_CFG_INTDIR (e.g. "." for Ninja, "$(Configuration)" for VS)
    # to set LLVM_RUNTIME_OUTPUT_INTDIR, so for VS the DLLs land in
    # <BINARY_DIR>/MinSizeRel/bin/ rather than <BINARY_DIR>/bin/. Track this
    # in _dxc_dll_subdir so byproducts and copy commands point to the right path.
    if(CMAKE_GENERATOR MATCHES "Ninja")
        set(_dxc_generator_args -G Ninja)
        set(_dxc_build_command ${CMAKE_COMMAND} --build <BINARY_DIR>)
        set(_dxc_dll_subdir "bin")
    else()
        set(_dxc_generator_args -G "${CMAKE_GENERATOR}")
        if(CMAKE_GENERATOR_PLATFORM)
            list(APPEND _dxc_generator_args -A "${CMAKE_GENERATOR_PLATFORM}")
        endif()
        if(CMAKE_GENERATOR_TOOLSET)
            list(APPEND _dxc_generator_args -T "${CMAKE_GENERATOR_TOOLSET}")
        endif()
        set(_dxc_build_command
            ${CMAKE_COMMAND}
            --build
            <BINARY_DIR>
            --config
            MinSizeRel
        )
        set(_dxc_dll_subdir "MinSizeRel/bin")
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(_dxc_src_byproducts
            "<BINARY_DIR>/${_dxc_dll_subdir}/dxcompiler.dll"
            "<BINARY_DIR>/${_dxc_dll_subdir}/dxil.dll"
        )
        set(_dxc_warning_flags "")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # DXC's Linux source build produces libdxcompiler.so and libdxil.so
        # (the DXIL validator), matching the layout of the prebuilt tarballs.
        set(_dxc_src_byproducts
            <BINARY_DIR>/bin/dxc
            <BINARY_DIR>/lib/libdxcompiler.so
            <BINARY_DIR>/lib/libdxil.so
        )
        # LLVM_ENABLE_WARNINGS=OFF prevents adding warning flags but does not
        # actively suppress existing ones. Pass -w via CMAKE_*_FLAGS to silence
        # all GCC/Clang warnings from DXC's source.
        set(_dxc_warning_flags "-DCMAKE_C_FLAGS=-w" "-DCMAKE_CXX_FLAGS=-w")
    else()
        message(
            FATAL_ERROR
            "SLANG_DXC_BUILD_FROM_SOURCE is not supported on "
            "${CMAKE_SYSTEM_NAME}; supported platforms are Windows and Linux."
        )
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
        BINARY_DIR "${_dxc_build_dir}"
        GIT_REPOSITORY "https://github.com/microsoft/DirectXShaderCompiler.git"
        GIT_TAG "${_dxc_version_tag}"
        GIT_SHALLOW ON
        GIT_SUBMODULES_RECURSE ON
        GIT_PROGRESS ON
        UPDATE_DISCONNECTED ON
        CONFIGURE_COMMAND
            ${CMAKE_COMMAND} -B <BINARY_DIR> -S <SOURCE_DIR>
            ${_dxc_generator_args} -C
            <SOURCE_DIR>/cmake/caches/PredefinedParams.cmake
            # CMAKE_BUILD_TYPE is ignored by multi-config generators (VS);
            # the config is selected via --config MinSizeRel in BUILD_COMMAND.
            # Passing it here covers single-config generators (Ninja) in one line.
            -DCMAKE_BUILD_TYPE=MinSizeRel -DHLSL_COPY_GENERATED_SOURCES=ON
            -DLLVM_INCLUDE_TESTS=OFF -DCLANG_INCLUDE_TESTS=OFF
            -DLLVM_ENABLE_WARNINGS=OFF ${_dxc_warning_flags} -Wno-dev
        BUILD_COMMAND ${_dxc_build_command}
        INSTALL_COMMAND ""
        BUILD_BYPRODUCTS ${_dxc_src_byproducts}
    )

    ExternalProject_Get_Property(dxc_from_source BINARY_DIR)

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        foreach(_dll dxcompiler dxil)
            set(_src "${BINARY_DIR}/${_dxc_dll_subdir}/${_dll}.dll")
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

    # Prevent slang-rhi from downloading its own prebuilt DXC independently.
    # slang-rhi's FetchPackage macro calls FetchContent_GetProperties(dxc),
    # which reads the internal _FetchContent_dxc_populated global property and
    # skips its own download when TRUE. Setting this property directly is the
    # only practical option here: FetchContent_MakeAvailable would conflict with
    # ExternalProject_Add (which owns the actual clone), and setting the
    # public-facing dxc_POPULATED variable in this scope would not propagate to
    # the calling scope where slang-rhi's FetchPackage runs.
    #
    # Note: slang-rhi only accesses dxc_SOURCE_DIR on Windows (inside an
    # if(WIN32) block); on Linux, population state is the only thing checked.
    # If the project's CMake minimum is ever raised to 3.24+, replace the
    # set_property call below with FetchContent_SetPopulated(dxc), which is
    # the public API for this pattern.
    set_property(GLOBAL PROPERTY _FetchContent_dxc_populated TRUE)

    return()
endif()

# ---------------------------------------------------------------------------
# Prebuilt binary download path.
# ---------------------------------------------------------------------------

if(NOT DEFINED SLANG_DXC_BINARY_URL)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(SLANG_DXC_BINARY_URL
            "https://github.com/microsoft/DirectXShaderCompiler/releases/download/${_dxc_version_tag}/dxc_${_dxc_release_date}.zip"
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
            set(SLANG_DXC_BINARY_URL
                "https://github.com/microsoft/DirectXShaderCompiler/releases/download/${_dxc_version_tag}/linux_dxc_${_dxc_release_date}.x86_64.tar.gz"
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
