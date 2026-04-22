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
#   SLANG_GITHUB_TOKEN    - GitHub token for authenticated downloads (optional)
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
