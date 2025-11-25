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
    if(NOT MSVC)
        add_compile_options(-w)
    endif()
endif()

# Unordered dense
if(NOT ${SLANG_USE_SYSTEM_UNORDERED_DENSE})
    if(NOT SLANG_OVERRIDE_UNORDERED_DENSE_PATH)
        add_subdirectory(unordered_dense EXCLUDE_FROM_ALL ${system})
    else()
        add_subdirectory(
            ${SLANG_OVERRIDE_UNORDERED_DENSE_PATH}/unordered_dense
            unordered_dense
            EXCLUDE_FROM_ALL
            ${system}
        )
    endif()
endif()

# Miniz
if(NOT ${SLANG_USE_SYSTEM_MINIZ})
    if(NOT SLANG_OVERRIDE_MINIZ_PATH)
        add_subdirectory(miniz EXCLUDE_FROM_ALL ${system})
    else()
        add_subdirectory(
            ${SLANG_OVERRIDE_MINIZ_PATH}/miniz
            miniz
            EXCLUDE_FROM_ALL
            ${system}
        )
    endif()
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
    if(NOT SLANG_OVERRIDE_LZ4_PATH)
        add_subdirectory(lz4/build/cmake EXCLUDE_FROM_ALL ${system})
    else()
        add_subdirectory(
            ${SLANG_OVERRIDE_LZ4_PATH}/lz4/build/cmake
            lz4/build/cmake
            EXCLUDE_FROM_ALL
            ${system}
        )
    endif()
    if(MSVC)
        target_compile_options(
            lz4_static
            PRIVATE /wd5045 /wd4820 /wd4711 /wd6385 /wd6262
        )
    endif()
endif()

# Vulkan headers
if(NOT TARGET Vulkan::Headers)
    if(NOT ${SLANG_USE_SYSTEM_VULKAN_HEADERS})
        if(NOT SLANG_OVERRIDE_VULKAN_HEADERS_PATH)
            add_subdirectory(vulkan EXCLUDE_FROM_ALL ${system})
        else()
            add_subdirectory(
                ${SLANG_OVERRIDE_VULKAN_HEADERS_PATH}/vulkan
                vulkan
                EXCLUDE_FROM_ALL
                ${system}
            )
        endif()
    endif()
endif()

# metal-cpp headers
add_library(metal-cpp INTERFACE)
target_include_directories(
    metal-cpp
    INTERFACE "${CMAKE_CURRENT_LIST_DIR}/metal-cpp"
)

# SPIRV-Headers
# Avoid using ${system} intentionally in `add_subdirectory`, because if we do, it will end up using
# spirv.h installed in the system directory on MacOS.
if(NOT TARGET SPIRV-Headers)
    if(SLANG_USE_SYSTEM_SPIRV_HEADERS)
        if(SLANG_OVERRIDE_SPIRV_HEADERS_PATH)
            message(
                WARNING
                "SLANG_OVERRIDE_SPIRV_HEADERS_PATH does nothing when SLANG_USE_SPIRV_HEADERS is set"
            )
        endif()
    elseif(SLANG_OVERRIDE_SPIRV_HEADERS_PATH)
        add_subdirectory(
            ${SLANG_OVERRIDE_SPIRV_HEADERS_PATH}/spirv-headers
            spirv-headers
            EXCLUDE_FROM_ALL
        )
    else()
        add_subdirectory(spirv-headers EXCLUDE_FROM_ALL)
    endif()
endif()

