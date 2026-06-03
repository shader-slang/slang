# Image-pipeline coverage demo.
#
# Deliberate deviation from the standard `example()` helper used by
# other examples: this demo is raw-Vulkan-based, not slang-rhi-based,
# because slang-rhi `main` cannot bind hidden synthetic resources
# (Slang's `__slang_coverage` buffer) until slang-rhi PR #739 lands.
# When that PR merges and the slang-rhi submodule is bumped, this
# example can be converted to the standard `example()` helper.
#
# Requires Vulkan SDK at build time for the loader (libvulkan).
# Headers come from Slang's Vulkan-Headers submodule.

find_package(Vulkan)
if(NOT Vulkan_FOUND)
    message(
        STATUS
        "examples: skipping shader-coverage-image-pipeline (Vulkan SDK not found)"
    )
    return()
endif()

set(_asset_files
    denoise.slang
    gamma.slang
    pipeline.slang
    tonemap.slang
)

set(_runtime_dir ${CMAKE_CURRENT_BINARY_DIR})
set(_copy_target shader-coverage-image-pipeline-copy-assets)

set(_copied_assets "")
foreach(_asset IN LISTS _asset_files)
    set(_dst "${_runtime_dir}/${_asset}")
    add_custom_command(
        OUTPUT ${_dst}
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${_asset} ${_dst}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_asset}
        COMMENT "Copy ${_asset} to runtime dir"
    )
    list(APPEND _copied_assets ${_dst})
endforeach()

add_custom_target(${_copy_target} DEPENDS ${_copied_assets})
set_target_properties(${_copy_target} PROPERTIES FOLDER "examples/copy_assets")

add_executable(shader-coverage-image-pipeline main.cpp vk_compute_demo.h)
add_dependencies(shader-coverage-image-pipeline ${_copy_target})

target_include_directories(
    shader-coverage-image-pipeline
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    shader-coverage-image-pipeline
    PRIVATE Vulkan::Vulkan slang core
)
target_compile_features(shader-coverage-image-pipeline PRIVATE cxx_std_17)
set_target_properties(
    shader-coverage-image-pipeline
    PROPERTIES FOLDER examples
)
