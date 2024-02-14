# Don't let all these packages spam up with status messages, filter to only the
# important stuff.
# Don't change things however if the user has explicitly set
# CMAKE_MESSAGE_LOG_LEVEL
set(set_cmake_log_level FALSE)
if(NOT CMAKE_MESSAGE_LOG_LEVEL)
    set(set_cmake_log_level TRUE)
    set(CMAKE_MESSAGE_LOG_LEVEL NOTICE)
endif()

# Miniz
add_subdirectory(miniz EXCLUDE_FROM_ALL)
set_property(TARGET miniz PROPERTY POSITION_INDEPENDENT_CODE ON)
# Work around https://github.com/richgel999/miniz/pull/292
get_target_property(miniz_c_launcher miniz C_COMPILER_LAUNCHER)
if(MSVC AND miniz_c_launcher MATCHES "ccache")
    set_property(TARGET miniz PROPERTY C_COMPILER_LAUNCHER)
    set_property(TARGET miniz PROPERTY MSVC_DEBUG_INFORMATION_FORMAT "")
endif()

# LZ4
set(LZ4_BUNDLED_MODE ON)
add_subdirectory(lz4/build/cmake EXCLUDE_FROM_ALL)
if(MSVC)
    target_compile_options(
        lz4_static
        PRIVATE /wd5045 /wd4820 /wd4711 /wd6385 /wd6262
    )
endif()

# Vulkan headers
add_subdirectory(vulkan EXCLUDE_FROM_ALL)

# SPIRV-Headers
add_subdirectory(spirv-headers EXCLUDE_FROM_ALL)

if(SLANG_ENABLE_SLANG_GLSLANG)
    # SPIRV-Tools
    set(SPIRV_TOOLS_BUILD_STATIC ON)
    set(SPIRV_WERROR OFF)
    set(SPIRV_HEADER_DIR "${CMAKE_CURRENT_LIST_DIR}/spirv-headers/")
    set(SPIRV_SKIP_TESTS ON)
    add_subdirectory(spirv-tools EXCLUDE_FROM_ALL)

    # glslang
    set(SKIP_GLSLANG_INSTALL ON)
    set(ENABLE_OPT ON)
    set(ENABLE_PCH OFF)
    add_subdirectory(glslang EXCLUDE_FROM_ALL)
endif()

# imgui
add_library(imgui INTERFACE)
target_include_directories(imgui INTERFACE "${CMAKE_CURRENT_LIST_DIR}/imgui")

# Tidy things up:

# Restore log level if we set it
if(set_cmake_log_level)
    unset(CMAKE_MESSAGE_LOG_LEVEL)
endif()

# for this directory and all subdirectories, prepend
# `external/` to the IDE FOLDER property to every target
function(make_external dir)
    get_property(external_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(external_target ${external_targets})
        get_property(folder TARGET ${external_target} PROPERTY FOLDER)
        set_property(
            TARGET ${external_target}
            PROPERTY FOLDER "external/${folder}"
        )
    endforeach()

    get_property(subdirs DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirs})
        make_external(${subdir})
    endforeach()
endfunction()
make_external(.)
