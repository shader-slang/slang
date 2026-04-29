# FetchDXC.cmake
#
# Fetches DXC (DirectXShaderCompiler) prebuilt binaries via FetchContent and
# copies them to the build output directory.
#
# On Linux, the prebuilt binary is downloaded at Slang configure time to detect
# the actual GLIBC version it requires. Both libdxcompiler.so and libdxil.so are
# inspected; if either requires a newer GLIBC than the system provides, DXC is
# built from source instead. The source build also runs cmake-configuration at
# Slang configure time so build-time surprises are avoided.
#
# All configure-time downloads and inspections are cached with stamp files so
# that subsequent cmake reconfigures skip them.
#
# Both paths register the FetchContent 'dxc' name as populated so that slang-rhi
# (which fetches its own DXC under the same name) skips its own download.
#
# Variables:
#   SLANG_DXC_BINARY_URL        - Override the prebuilt binary download URL
#                                 (optional; skips auto-detection when set)
#   SLANG_DXC_BUILD_FROM_SOURCE - ON: always build from source; OFF: always use
#                                 prebuilt (skips detection); unset: auto-detect on
#                                 Linux by downloading the prebuilt binary and
#                                 inspecting the GLIBC requirements of both
#                                 libdxcompiler.so and libdxil.so
#   SLANG_GITHUB_TOKEN          - GitHub token for authenticated downloads (optional)
#
# Requires the following variables to be set by the caller (set in SlangTarget.cmake):
#   runtime_subdir   - Destination for Windows DLLs (e.g. "bin")
#   library_subdir   - Destination for Linux .so files (e.g. "lib")

include(FetchContent)

# DXC version Git tag and release date.
# When upgrading DXC, bump BOTH variables together.
set(_dxc_version_tag "v1.9.2602")
set(_dxc_release_date "2026_02_20")

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
    AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64"
)
    # Auto-detect: download the prebuilt Linux binary and inspect both
    # libdxcompiler.so and libdxil.so to find the highest GLIBC version they
    # require across both libraries. Compare that against the system GLIBC; if
    # the system is too old for either library, fall back to a source build.
    #
    # All steps are cached via stamp files so subsequent reconfigures are fast.

    set(_dxc_probe_url
        "https://github.com/microsoft/DirectXShaderCompiler/releases/download/${_dxc_version_tag}/linux_dxc_${_dxc_release_date}.x86_64.tar.gz"
    )
    set(_dxc_probe_dir "${CMAKE_BINARY_DIR}/_dxc_probe")
    set(_dxc_probe_tarball "${_dxc_probe_dir}/dxc.tar.gz")
    # Stamp stores the highest GLIBC version required across both .so files.
    # Written only after a successful detection; never contains "0.0".
    set(_dxc_glibc_stamp "${_dxc_probe_dir}/req_glibc_${_dxc_version_tag}.txt")

    set(_dxc_required_glibc "0.0")
    if(EXISTS "${_dxc_glibc_stamp}")
        file(READ "${_dxc_glibc_stamp}" _dxc_required_glibc)
        string(STRIP "${_dxc_required_glibc}" _dxc_required_glibc)
    else()
        file(MAKE_DIRECTORY "${_dxc_probe_dir}")

        # Download the tarball once; reused by FetchContent for the actual
        # binary deployment if the system GLIBC turns out to be new enough.
        if(NOT EXISTS "${_dxc_probe_tarball}")
            message(
                STATUS
                "Downloading DXC prebuilt binary to detect GLIBC requirement..."
            )
            set(_dl_headers "")
            if(SLANG_GITHUB_TOKEN)
                set(_dl_headers
                    HTTPHEADER
                    "Authorization: token ${SLANG_GITHUB_TOKEN}"
                )
            endif()
            file(
                DOWNLOAD "${_dxc_probe_url}" "${_dxc_probe_tarball}"
                STATUS _dl_status
                SHOW_PROGRESS
                ${_dl_headers}
            )
            list(GET _dl_status 0 _dl_code)
            if(NOT _dl_code EQUAL 0)
                list(GET _dl_status 1 _dl_msg)
                message(
                    WARNING
                    "Failed to download DXC prebuilt binary: ${_dl_msg}. "
                    "GLIBC requirement detection skipped. "
                    "Set -DSLANG_DXC_BUILD_FROM_SOURCE=ON if DXC is required."
                )
                file(REMOVE "${_dxc_probe_tarball}")
            endif()
        endif()

        if(EXISTS "${_dxc_probe_tarball}")
            # Extract only the two shared libraries that determine runtime
            # compatibility; avoids unpacking the full ~30 MB tarball.
            execute_process(
                COMMAND
                    ${CMAKE_COMMAND} -E tar xzf "${_dxc_probe_tarball}"
                    lib/libdxcompiler.so lib/libdxil.so
                WORKING_DIRECTORY "${_dxc_probe_dir}"
                OUTPUT_QUIET
                ERROR_QUIET
            )

            find_program(_dxc_objdump NAMES objdump)
            find_program(_dxc_readelf NAMES readelf)

            # Inspect each library and track the highest GLIBC version found
            # across both. If either library requires GLIBC X.Y, the system
            # must provide at least X.Y for both to load.
            foreach(_lib libdxcompiler.so libdxil.so)
                set(_so "${_dxc_probe_dir}/lib/${_lib}")
                if(NOT EXISTS "${_so}")
                    continue()
                endif()
                set(_elf_info "")
                if(_dxc_objdump)
                    execute_process(
                        COMMAND "${_dxc_objdump}" -p "${_so}"
                        OUTPUT_VARIABLE _elf_info
                        ERROR_QUIET
                    )
                elseif(_dxc_readelf)
                    execute_process(
                        COMMAND "${_dxc_readelf}" --version-info "${_so}"
                        OUTPUT_VARIABLE _elf_info
                        ERROR_QUIET
                    )
                endif()
                string(
                    REGEX MATCHALL
                    "GLIBC_([0-9]+[.][0-9]+)"
                    _ver_list
                    "${_elf_info}"
                )
                foreach(_entry ${_ver_list})
                    string(REGEX REPLACE "^GLIBC_" "" _ver "${_entry}")
                    if(_ver VERSION_GREATER _dxc_required_glibc)
                        set(_dxc_required_glibc "${_ver}")
                    endif()
                endforeach()
            endforeach()

            if(NOT _dxc_required_glibc STREQUAL "0.0")
                message(
                    STATUS
                    "DXC prebuilt binary (${_dxc_version_tag}) "
                    "requires GLIBC >= ${_dxc_required_glibc}"
                )
                file(WRITE "${_dxc_glibc_stamp}" "${_dxc_required_glibc}")
            else()
                message(
                    WARNING
                    "Could not extract GLIBC version requirements from DXC "
                    "shared libraries (objdump/readelf unavailable or produced "
                    "no output). Prebuilt binaries may silently fail to load "
                    "if the system GLIBC is older than required. "
                    "Set -DSLANG_DXC_BUILD_FROM_SOURCE=ON to force a source build."
                )
            endif()
        endif()
    endif()

    # Compare the detected requirement against the system GLIBC.
    if(NOT _dxc_required_glibc STREQUAL "0.0")
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
        string(
            REGEX MATCH
            "[Gg][Ll][Ii][Bb][Cc][^0-9]*([0-9]+)\\.([0-9]+)"
            _glibc_match
            "${_libc_probe}"
        )
        if(_glibc_match)
            set(_glibc_version "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}")
            message(STATUS "Detected system GLIBC version: ${_glibc_version}")
            if(_glibc_version VERSION_LESS _dxc_required_glibc)
                message(
                    STATUS
                    "System GLIBC ${_glibc_version} < required "
                    "${_dxc_required_glibc}: building DXC from source "
                    "(${_dxc_version_tag})"
                )
                set(_dxc_build_from_source ON)
            endif()
        else()
            message(
                WARNING
                "Could not detect system GLIBC version; "
                "prebuilt DXC binaries may not run on this system. "
                "Set -DSLANG_DXC_BUILD_FROM_SOURCE=ON to build from source."
            )
        endif()
    endif()
