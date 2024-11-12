# Don't let all these packages spam up with status messages, filter to only the
# important stuff.
# Don't change things however if the user has explicitly set
# CMAKE_MESSAGE_LOG_LEVEL
set(set_cmake_log_level FALSE)
if(NOT CMAKE_MESSAGE_LOG_LEVEL)
    set(set_cmake_log_level TRUE)
    set(CMAKE_MESSAGE_LOG_LEVEL NOTICE)
endif()

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.25")
    set(system SYSTEM)
else()
    message(
        STATUS
        "CMake 3.25 is required to suppress warnings originating in headers in external/ but you are using ${CMAKE_VERSION}, be prepared for some warnings"
    )
    set(system)
endif()

# Similarly, disable warnings for external projects
if(NOT SLANG_ENABLE_EXTERNAL_COMPILER_WARNINGS)
    if(MSVC)
        add_compile_options(/W0)
    else()
        add_compile_options(-w)
    endif()
endif()

if(NOT ${SLANG_USE_SYSTEM_UNORDERED_DENSE})
    add_subdirectory(unordered_dense EXCLUDE_FROM_ALL ${system})
endif()

# Miniz
if(NOT ${SLANG_USE_SYSTEM_MINIZ})
    add_subdirectory(miniz EXCLUDE_FROM_ALL ${system})
    set_property(TARGET miniz PROPERTY POSITION_INDEPENDENT_CODE ON)
    # Work around https://github.com/richgel999/miniz/pull/292
    get_target_property(miniz_c_launcher miniz C_COMPILER_LAUNCHER)
    if(MSVC AND miniz_c_launcher MATCHES "ccache")
        set_property(TARGET miniz PROPERTY C_COMPILER_LAUNCHER)
        set_property(TARGET miniz PROPERTY MSVC_DEBUG_INFORMATION_FORMAT "")
    endif()
endif()

# LZ4
if(NOT ${SLANG_USE_SYSTEM_LZ4})
    set(LZ4_BUNDLED_MODE ON)
    add_subdirectory(lz4/build/cmake EXCLUDE_FROM_ALL ${system})
    if(MSVC)
        target_compile_options(
            lz4_static
            PRIVATE /wd5045 /wd4820 /wd4711 /wd6385 /wd6262
        )
    endif()
endif()

# Vulkan headers
if(NOT ${SLANG_USE_SYSTEM_VULKAN_HEADERS})
    add_subdirectory(vulkan EXCLUDE_FROM_ALL ${system})
endif()

# metal-cpp headers
add_library(metal-cpp INTERFACE)
target_include_directories(
    metal-cpp
    INTERFACE "${CMAKE_CURRENT_LIST_DIR}/metal-cpp"
)

# SPIRV-Headers
if(NOT ${SLANG_USE_SYSTEM_SPIRV_HEADERS})
    add_subdirectory(spirv-headers EXCLUDE_FROM_ALL ${system})
endif()

if(SLANG_ENABLE_SLANG_GLSLANG)
    # SPIRV-Tools
    set(SPIRV_TOOLS_BUILD_STATIC ON)
    set(SPIRV_WERROR OFF)
    set(SPIRV_HEADER_DIR "${CMAKE_CURRENT_LIST_DIR}/spirv-headers/")
    set(SPIRV_SKIP_TESTS ON)
    add_subdirectory(spirv-tools EXCLUDE_FROM_ALL ${system})

    # glslang
    set(SKIP_GLSLANG_INSTALL ON)
    set(ENABLE_OPT ON)
    set(ENABLE_PCH OFF)
    add_subdirectory(glslang EXCLUDE_FROM_ALL ${system})
endif()

# imgui
add_library(imgui INTERFACE)
target_include_directories(imgui INTERFACE "${CMAKE_CURRENT_LIST_DIR}/imgui")

if(SLANG_ENABLE_SLANG_RHI)
    # slang-rhi
    set(SLANG_RHI_BINARY_DIR ${CMAKE_BINARY_DIR}/$<CONFIG>/bin)
    set(SLANG_RHI_SLANG_INCLUDE_DIR ${slang_SOURCE_DIR}/include)
    set(SLANG_RHI_SLANG_BINARY_DIR ${CMAKE_BINARY_DIR})
    set(SLANG_RHI_BUILD_TESTS OFF)
    set(SLANG_RHI_BUILD_EXAMPLES OFF)

    # Not disabling these break tests (issue #5474)
    set(SLANG_RHI_FETCH_DXC OFF)
    set(SLANG_RHI_FETCH_AGILITY_SDK OFF)

    if(SLANG_ENABLE_DX_ON_VK)
        set(SLANG_RHI_HAS_D3D12 ON)
    endif()
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(SLANG_RHI_ENABLE_WGPU OFF)
    endif()
    add_subdirectory(slang-rhi)
endif()

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
