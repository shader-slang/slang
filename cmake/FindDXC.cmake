# Locate an installed DXC for the SLANG_USE_SYSTEM_DXC path. The fetch /
# source-build path is in FetchDXC.cmake.
#
# Set DXC_ROOT or CMAKE_PREFIX_PATH to provide an installation prefix.
#
# Output cache variables:
#   DXC_INCLUDE_DIRS         directory containing dxc/dxcapi.h
#   DXC_DXCOMPILER_RUNTIME   shared library copied next to Slang binaries
#   DXC_DXIL_RUNTIME         dxil.dll on Windows; optional shared library
#                            on Linux and macOS
#   DXC_DXC_EXECUTABLE       optional, used for best-effort version detection
#   DXC_VERSION              detected version, or unset
#
# Version detection is best-effort: DXC has no compile-time version macro
# and IDxcVersionInfo is a runtime COM interface. A mismatch against the
# SLANG_DXC_VERSION_TAG pin is a warning, never fatal -- the user opted
# into their own DXC and owns compatibility.

include(DXCMetadata)
include(FindPackageHandleStandardArgs)

find_path(DXC_INCLUDE_DIRS NAMES dxc/dxcapi.h PATH_SUFFIXES include)

# Slang loads DXC dynamically and only stages its runtime shared libraries;
# import libraries are not build inputs on this path.
if(WIN32)
    find_file(DXC_DXCOMPILER_RUNTIME NAMES dxcompiler.dll PATH_SUFFIXES bin)
    find_file(DXC_DXIL_RUNTIME NAMES dxil.dll PATH_SUFFIXES bin)
else()
    # NAMES is the bare name; CMake adds the platform prefix/suffix.
    find_library(DXC_DXCOMPILER_RUNTIME NAMES dxcompiler PATH_SUFFIXES lib)
    find_library(DXC_DXIL_RUNTIME NAMES dxil PATH_SUFFIXES lib)
endif()

find_program(DXC_DXC_EXECUTABLE NAMES dxc)

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

set(_dxc_required_vars DXC_INCLUDE_DIRS DXC_DXCOMPILER_RUNTIME)
if(WIN32)
    list(APPEND _dxc_required_vars DXC_DXIL_RUNTIME)
endif()

find_package_handle_standard_args(
    DXC
    REQUIRED_VARS ${_dxc_required_vars}
    VERSION_VAR DXC_VERSION
)

mark_as_advanced(
    DXC_INCLUDE_DIRS
    DXC_DXCOMPILER_RUNTIME
    DXC_DXIL_RUNTIME
    DXC_DXC_EXECUTABLE
)