elseif(
    NOT DEFINED SLANG_DXC_BUILD_FROM_SOURCE
    AND CMAKE_SYSTEM_NAME STREQUAL "Linux"
    AND NOT CMAKE_CROSSCOMPILING
    AND NOT DEFINED SLANG_DXC_BINARY_URL
)
    # Non-x86_64 Linux: no prebuilt binary is available.
    message(
        WARNING
        "No prebuilt DXC binary is available for ${CMAKE_SYSTEM_PROCESSOR} "
        "on Linux. "
        "Set -DSLANG_DXC_BUILD_FROM_SOURCE=ON to build DXC from source."
    )
endif()

# ---------------------------------------------------------------------------
# Source build: clone and cmake-configure at Slang configure time.
# ---------------------------------------------------------------------------

if(_dxc_build_from_source)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(_dxc_warning_flags "")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
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
        set(_dxc_dll_subdir "bin")
    else()
        set(_dxc_generator_args -G "${CMAKE_GENERATOR}")
        if(CMAKE_GENERATOR_PLATFORM)
            list(APPEND _dxc_generator_args -A "${CMAKE_GENERATOR_PLATFORM}")
        endif()
        if(CMAKE_GENERATOR_TOOLSET)
            list(APPEND _dxc_generator_args -T "${CMAKE_GENERATOR_TOOLSET}")
        endif()
        set(_dxc_dll_subdir "MinSizeRel/bin")
    endif()

    # Step 1: Clone DXC source at Slang configure time.
    # FetchContent stamps ensure the clone is skipped on subsequent reconfigures.
    FetchContent_Declare(
        dxc_source
        GIT_REPOSITORY "https://github.com/microsoft/DirectXShaderCompiler.git"
        GIT_TAG "${_dxc_version_tag}"
        GIT_SHALLOW ON
        GIT_SUBMODULES_RECURSE ON
        GIT_PROGRESS ON
        UPDATE_DISCONNECTED ON
        SOURCE_SUBDIR
        _does_not_exist_
    )
    FetchContent_MakeAvailable(dxc_source)
    set(_dxc_src_dir "${dxc_source_SOURCE_DIR}")
    set(_dxc_build_dir "${dxc_source_BINARY_DIR}")

    # DXC's build runs clang-format on generated files inside its build
    # directory. Clang-format walks up from there and finds Slang's root
    # .clang-format, which uses options unsupported by older clang-format
    # versions. Placing DXC's own style here stops the upward search.
    file(MAKE_DIRECTORY "${_dxc_build_dir}")
    file(WRITE "${_dxc_build_dir}/.clang-format" "BasedOnStyle: LLVM\n")

    # Step 2: Configure DXC at Slang configure time.
    # A stamp file keyed on the version tag makes this idempotent: the
    # configure step is re-run only when the DXC version changes.
    set(_dxc_configure_stamp
        "${_dxc_build_dir}/.slang_dxc_configured_${_dxc_version_tag}"
    )
    if(NOT EXISTS "${_dxc_configure_stamp}")
        message(STATUS "Configuring DXC from source (${_dxc_version_tag})...")
        execute_process(
            COMMAND
                ${CMAKE_COMMAND} -B "${_dxc_build_dir}" -S "${_dxc_src_dir}"
                ${_dxc_generator_args} -C
                "${_dxc_src_dir}/cmake/caches/PredefinedParams.cmake"
                # CMAKE_BUILD_TYPE is ignored by multi-config generators (VS);
                # the config is selected via --config MinSizeRel in the build
                # command. Passing it here covers single-config generators (Ninja).
                -DCMAKE_BUILD_TYPE=MinSizeRel -DHLSL_COPY_GENERATED_SOURCES=ON
                -DLLVM_INCLUDE_TESTS=OFF -DCLANG_INCLUDE_TESTS=OFF
                -DLLVM_ENABLE_WARNINGS=OFF ${_dxc_warning_flags} -Wno-dev
            RESULT_VARIABLE _dxc_configure_result
            OUTPUT_VARIABLE _dxc_configure_output
            ERROR_VARIABLE _dxc_configure_error
        )
        if(NOT _dxc_configure_result EQUAL 0)
            message(
                FATAL_ERROR
                "DXC cmake configure failed "
                "(exit ${_dxc_configure_result}):\n"
                "${_dxc_configure_output}\n${_dxc_configure_error}"
            )
        endif()
        file(WRITE "${_dxc_configure_stamp}" "${_dxc_version_tag}")
        message(STATUS "DXC configured successfully")
    endif()

    # Step 3: Build DXC at Slang build time.
    # The DXIL validator target is named 'dxildll' in DXC's CMake (it produces
    # libdxil.so on Linux / dxil.dll on Windows).
    if(CMAKE_GENERATOR MATCHES "Ninja")
        set(_dxc_build_cmd
            ${CMAKE_COMMAND}
            --build
            "${_dxc_build_dir}"
            --target
            dxcompiler
            dxildll
        )
    else()
        set(_dxc_build_cmd
            ${CMAKE_COMMAND}
            --build
            "${_dxc_build_dir}"
            --config
            MinSizeRel
            --target
            dxcompiler
            --target
            dxildll
        )
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(_dxc_src_byproducts
            "${_dxc_build_dir}/${_dxc_dll_subdir}/dxcompiler.dll"
            "${_dxc_build_dir}/${_dxc_dll_subdir}/dxil.dll"
        )
    else()
        set(_dxc_src_byproducts
            "${_dxc_build_dir}/lib/libdxcompiler.so"
            "${_dxc_build_dir}/lib/libdxil.so"
        )
    endif()

    add_custom_target(
        dxc_from_source
        COMMAND ${_dxc_build_cmd}
        BYPRODUCTS ${_dxc_src_byproducts}
        COMMENT "Building DXC ${_dxc_version_tag} from source"
        VERBATIM
    )

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        foreach(_dll dxcompiler dxil)
            set(_src "${_dxc_build_dir}/${_dxc_dll_subdir}/${_dll}.dll")
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
            set(_src "${_dxc_build_dir}/lib/lib${_lib}.so")
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
    # the source build (which owns the actual clone), and setting the
    # public-facing dxc_POPULATED variable in this scope would not propagate to
    # the calling scope where slang-rhi's FetchPackage runs.
    #
    # On Linux, slang-rhi's DXC fetch is guarded by if(WIN32) and is never
    # reached, so setting the population flag is sufficient.
    # On Windows with this source-build path, slang-rhi will see
    # dxc_POPULATED=TRUE and skip its own download, but its subsequent
    # copy_file(${dxc_SOURCE_DIR}/bin/x64/...) step will get an empty
    # dxc_SOURCE_DIR. This is a known limitation of the Windows source-build
    # path. Slang's own copy-dxcompiler/copy-dxil targets (defined above)
    # correctly deploy the source-built DLLs regardless, so DXC is available
    # to the compiler at runtime even if slang-rhi's copy step fails.
    #
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

message(STATUS "Fetching DXC prebuilt binary from: ${SLANG_DXC_BINARY_URL} ...")
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
