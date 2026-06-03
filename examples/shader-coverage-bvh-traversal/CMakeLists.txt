# BVH-traversal coverage demo.
#
# See the image-pipeline demo's CMakeLists.txt for the rationale on
# why this example uses raw Vulkan instead of slang-rhi.

find_package(Vulkan)
if(NOT Vulkan_FOUND)
    message(
        STATUS
        "examples: skipping shader-coverage-bvh-traversal (Vulkan SDK not found)"
    )
    return()
endif()

set(_asset_files
    bvh_traverse.slang
    bvh_types.slang
    materials.slang
    ray_intersect.slang
)

set(_runtime_dir ${CMAKE_CURRENT_BINARY_DIR})
set(_copy_target shader-coverage-bvh-traversal-copy-assets)

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

add_executable(shader-coverage-bvh-traversal main.cpp vk_compute_demo.h)
add_dependencies(shader-coverage-bvh-traversal ${_copy_target})

target_include_directories(
    shader-coverage-bvh-traversal
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    shader-coverage-bvh-traversal
    PRIVATE Vulkan::Vulkan slang core
)
target_compile_features(shader-coverage-bvh-traversal PRIVATE cxx_std_17)
set_target_properties(
    shader-coverage-bvh-traversal
    PROPERTIES FOLDER examples
)
