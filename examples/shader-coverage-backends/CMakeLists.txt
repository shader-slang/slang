# Selectable-backend coverage demo: one shader dispatched on CPU,
# Vulkan, or Metal, each path implementing the corresponding binding
# recipe from docs/design/shader-coverage-host-interface.md.
#
# Unlike the sibling coverage examples, missing Vulkan does NOT skip
# the whole example — the CPU path (and Metal on Apple platforms)
# stays available; the Vulkan path is compiled in only when a loader
# is found. Like the siblings, this uses raw Vulkan rather than
# slang-rhi until slang-rhi PR #739 lands (see vk_compute_demo.h).

set(_sources main.cpp)
set(_defines "")
set(_libs slang core)

# Vulkan path: only the loader must be found on the machine. Slang's
# bundled Vulkan-Headers target guarantees headers even when
# find_library locates a bare loader; when the SDK's Vulkan::Vulkan
# target is found instead, the SDK's own headers take precedence over
# the bundled ones (the same pattern as the sibling coverage examples).
find_package(Vulkan QUIET)
if(TARGET Vulkan::Vulkan)
    set(_vulkan_loader Vulkan::Vulkan)
else()
    find_library(SLANG_VULKAN_LOADER NAMES vulkan vulkan-1 libvulkan.so.1)
    if(SLANG_VULKAN_LOADER)
        set(_vulkan_loader ${SLANG_VULKAN_LOADER})
    else()
        set(_vulkan_loader "")
        message(
            STATUS
            "examples: shader-coverage-backends builds without the vulkan backend (loader not found)"
        )
    endif()
endif()
if(_vulkan_loader)
    list(APPEND _sources vk_compute_demo.h vk_compute_demo.cpp)
    list(APPEND _defines SLANG_EXAMPLE_HAS_VULKAN=1)
    list(APPEND _libs ${_vulkan_loader} Vulkan::Headers)
endif()

# Metal path: Apple platforms only; uses the vendored metal-cpp
# headers and the Metal framework's runtime MSL compiler, so no
# external Metal toolchain is required.
if(APPLE)
    list(APPEND _defines SLANG_EXAMPLE_HAS_METAL=1)
    list(APPEND _libs metal-cpp "-framework Foundation" "-framework Metal")
endif()

set(_runtime_dir ${CMAKE_CURRENT_BINARY_DIR})
add_custom_command(
    OUTPUT ${_runtime_dir}/hello-coverage.slang
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/hello-coverage.slang
        ${_runtime_dir}/hello-coverage.slang
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/hello-coverage.slang
    COMMENT "Copy hello-coverage.slang to runtime dir"
)
add_custom_target(
    shader-coverage-backends-copy-assets
    DEPENDS ${_runtime_dir}/hello-coverage.slang
)
set_target_properties(
    shader-coverage-backends-copy-assets
    PROPERTIES FOLDER "examples/copy_assets"
)

add_executable(shader-coverage-backends ${_sources})
add_dependencies(shader-coverage-backends shader-coverage-backends-copy-assets)
target_include_directories(
    shader-coverage-backends
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(shader-coverage-backends PRIVATE ${_defines})
target_link_libraries(shader-coverage-backends PRIVATE ${_libs})
target_compile_features(shader-coverage-backends PRIVATE cxx_std_17)
set_target_properties(shader-coverage-backends PROPERTIES FOLDER examples)

# Attach to the all-examples meta target explicitly (raw
# add_executable, same as the sibling coverage demos).
add_dependencies(all-examples shader-coverage-backends)
