set(Optix_ROOT_DIR "" CACHE PATH "Path to an installed OptiX SDK")

# First try the git submodule location
find_path(
    OptiX_INCLUDE_DIRS
    NAMES optix.h
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}/external/optix-dev/include"
    NO_DEFAULT_PATH
)

# If not found and a custom path is provided, try that
if(NOT OptiX_INCLUDE_DIRS AND Optix_ROOT_DIR)
    find_path(
        OptiX_INCLUDE_DIRS
        NAMES optix.h
        PATH_SUFFIXES include
        PATHS "${Optix_ROOT_DIR}"
        NO_DEFAULT_PATH
    )
endif()

# Finally, try system paths as fallback
if(NOT OptiX_INCLUDE_DIRS)
    find_path(OptiX_INCLUDE_DIRS NAMES optix.h)
endif()

find_package_handle_standard_args(OptiX REQUIRED_VARS OptiX_INCLUDE_DIRS)
