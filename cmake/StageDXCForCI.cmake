# Invoked via `cmake -P` from the option-matrix CI workflow as the
# pre_configure step for SLANG_USE_SYSTEM_DXC, which needs an actual DXC
# install on the runner before find_package(DXC REQUIRED) can succeed.
# Reuses DXCMetadata.cmake so URL/hash never drift from the pinned tag.
#
# After this script runs, pass -DDXC_ROOT_DIR=$RUNNER_TEMP/dxc to configure.

cmake_minimum_required(VERSION 3.22)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
include(DXCMetadata)

if(DEFINED ENV{RUNNER_TEMP} AND NOT "$ENV{RUNNER_TEMP}" STREQUAL "")
    set(_dest "$ENV{RUNNER_TEMP}/dxc")
else()
    set(_dest "${CMAKE_BINARY_DIR}/_dxc_ci")
endif()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(_url "${SLANG_DXC_WINDOWS_URL}")
    set(_hash "${SLANG_DXC_WINDOWS_SHA256}")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(_url "${SLANG_DXC_LINUX_URL}")
    set(_hash "${SLANG_DXC_LINUX_SHA256}")
else()
    message(
        FATAL_ERROR
        "StageDXCForCI: no Microsoft DXC prebuilt for "
        "${CMAKE_HOST_SYSTEM_NAME}; the SLANG_USE_SYSTEM_DXC matrix entry "
        "should skip this host."
    )
endif()

file(MAKE_DIRECTORY "${_dest}")
set(_archive "${_dest}/dxc-prebuilt-archive")

message(STATUS "StageDXCForCI: downloading ${_url}")
file(
    DOWNLOAD "${_url}"
    "${_archive}"
    EXPECTED_HASH "SHA256=${_hash}"
    TLS_VERIFY ON
    STATUS _dl_status
    SHOW_PROGRESS
)
list(GET _dl_status 0 _dl_code)
if(NOT _dl_code EQUAL 0)
    list(GET _dl_status 1 _dl_msg)
    message(FATAL_ERROR "StageDXCForCI: download failed: ${_dl_msg}")
endif()

file(ARCHIVE_EXTRACT INPUT "${_archive}" DESTINATION "${_dest}")
file(REMOVE "${_archive}")

# The Windows archive nests artifacts under bin/<arch>/ and lib/<arch>/,
# and uses inc/ rather than include/. Flatten to bin/, lib/, include/ so
# FindDXC's PATH_SUFFIXES resolve against -DDXC_ROOT_DIR=$RUNNER_TEMP/dxc.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "ARM64|arm64|aarch64")
        set(_dxc_arch "arm64")
    else()
        set(_dxc_arch "x64")
    endif()
    foreach(_subdir bin lib)
        if(EXISTS "${_dest}/${_subdir}/${_dxc_arch}")
            file(GLOB _arch_files "${_dest}/${_subdir}/${_dxc_arch}/*")
            foreach(_f IN LISTS _arch_files)
                get_filename_component(_name "${_f}" NAME)
                file(RENAME "${_f}" "${_dest}/${_subdir}/${_name}")
            endforeach()
        endif()
    endforeach()
    if(EXISTS "${_dest}/inc" AND NOT EXISTS "${_dest}/include")
        file(RENAME "${_dest}/inc" "${_dest}/include")
    endif()
endif()

message(STATUS "StageDXCForCI: DXC staged at ${_dest}")
