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
    DEFINED SLANG_DXC_BUILD_FROM_SOURCE
    AND NOT SLANG_DXC_BUILD_FROM_SOURCE
    AND CMAKE_SYSTEM_NAME STREQUAL "Linux"
    AND NOT CMAKE_CROSSCOMPILING
    AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64"
)
    # User explicitly disabled the source build on x86_64 Linux; skip
    # auto-detection and use prebuilt binaries without any GLIBC check.
    # Non-x86_64 Linux falls through to the dedicated architecture warning
    # below (no prebuilt binary exists for those architectures).
    message(
        STATUS
        "SLANG_DXC_BUILD_FROM_SOURCE=OFF: using prebuilt DXC without "
        "GLIBC compatibility check — prebuilt binaries may fail to load "
        "if the system GLIBC is older than required."
    )
elseif(
    NOT DEFINED SLANG_DXC_BUILD_FROM_SOURCE
    AND CMAKE_SYSTEM_NAME STREQUAL "Linux"
    AND NOT CMAKE_CROSSCOMPILING
    AND DEFINED SLANG_DXC_BINARY_URL
)
    # User supplied a custom URL; skip GLIBC auto-detection and use it as-is.
    message(
        STATUS
        "SLANG_DXC_BINARY_URL set: skipping GLIBC auto-detection, "
        "using custom prebuilt URL."
    )
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
    # Include the version tag in the filename so that bumping _dxc_version_tag
    # invalidates the cached tarball and forces a fresh download rather than
    # re-using a stale archive from a previous version.
    set(_dxc_probe_tarball "${_dxc_probe_dir}/dxc_${_dxc_version_tag}.tar.gz")
    # Stamp stores the highest GLIBC version required across both .so files
    # (e.g. "2.34"), or the sentinel "DETECTION_FAILED" when objdump/readelf
    # could not extract version info.
    set(_dxc_glibc_stamp "${_dxc_probe_dir}/req_glibc_${_dxc_version_tag}.txt")

    set(_dxc_required_glibc "0.0")
    if(EXISTS "${_dxc_glibc_stamp}")
        file(READ "${_dxc_glibc_stamp}" _dxc_required_glibc)
        string(STRIP "${_dxc_required_glibc}" _dxc_required_glibc)
        # "DETECTION_FAILED" is written when objdump/readelf produced no
        # output; treat it the same as "0.0" so we skip the system check
        # without re-running the (failing) inspection on every reconfigure.
        if(_dxc_required_glibc STREQUAL "DETECTION_FAILED")
            set(_dxc_required_glibc "0.0")
        endif()
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
                RESULT_VARIABLE _tar_result
                OUTPUT_QUIET
                ERROR_VARIABLE _tar_error
            )
            if(NOT _tar_result EQUAL 0)
                message(
                    WARNING
                    "Failed to extract shared libraries from DXC prebuilt "
                    "tarball (exit ${_tar_result}): ${_tar_error}. "
                    "GLIBC requirement detection skipped. "
                    "Set -DSLANG_DXC_BUILD_FROM_SOURCE=ON if DXC is required."
                )
                # Remove the tarball so a fresh download is attempted on the
                # next reconfigure rather than retrying with a bad archive.
                file(REMOVE "${_dxc_probe_tarball}")
            endif()

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
                # Write a sentinel so subsequent reconfigures skip the
                # inspection and avoid repeating this warning.
                file(WRITE "${_dxc_glibc_stamp}" "DETECTION_FAILED")
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
                ERROR_VARIABLE _libc_probe_stderr
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_STRIP_TRAILING_WHITESPACE
            )
            # Some distributions (e.g. older CentOS/RHEL) print the version
            # to stderr rather than stdout; fall back to stderr when stdout
            # is empty.
            if(NOT _libc_probe)
                set(_libc_probe "${_libc_probe_stderr}")
            endif()
        endif()
        # Detect musl libc before attempting the glibc regex: prebuilt DXC
        # binaries are glibc-linked and will definitely not load on musl
        # systems, so force a source build immediately.
        if(
            _libc_probe MATCHES "[Mm]usl"
            OR EXISTS "/lib/ld-musl-x86_64.so.1"
            OR EXISTS "/lib/ld-musl-aarch64.so.1"
        )
            message(
                STATUS
                "Detected musl libc; prebuilt DXC binaries require glibc "
                "and will not load. Building DXC from source "
                "(${_dxc_version_tag})."
            )
            set(_dxc_build_from_source ON)
        else()
            string(
                REGEX MATCH
                "[Gg][Ll][Ii][Bb][Cc][^0-9]*([0-9]+)\\.([0-9]+)"
                _glibc_match
                "${_libc_probe}"
            )
            if(_glibc_match)
                set(_glibc_version "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}")
                message(
                    STATUS
                    "Detected system GLIBC version: ${_glibc_version}"
                )
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
                    "Could not detect system GLIBC version (getconf and ldd "
                    "both failed or produced unrecognisable output). "
                    "Proceeding with prebuilt DXC binaries; if the system libc "
                    "is older than the binaries require, they will fail to load "
                    "at runtime with a dynamic-linker error. "
                    "Set -DSLANG_DXC_BUILD_FROM_SOURCE=ON to build from source "
                    "and guarantee compatibility."
                )
            endif()
        endif()
    endif()
