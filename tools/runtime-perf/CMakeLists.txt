cmake_minimum_required(VERSION 3.25)
project(slang-runtime-perf LANGUAGES C CXX)

# A standalone build deliberately uses a caller-selected compiler package.
# It does not rebuild Slang or require a particular CI provider.
set(SLANG_ROOT
    ""
    CACHE PATH
    "Slang package/build configuration containing bin and lib"
)
set(SLANG_INCLUDE_DIR
    "${SLANG_ROOT}/include"
    CACHE PATH
    "Matching Slang public headers"
)
set(SLANG_RHI_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../../external/slang-rhi"
    CACHE PATH
    "Pinned slang-rhi source"
)
if(NOT EXISTS "${SLANG_INCLUDE_DIR}/slang.h")
    message(
        FATAL_ERROR
        "Set SLANG_ROOT and SLANG_INCLUDE_DIR to a Slang compiler package and its headers"
    )
endif()
set(SLANG_RHI_FETCH_SLANG OFF CACHE BOOL "" FORCE)
set(SLANG_RHI_SLANG_INCLUDE_DIR "${SLANG_INCLUDE_DIR}" CACHE STRING "" FORCE)
set(SLANG_RHI_SLANG_BINARY_DIR "${SLANG_ROOT}" CACHE STRING "" FORCE)
set(SLANG_RHI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SLANG_RHI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SLANG_RHI_BUILD_TESTS_WITH_GLFW OFF CACHE BOOL "" FORCE)
set(SLANG_RHI_ENABLE_WGPU OFF CACHE BOOL "")
set(SLANG_RHI_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("${SLANG_RHI_SOURCE_DIR}" rhi)
add_executable(slang-runtime-perf native/runner.cpp)
target_compile_features(slang-runtime-perf PRIVATE cxx_std_17)
target_link_libraries(slang-runtime-perf PRIVATE slang-rhi)
set_property(
    DIRECTORY
    APPEND
    PROPERTY
        CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/native/runner.cpp"
)
file(SHA256 "${CMAKE_CURRENT_LIST_DIR}/native/runner.cpp" RUNNER_SOURCE_SHA)
execute_process(
    COMMAND git -C "${SLANG_RHI_SOURCE_DIR}" rev-parse HEAD
    OUTPUT_VARIABLE RHI_REV
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_compile_definitions(
    slang-runtime-perf
    PRIVATE
        RUNTIME_PERF_RHI_REV="${RHI_REV}"
        RUNTIME_PERF_SOURCE_SHA="${RUNNER_SOURCE_SHA}"
)
if(WIN32)
    add_custom_command(
        TARGET slang-runtime-perf
        POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:slang>
            $<TARGET_FILE_DIR:slang-runtime-perf>
    )
endif()
