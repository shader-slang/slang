# Make sure that shared debug info doesn't intefere with caching
# See the sccache readme
message(NOTICE "MSVC: ${MSVC}")
if(MSVC)
    message(NOTICE "MSVC true")
endif()
if(DEFINED CMAKE_MSVC_DEBUG_INFORMATION_FORMAT)
    message(NOTICE "CMAKE_MSVC_DEBUG_INFORMATION_FORMAT is defined")
    message(NOTICE "CMAKE_MSVC_DEBUG_INFORMATION_FORMAT ${CMAKE_MSVC_DEBUG_INFORMATION_FORMAT}")
endif()
message(NOTICE "CMAKE_C_COMPILER_LAUNCHER: ${CMAKE_C_COMPILER_LAUNCHER}")
message(NOTICE "CMAKE_CXX_COMPILER_LAUNCHER: ${CMAKE_CXX_COMPILER_LAUNCHER}")
message(NOTICE "ENV:CMAKE_C_COMPILER_LAUNCHER: $ENV{CMAKE_C_COMPILER_LAUNCHER}")
message(NOTICE "ENV:CMAKE_CXX_COMPILER_LAUNCHER: $ENV{CMAKE_CXX_COMPILER_LAUNCHER}")
if(
    MSVC
    AND (NOT DEFINED CMAKE_MSVC_DEBUG_INFORMATION_FORMAT)
    AND (
        CMAKE_C_COMPILER_LAUNCHER MATCHES "ccache"
        OR CMAKE_CXX_COMPILER_LAUNCHER MATCHES "ccache"
    )
)
    message(
        NOTICE
        "Setting embedded debug info for MSVC to work around (s)ccache's inability to cache shared debug info files, Note that this requires CMake 3.25 or greater"
    )
    cmake_minimum_required(VERSION 3.25)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
        "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>"
    )
endif()