elseif(
    CMAKE_SYSTEM_NAME STREQUAL "Linux"
    AND NOT CMAKE_CROSSCOMPILING
    AND NOT DEFINED SLANG_DXC_BINARY_URL
    AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64"
)
    # Non-x86_64 Linux: no prebuilt binary is available for this architecture,
    # regardless of whether SLANG_DXC_BUILD_FROM_SOURCE was explicitly set to
    # OFF or left unset. Warn and return immediately so the downstream URL
    # check (which would also return early) is not the only signal that DXC
    # is unavailable.
    if(DEFINED SLANG_DXC_BUILD_FROM_SOURCE AND NOT SLANG_DXC_BUILD_FROM_SOURCE)
        message(
            WARNING
            "SLANG_DXC_BUILD_FROM_SOURCE=OFF but no prebuilt DXC binary is "
            "available for ${CMAKE_SYSTEM_PROCESSOR} on Linux. "
            "DXC will be unavailable. "
            "Set -DSLANG_DXC_BUILD_FROM_SOURCE=ON to build DXC from source, "
            "or set -DSLANG_DXC_BINARY_URL=<url> to use a custom prebuilt binary."
        )
        return()
    else()
        message(
            WARNING
            "No prebuilt DXC binary is available for ${CMAKE_SYSTEM_PROCESSOR} "
            "on Linux. "
            "Set -DSLANG_DXC_BUILD_FROM_SOURCE=ON to build DXC from source, "
            "or set -DSLANG_DXC_BINARY_URL=<url> to use a custom prebuilt binary."
        )
        return()
    endif()
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
    # DXC includes LLVM/Clang as submodules; the first clone can take
    # 10–30 minutes depending on network speed, and the subsequent build
    # adds another 10–30 minutes. Subsequent reconfigures and incremental
    # builds skip both steps via stamp files and add_custom_command OUTPUT
    # tracking.
    message(
        STATUS
        "Cloning DXC ${_dxc_version_tag} from source "
        "(first run: ~500 MB download + 10-30 min build; "
        "subsequent runs are skipped via stamp files)..."
    )
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

    # If HLSL_COPY_GENERATED_SOURCES were ever re-enabled, DXC's build would run
    # clang-format on generated files and walk up to find .clang-format. Slang's
    # root .clang-format uses options unsupported by older clang-format versions,
    # so we place DXC's own style here to stop the upward search at the right level.
    file(MAKE_DIRECTORY "${_dxc_build_dir}")
    file(WRITE "${_dxc_build_dir}/.clang-format" "BasedOnStyle: LLVM\n")

    # Step 2: Configure DXC at Slang configure time.
    # The stamp is keyed on version tag, generator, toolset, and compiler so
    # that switching any of these invalidates the cached configure. A SHA256
    # hash keeps the filename short regardless of path lengths.
    string(
        SHA256
        _dxc_config_hash
        "${_dxc_version_tag}_${CMAKE_GENERATOR}_${CMAKE_GENERATOR_TOOLSET}_${CMAKE_CXX_COMPILER}"
    )
    set(_dxc_configure_stamp
        "${_dxc_build_dir}/.slang_dxc_configured_${_dxc_config_hash}"
    )
    if(NOT EXISTS "${_dxc_configure_stamp}")
        message(STATUS "Configuring DXC from source (${_dxc_version_tag})...")
        execute_process(
            COMMAND
                ${CMAKE_COMMAND} -B "${_dxc_build_dir}" -S "${_dxc_src_dir}"
                ${_dxc_generator_args} -C
                "${_dxc_src_dir}/cmake/caches/PredefinedParams.cmake"
                # Forward parent compilers so DXC uses the same toolchain.
                "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
                "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
                # CMAKE_BUILD_TYPE is ignored by multi-config generators (VS);
                # the config is selected via --config MinSizeRel in the build
                # command. Passing it here covers single-config generators (Ninja).
                -DCMAKE_BUILD_TYPE=MinSizeRel -DHLSL_COPY_GENERATED_SOURCES=OFF
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

    # Use add_custom_command(OUTPUT) rather than add_custom_target so the
    # inner DXC build is only re-invoked when the output libraries do not
    # exist. add_custom_target is always considered out-of-date and would
    # run cmake --build on every incremental Slang build.
    add_custom_command(
        OUTPUT ${_dxc_src_byproducts}
        COMMAND ${_dxc_build_cmd}
        COMMENT "Building DXC ${_dxc_version_tag} from source"
        VERBATIM
    )
    add_custom_target(dxc_from_source DEPENDS ${_dxc_src_byproducts})

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        foreach(_dll dxcompiler dxil)
            set(_src "${_dxc_build_dir}/${_dxc_dll_subdir}/${_dll}.dll")
            set(_dst
                "${CMAKE_BINARY_DIR}/$<CONFIG>/${runtime_subdir}/${_dll}.dll"
            )
            add_custom_command(
                OUTPUT "${_dst}"
                COMMAND
                    ${CMAKE_COMMAND} -E copy_if_different "${_src}" "${_dst}"
                DEPENDS "${_src}"
                VERBATIM
            )
            add_custom_target(copy-${_dll} DEPENDS "${_dst}")
            set_target_properties(copy-${_dll} PROPERTIES FOLDER generated)
        endforeach()
    else()
        foreach(_lib dxcompiler dxil)
            set(_src "${_dxc_build_dir}/lib/lib${_lib}.so")
            set(_dst
                "${CMAKE_BINARY_DIR}/$<CONFIG>/${library_subdir}/lib${_lib}.so"
            )
            add_custom_command(
                OUTPUT "${_dst}"
                COMMAND
                    ${CMAKE_COMMAND} -E copy_if_different "${_src}" "${_dst}"
                DEPENDS "${_src}"
                VERBATIM
            )
            add_custom_target(copy-${_lib} DEPENDS "${_dst}")
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
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        message(
            WARNING
            "SLANG_DXC_BUILD_FROM_SOURCE=ON on Windows: slang-rhi's DXC "
            "copy step may fail because dxc_SOURCE_DIR is not set in the "
            "source-build path. Slang's own copy-dxcompiler/copy-dxil "
            "targets still deploy the built DLLs correctly."
        )
    endif()
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
            # Reuse the probe tarball already downloaded during GLIBC detection
            # to avoid fetching the same file twice.
            if(DEFINED _dxc_probe_tarball AND EXISTS "${_dxc_probe_tarball}")
                set(SLANG_DXC_BINARY_URL "file://${_dxc_probe_tarball}")
            else()
                set(SLANG_DXC_BINARY_URL
                    "https://github.com/microsoft/DirectXShaderCompiler/releases/download/${_dxc_version_tag}/linux_dxc_${_dxc_release_date}.x86_64.tar.gz"
                )
            endif()
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
