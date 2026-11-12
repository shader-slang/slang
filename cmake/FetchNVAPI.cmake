# FetchNVAPI.cmake
#
# Fetches NVAPI shader extension headers via FetchContent on Windows.
# Pre-sets FindNVAPI cache variables so auto_option(SLANG_ENABLE_NVAPI) succeeds.
# Creates copy-nvapi-headers target placing headers at build/$<CONFIG>/external/nvapi/
# so slang-test / render-test can find them via TestToolUtil::getRootPath().
#
# Uses FetchContent name 'nvapi' — identical to external/slang-rhi — so that
# slang-rhi's FetchPackage(nvapi ...) sees nvapi_POPULATED=TRUE and skips
# re-downloading (CMake FetchContent deduplication by name).
#
# Variables:
#   SLANG_NVAPI_URL     - Override the download URL (optional)
#   SLANG_GITHUB_TOKEN  - GitHub token for authenticated downloads (optional)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    return()
endif()

include(FetchContent)

if(NOT DEFINED SLANG_NVAPI_URL)
    # Keep this commit hash in sync with SLANG_RHI_NVAPI_URL in
    # external/slang-rhi/CMakeLists.txt
    set(SLANG_NVAPI_URL
        "https://github.com/NVIDIA/nvapi/archive/3d34a4faf095996663321646ebe003539a908f89.zip"
        CACHE STRING "URL for NVAPI download"
    )
endif()

message(STATUS "Fetching NVAPI from: ${SLANG_NVAPI_URL} ...")

set(_nvapi_fetch_args
    nvapi
    URL "${SLANG_NVAPI_URL}"
    SOURCE_SUBDIR _does_not_exist_
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
if(SLANG_GITHUB_TOKEN)
    list(
        APPEND _nvapi_fetch_args
        HTTP_HEADER "Authorization: token ${SLANG_GITHUB_TOKEN}"
    )
endif()

FetchContent_Declare(${_nvapi_fetch_args})
FetchContent_GetProperties(nvapi)
if(NOT nvapi_POPULATED)
    FetchContent_MakeAvailable(nvapi)
endif()

# Pre-set FindNVAPI cache variables so auto_option(SLANG_ENABLE_NVAPI NVAPI ...)
# succeeds without needing to search the removed external/nvapi/ submodule.
set(NVAPI_ROOT_DIR
    "${nvapi_SOURCE_DIR}"
    CACHE PATH "NVAPI root"
    FORCE
)
set(NVAPI_INCLUDE_DIRS
    "${nvapi_SOURCE_DIR}"
    CACHE PATH "NVAPI include dir"
    FORCE
)
set(NVAPI_LIBRARIES
    "${nvapi_SOURCE_DIR}/amd64/nvapi64.lib"
    CACHE FILEPATH "NVAPI library"
    FORCE
)

# Copy HLSL extension headers to build/$<CONFIG>/external/nvapi/ so that
# TestToolUtil::getIncludePath(rootPath, "external/nvapi/nvHLSLExtns.h") resolves
# in both local builds and CI test artifact trees.
set(_nvapi_dst "${CMAKE_BINARY_DIR}/$<CONFIG>/external/nvapi")
set(_nvapi_headers nvHLSLExtns.h nvHLSLExtnsInternal.h nvShaderExtnEnums.h)
set(_nvapi_header_outputs "")

foreach(_hdr IN LISTS _nvapi_headers)
    set(_src "${nvapi_SOURCE_DIR}/${_hdr}")
    set(_dst "${_nvapi_dst}/${_hdr}")
    add_custom_command(
        OUTPUT "${_dst}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_nvapi_dst}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_src}" "${_dst}"
        DEPENDS "${_src}"
        VERBATIM
    )
    list(APPEND _nvapi_header_outputs "${_dst}")
endforeach()

add_custom_target(copy-nvapi-headers DEPENDS ${_nvapi_header_outputs})
set_target_properties(copy-nvapi-headers PROPERTIES FOLDER generated)
