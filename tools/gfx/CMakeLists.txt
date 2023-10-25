set(SLANG_GFX_SOURCE
    cpu/cpu-buffer.cpp
    cpu/cpu-device.cpp
    cpu/cpu-helper-functions.cpp
    cpu/cpu-pipeline-state.cpp
    cpu/cpu-query.cpp
    cpu/cpu-resource-views.cpp
    cpu/cpu-shader-object-layout.cpp
    cpu/cpu-shader-object.cpp
    cpu/cpu-texture.cpp
    cuda/cuda-buffer.cpp
    cuda/cuda-command-buffer.cpp
    cuda/cuda-command-encoder.cpp
    cuda/cuda-command-queue.cpp
    cuda/cuda-device.cpp
    cuda/cuda-helper-functions.cpp
    cuda/cuda-pipeline-state.cpp
    cuda/cuda-query.cpp
    cuda/cuda-shader-object-layout.cpp
    cuda/cuda-shader-object.cpp
    cuda/cuda-shader-program.cpp
    cuda/cuda-texture.cpp
    d3d/d3d-swapchain.cpp
    d3d/d3d-util.cpp
    d3d11/d3d11-buffer.cpp
    d3d11/d3d11-device.cpp
    d3d11/d3d11-helper-functions.cpp
    d3d11/d3d11-pipeline-state.cpp
    d3d11/d3d11-query.cpp
    d3d11/d3d11-scopeNVAPI.cpp
    d3d11/d3d11-shader-object-layout.cpp
    d3d11/d3d11-shader-object.cpp
    d3d11/d3d11-shader-program.cpp
    d3d11/d3d11-swap-chain.cpp
    d3d12/d3d12-buffer.cpp
    d3d12/d3d12-command-buffer.cpp
    d3d12/d3d12-command-encoder.cpp
    d3d12/d3d12-command-queue.cpp
    d3d12/d3d12-descriptor-heap.cpp
    d3d12/d3d12-device.cpp
    d3d12/d3d12-fence.cpp
    d3d12/d3d12-helper-functions.cpp
    d3d12/d3d12-pipeline-state.cpp
    d3d12/d3d12-posix-synchapi.cpp
    d3d12/d3d12-query.cpp
    d3d12/d3d12-render-pass.cpp
    d3d12/d3d12-resource-views.cpp
    d3d12/d3d12-resource.cpp
    d3d12/d3d12-sampler.cpp
    d3d12/d3d12-shader-object-layout.cpp
    d3d12/d3d12-shader-object.cpp
    d3d12/d3d12-shader-program.cpp
    d3d12/d3d12-shader-table.cpp
    d3d12/d3d12-submitter.cpp
    d3d12/d3d12-swap-chain.cpp
    d3d12/d3d12-texture.cpp
    d3d12/d3d12-transient-heap.cpp
    debug-layer/debug-buffer.cpp
    debug-layer/debug-command-buffer.cpp
    debug-layer/debug-command-encoder.cpp
    debug-layer/debug-command-queue.cpp
    debug-layer/debug-device.cpp
    debug-layer/debug-fence.cpp
    debug-layer/debug-helper-functions.cpp
    debug-layer/debug-pipeline-state.cpp
    debug-layer/debug-query.cpp
    debug-layer/debug-resource-views.cpp
    debug-layer/debug-sampler-state.cpp
    debug-layer/debug-shader-object.cpp
    debug-layer/debug-shader-program.cpp
    debug-layer/debug-swap-chain.cpp
    debug-layer/debug-texture.cpp
    debug-layer/debug-transient-heap.cpp
    flag-combiner.cpp
    immediate-renderer-base.cpp
    nvapi/nvapi-util.cpp
    open-gl/render-gl.cpp
    render.cpp
    renderer-shared.cpp
    resource-desc-utils.cpp
    simple-render-pass-layout.cpp
    vulkan/vk-api.cpp
    vulkan/vk-buffer.cpp
    vulkan/vk-command-buffer.cpp
    vulkan/vk-command-encoder.cpp
    vulkan/vk-command-queue.cpp
    vulkan/vk-descriptor-allocator.cpp
    vulkan/vk-device-queue.cpp
    vulkan/vk-device.cpp
    vulkan/vk-fence.cpp
    vulkan/vk-framebuffer.cpp
    vulkan/vk-helper-functions.cpp
    vulkan/vk-module.cpp
    vulkan/vk-pipeline-state.cpp
    vulkan/vk-query.cpp
    vulkan/vk-render-pass.cpp
    vulkan/vk-resource-views.cpp
    vulkan/vk-sampler.cpp
    vulkan/vk-shader-object-layout.cpp
    vulkan/vk-shader-object.cpp
    vulkan/vk-shader-program.cpp
    vulkan/vk-shader-table.cpp
    vulkan/vk-swap-chain.cpp
    vulkan/vk-texture.cpp
    vulkan/vk-transient-heap.cpp
    vulkan/vk-util.cpp)

add_library(gfx ${SLANG_GFX_SOURCE})
set_default_compile_options(gfx)
target_link_libraries(gfx PRIVATE core slang Vulkan-Headers)

# slang-gfx.h is in the project root, so include that directory in the interface
# for gfx
target_include_directories(
  gfx PUBLIC $<BUILD_INTERFACE:${SLANG_ROOT_DIR}>
             $<INSTALL_INTERFACE:include> # <prefix>/include/
)

get_target_property(SLANG_GFX_TARGET_TYPE gfx TYPE)
if(SLANG_GFX_TARGET_TYPE STREQUAL SHARED_LIBRARY)
  target_compile_definitions(gfx PRIVATE SLANG_GFX_DYNAMIC
                                         SLANG_GFX_DYNAMIC_EXPORT)
endif()

if(SLANG_ENABLE_AFTERMATH)
  target_compile_definitions(gfx PRIVATE GFX_NV_AFTERMATH)
  target_link_libraries(gfx PRIVATE aftermath)
endif()

if(SLANG_ENABLE_XLIB)
  find_package(X11 REQUIRED)
  target_link_libraries(gfx PRIVATE X11::X11)
endif()

if(SLANG_ENABLE_NVAPI)
  target_compile_definitions(gfx PRIVATE GFX_NVAPI)
endif()

if(SLANG_ENABLE_CUDA)
  target_compile_definitions(gfx PRIVATE GFX_ENABLE_CUDA RENDER_TEST_CUDA)
endif()

if(SLANG_ENABLE_OPTIX)
  target_compile_definitions(gfx PRIVATE GFX_OPTIX)
endif()