if(SLANG_ENABLE_SLANG_GLSLANG)
    # When using spirv headers via find_package, SPIRV-Headers_SOURCE_DIR is not set
    # SPIRV-Tools requires that variable, as it uses it to detect if SPIRV-Headers is provided by the consumer.
    # Fake a source build by setting it here. Ideally SPIRV-Tools should not depend on _SOURCE_DIR
    if(${SLANG_USE_SYSTEM_SPIRV_HEADERS})
        get_target_property(
            SPIRV-Headers_SOURCE_DIR
            SPIRV-Headers::SPIRV-Headers
            INTERFACE_INCLUDE_DIRECTORIES
        )
        cmake_path(
            GET
            SPIRV-Headers_SOURCE_DIR
            PARENT_PATH SPIRV-Headers_SOURCE_DIR
        )
    endif()

    if(NOT ${SLANG_USE_SYSTEM_SPIRV_TOOLS})
        # SPIRV-Tools
        set(SPIRV_TOOLS_BUILD_STATIC ON)
        set(SPIRV_WERROR OFF)
        set(SPIRV_SKIP_TESTS ON)

        # Enable mimalloc for SPIRV-Tools to improve compilation performance
        # Based on SPIRV-Tools PR #6267: https://github.com/KhronosGroup/SPIRV-Tools/pull/6267
        if(SLANG_ENABLE_SPIRV_TOOLS_MIMALLOC)
            if(NOT SLANG_OVERRIDE_MIMALLOC_PATH)
                # Place mimalloc in slang/external instead of spirv-tools/external for better organization
                set(MIMALLOC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/mimalloc")
            else()
                set(MIMALLOC_PATH "${SLANG_OVERRIDE_MIMALLOC_PATH}/mimalloc")
            endif()

            # Download mimalloc if it's not available
            if(NOT EXISTS "${MIMALLOC_PATH}/CMakeLists.txt")
                message(
                    STATUS
                    "mimalloc not found at ${MIMALLOC_PATH} - downloading..."
                )
                execute_process(
                    COMMAND
                        ${GIT_EXECUTABLE} clone --depth 1 --branch v2.1.7
                        https://github.com/microsoft/mimalloc.git
                        "${MIMALLOC_PATH}"
                    RESULT_VARIABLE git_result
                    OUTPUT_QUIET
                    ERROR_QUIET
                )
                if(git_result EQUAL 0)
                    # Point SPIRV-Tools to our mimalloc location
                    set(mimalloc_SOURCE_DIR "${MIMALLOC_PATH}")
                    set(SPIRV_TOOLS_USE_MIMALLOC ON)
                    set(SPIRV_TOOLS_USE_MIMALLOC_IN_STATIC_BUILD ON)
                    # set(MI_INSTALL_TOPLEVEL OFF)
                    message(
                        STATUS
                        "Successfully downloaded and enabled mimalloc for SPIRV-Tools at ${MIMALLOC_PATH}"
                    )
                else()
                    message(
                        WARNING
                        "Failed to download mimalloc. Building SPIRV-Tools without mimalloc support."
                    )
                endif()
            endif()

            # Check if mimalloc is available now
            if(EXISTS "${MIMALLOC_PATH}/CMakeLists.txt")
                # Point SPIRV-Tools to our mimalloc location
                set(mimalloc_SOURCE_DIR "${MIMALLOC_PATH}")
                set(SPIRV_TOOLS_USE_MIMALLOC ON)
                # Also enable mimalloc in static builds for better performance
                set(SPIRV_TOOLS_USE_MIMALLOC_IN_STATIC_BUILD ON)
                # Disable mimalloc installation to avoid install target conflicts
                # set(MI_INSTALL_TOPLEVEL OFF)
                message(
                    STATUS
                    "Enabling mimalloc for SPIRV-Tools (including static builds) from ${MIMALLOC_PATH}"
                )
            endif()
        endif()

        # Tools
        if(NOT SLANG_OVERRIDE_SPIRV_TOOLS_PATH)
            add_subdirectory(spirv-tools EXCLUDE_FROM_ALL)
        else()
            add_subdirectory(
                ${SLANG_OVERRIDE_SPIRV_TOOLS_PATH}/spirv-tools
                spirv-tools
                EXCLUDE_FROM_ALL
            )
        endif()
    endif()

    if(NOT ${SLANG_USE_SYSTEM_GLSLANG})
        # glslang
        set(SKIP_GLSLANG_INSTALL ON)
        set(ENABLE_OPT ON)
        set(ENABLE_PCH OFF)
        set(ALLOW_EXTERNAL_SPIRV_TOOLS ${SLANG_USE_SYSTEM_SPIRV_TOOLS})
        if(NOT SLANG_OVERRIDE_GLSLANG_PATH)
            add_subdirectory(glslang EXCLUDE_FROM_ALL ${system})
        else()
            add_subdirectory(
                ${SLANG_OVERRIDE_GLSLANG_PATH}/glslang
                glslang
                EXCLUDE_FROM_ALL
                ${system}
            )
        endif()
    endif()
endif()

if(
    SLANG_ENABLE_GFX
    OR (SLANG_ENABLE_SLANG_RHI AND (SLANG_ENABLE_TESTS OR SLANG_ENABLE_EXAMPLES))
)
    # imgui
    add_library(imgui INTERFACE)
    if(NOT SLANG_OVERRIDE_IMGUI_PATH)
        target_include_directories(
            imgui
            ${system}
            INTERFACE "${CMAKE_CURRENT_LIST_DIR}/imgui"
        )
    else()
        target_include_directories(
            imgui
            ${system}
            INTERFACE "${SLANG_OVERRIDE_IMGUI_PATH}/imgui"
        )
    endif()

    # stb
    add_library(stb INTERFACE)
    target_include_directories(
        stb
        ${system}
        INTERFACE "${CMAKE_CURRENT_LIST_DIR}/stb"
    )
endif()

# slang-rhi
if(SLANG_ENABLE_SLANG_RHI)
    set(SLANG_RHI_BUILD_FROM_SLANG_REPO ON)
    set(SLANG_RHI_BUILD_TESTS ON)
    set(SLANG_RHI_BUILD_TESTS_WITH_GLFW OFF)
    set(SLANG_RHI_INSTALL OFF)
    set(SLANG_RHI_BINARY_DIR ${CMAKE_BINARY_DIR}/$<CONFIG>/bin)
    set(SLANG_RHI_FETCH_SLANG OFF)
    set(SLANG_RHI_SLANG_INCLUDE_DIR ${slang_SOURCE_DIR}/include)
    set(SLANG_RHI_SLANG_BINARY_DIR ${CMAKE_BINARY_DIR})
    set(SLANG_RHI_ENABLE_NVAPI ${SLANG_ENABLE_NVAPI})

    # CoopVec requires 1.717.0-preview, but we cannot use the version until slang-rhi
    # allow slang to override D3D12Version value
    #set(SLANG_RHI_AGILITY_SDK_VERSION D3D12_PREVIEW_SDK_VERSION)
    #set(SLANG_RHI_FETCH_AGILITY_SDK_VERSION
    #    "1.717.0-preview"
    #    CACHE STRING
    #    "Agility SDK version to fetch"
    #    FORCE
    #)

    if(SLANG_ENABLE_DX_ON_VK)
        set(SLANG_RHI_HAS_D3D12 ON)
    endif()
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(SLANG_RHI_ENABLE_WGPU OFF)
    endif()
    if(MINGW)
        set(SLANG_RHI_ENABLE_D3D11 OFF)
        set(SLANG_RHI_ENABLE_D3D12 OFF)
    endif()
    if(NOT SLANG_OVERRIDE_SLANG_RHI_PATH)
        add_subdirectory(slang-rhi)
    else()
        add_subdirectory(
            ${SLANG_OVERRIDE_SLANG_RHI_PATH}/slang-rhi
            slang-rhi
            EXCLUDE_FROM_ALL
            ${system}
        )
    endif()

    # Output slang-rhi-tests to the bin directory so we can run it easily.
    set_target_properties(
        slang-rhi-tests
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/bin
    )
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
