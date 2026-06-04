# Locate an installed DXC. Shape matches FindNVAPI / FindAftermath; used
# only on the SLANG_USE_SYSTEM_DXC path. The fetch / source-build path is
# in FetchDXC.cmake.
#
# Output cache variables:
#   DXC_INCLUDE_DIRS         directory containing dxc/dxcapi.h
#   DXC_DXCOMPILER_LIBRARY   linker-facing library (.lib on Windows,
#                            .so/.dylib elsewhere)
#   DXC_DXIL_LIBRARY         same for dxil; required on Windows only
#                            (DXIL signing is Windows-only and dxil is not
#                            redistributed for Linux/macOS)
#   DXC_DXCOMPILER_RUNTIME   loader-facing artifact that the build copies
#                            next to the binaries: dxcompiler.dll on
#                            Windows, same as the .so/.dylib otherwise
#   DXC_DXIL_RUNTIME         same for dxil
#   DXC_DXC_EXECUTABLE       optional, used for best-effort version detection
#   DXC_VERSION              detected version, or unset
#
# Version detection is best-effort: DXC has no compile-time version macro
# and IDxcVersionInfo is a runtime COM interface. A mismatch against the
# SLANG_DXC_VERSION_TAG pin is a warning, never fatal -- the user opted
# into their own DXC and owns compatibility.

include(DXCMetadata)
include(FindPackageHandleStandardArgs)

set(DXC_ROOT_DIR "" CACHE PATH "Path to an installed DXC SDK")

if(DXC_ROOT_DIR)
    find_path(
        DXC_INCLUDE_DIRS
        NAMES dxc/dxcapi.h
        PATH_SUFFIXES include
        PATHS "${DXC_ROOT_DIR}"
        NO_DEFAULT_PATH
    )
else()
    find_path(DXC_INCLUDE_DIRS NAMES dxc/dxcapi.h PATH_SUFFIXES include)
endif()

# NAMES is the bare name; CMake adds the platform prefix/suffix.
if(DXC_ROOT_DIR)
    find_library(
        DXC_DXCOMPILER_LIBRARY
        NAMES dxcompiler
        PATH_SUFFIXES lib bin
        PATHS "${DXC_ROOT_DIR}"
        NO_DEFAULT_PATH
    )
    find_library(
        DXC_DXIL_LIBRARY
        NAMES dxil
        PATH_SUFFIXES lib bin
        PATHS "${DXC_ROOT_DIR}"
        NO_DEFAULT_PATH
    )
else()
    find_library(DXC_DXCOMPILER_LIBRARY NAMES dxcompiler PATH_SUFFIXES lib bin)
    find_library(DXC_DXIL_LIBRARY NAMES dxil PATH_SUFFIXES lib bin)
endif()

# On Windows find_library returns the .lib import library; locate the .dll
# separately so staging copies the runtime artifact. Elsewhere the
# find_library result is already the runtime shared object.
if(WIN32)
    if(DXC_ROOT_DIR)
        find_file(
            DXC_DXCOMPILER_RUNTIME
            NAMES dxcompiler.dll
            PATH_SUFFIXES bin
            PATHS "${DXC_ROOT_DIR}"
            NO_DEFAULT_PATH
        )
        find_file(
            DXC_DXIL_RUNTIME
            NAMES dxil.dll
            PATH_SUFFIXES bin
            PATHS "${DXC_ROOT_DIR}"
            NO_DEFAULT_PATH
        )
    else()
        find_file(DXC_DXCOMPILER_RUNTIME NAMES dxcompiler.dll PATH_SUFFIXES bin)
        find_file(DXC_DXIL_RUNTIME NAMES dxil.dll PATH_SUFFIXES bin)
    endif()
else()
    set(DXC_DXCOMPILER_RUNTIME "${DXC_DXCOMPILER_LIBRARY}")
    set(DXC_DXIL_RUNTIME "${DXC_DXIL_LIBRARY}")
endif()

if(DXC_ROOT_DIR)
    find_program(
        DXC_DXC_EXECUTABLE
        NAMES dxc
        PATH_SUFFIXES bin
        PATHS "${DXC_ROOT_DIR}"
        NO_DEFAULT_PATH
    )
else()
    find_program(DXC_DXC_EXECUTABLE NAMES dxc)
endif()

if(DXC_DXC_EXECUTABLE)
    execute_process(
        COMMAND "${DXC_DXC_EXECUTABLE}" --version
        OUTPUT_VARIABLE _dxc_version_output
        ERROR_VARIABLE _dxc_version_output
        OUTPUT_STRIP_TRAILING_WHITESPACE
        TIMEOUT 10
    )
    string(
        REGEX MATCH
        "[0-9]+\\.[0-9]+\\.[0-9]+"
        DXC_VERSION
        "${_dxc_version_output}"
    )
    if(DXC_VERSION)
        string(REGEX REPLACE "^v" "" _dxc_expected "${SLANG_DXC_VERSION_TAG}")
        if(NOT DXC_VERSION STREQUAL _dxc_expected)
            message(
                WARNING
                "System DXC version ${DXC_VERSION} does not match the "
                "version Slang is tested against (${_dxc_expected}); "
                "features that depend on specific DXC behavior may fail."
            )
        endif()
    endif()
endif()

set(_dxc_required_vars
    DXC_INCLUDE_DIRS
    DXC_DXCOMPILER_LIBRARY
    DXC_DXCOMPILER_RUNTIME
)
if(WIN32)
    list(APPEND _dxc_required_vars DXC_DXIL_LIBRARY DXC_DXIL_RUNTIME)
endif()

find_package_handle_standard_args(
    DXC
    REQUIRED_VARS ${_dxc_required_vars}
    VERSION_VAR DXC_VERSION
)

mark_as_advanced(
    DXC_INCLUDE_DIRS
    DXC_DXCOMPILER_LIBRARY
    DXC_DXIL_LIBRARY
    DXC_DXCOMPILER_RUNTIME
    DXC_DXIL_RUNTIME
    DXC_DXC_EXECUTABLE
)
