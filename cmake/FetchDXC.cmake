# FetchDXC.cmake
#
# Fetches DXC (DirectXShaderCompiler) prebuilt binaries via FetchContent and
# copies them to the build output directory.
#
# Using FetchContent with the name 'dxc' ensures that slang-rhi (which uses the
# same FetchContent name) will see dxc_POPULATED=TRUE and skip its own download.
#
# Variables:
#   SLANG_DXC_BINARY_URL  - Override the download URL (optional)
#
# Requires the following variables to be set by the caller (set in SlangTarget.cmake):
#   runtime_subdir        - Destination for Windows DLLs (e.g. "bin")
#   library_subdir        - Destination for Linux .so files (e.g. "lib")

include(FetchContent)

if(NOT DEFINED SLANG_DXC_BINARY_URL)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(SLANG_DXC_BINARY_URL
            "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2602/dxc_2026_02_20.zip"
        )
        set(_dxc_url_hash
            "SHA256=a1e89031421cf3c1fca6627766ab3020ca4f962ac7e2caa7fab2b33a8436151e"
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
            set(SLANG_DXC_BINARY_URL
                "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2602/linux_dxc_2026_02_20.x86_64.tar.gz"
            )
            set(_dxc_url_hash
                "SHA256=a1d3e3b5e1c5685b3eb27d5e8890e41d87df45def05112a2d6f1a63a931f7d60"
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
# URL_HASH only applies to the default URLs we ship; if the caller overrides
# SLANG_DXC_BINARY_URL we have no way to know its digest.
if(DEFINED _dxc_url_hash)
    list(APPEND _dxc_fetch_args URL_HASH "${_dxc_url_hash}")
endif()

FetchContent_Declare(${_dxc_fetch_args})

FetchContent_GetProperties(dxc)
if(NOT dxc_POPULATED)
    FetchContent_MakeAvailable(dxc)
endif()

if(SLANG_ENABLE_TESTS)
    # Stage public DXC HLSL headers (e.g. dx/linalg.h) at a stable path so test
    # directives like `-Xdxc -Ibuild/dxc/include` can resolve them. Done as a
    # build-graph custom command, matching the DLL/.so copy pattern below.
    if(EXISTS "${dxc_SOURCE_DIR}/include/hlsl/dx/linalg.h")
        set(_dxc_hlsl_include_dir "${dxc_SOURCE_DIR}/include/hlsl")
    elseif(EXISTS "${dxc_SOURCE_DIR}/inc/hlsl/dx/linalg.h")
        set(_dxc_hlsl_include_dir "${dxc_SOURCE_DIR}/inc/hlsl")
    else()
        message(
            FATAL_ERROR
            "DXC archive at ${SLANG_DXC_BINARY_URL} is missing dx/linalg.h. "
            "The cooperative-{vector,matrix} tests rely on this header. "
            "If Microsoft has reorganized the archive layout, update FetchDXC.cmake."
        )
    endif()
    set(_dxc_inc_src "${_dxc_hlsl_include_dir}/dx/linalg.h")
    set(_dxc_inc_dst "${CMAKE_BINARY_DIR}/dxc/include/dx/linalg.h")
    add_custom_command(
        OUTPUT "${_dxc_inc_dst}"
        COMMAND
            ${CMAKE_COMMAND} -E copy_directory "${_dxc_hlsl_include_dir}"
            "${CMAKE_BINARY_DIR}/dxc/include"
        DEPENDS "${_dxc_inc_src}"
        VERBATIM
    )
    add_custom_target(stage-dxc-headers DEPENDS "${_dxc_inc_dst}")
    set_target_properties(stage-dxc-headers PROPERTIES FOLDER generated)
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
