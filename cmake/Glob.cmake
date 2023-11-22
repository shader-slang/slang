#
# glob_append(MY_VAR my_glob) will append the results of file(GLOB
# CONFIGURE_DEPENDS my_glob) to MY_VAR
#
# Any number of globs may be specified
#
function(glob_append dest)
    file(GLOB files CONFIGURE_DEPENDS ${ARGN})
    list(APPEND ${dest} ${files})
    set(${dest} ${${dest}} PARENT_SCOPE)
endfunction()

#
# Append a glob to a variable conditionally
#
# Example usage: glob_append_if(WIN32 MY_VAR "windows/*.cpp")
#
function(glob_append_if cond dest)
    if(${cond})
        glob_append(${dest} ${ARGN})
        set(${dest} ${${dest}} PARENT_SCOPE)
    endif()
endfunction()

function(slang_glob_sources var)
    file(GLOB_RECURSE files CONFIGURE_DEPENDS ${ARGN})

    if(NOT WIN32)
        list(FILTER files EXCLUDE REGEX "(^|/)windows/.*")
    endif()

    if(NOT UNIX)
        list(FILTER files EXCLUDE REGEX "(^|/)unix/.*")
    endif()

    if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows" AND NOT SLANG_ENABLE_DX_ON_VK)
        list(FILTER files EXCLUDE REGEX "(^|/)d3d.*/.*")
    endif()

    if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows|Linux")
        list(FILTER files EXCLUDE REGEX "(^|/)vulkan/.*")
    endif()

    if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
        list(FILTER files EXCLUDE REGEX "(^|/)open-gl/.*")
    endif()

    list(APPEND ${var} ${files})
    set(${var} ${${var}} PARENT_SCOPE)
endfunction()
