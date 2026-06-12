// Minimal raw-Vulkan compute helper for the shader-coverage demos.
//
// Single-queue, single-pipeline, storage-buffer-only. No swap chain, no
// graphics, no images. Host-coherent memory for everything to keep the
// demo code linear. MoltenVK-friendly: enables VK_KHR_portability_*
// extensions when present.
//
// ============================================================
// WHEN slang-rhi PR #739 LANDS, THIS WHOLE FILE BECOMES OBSOLETE.
// ============================================================
//
// Today the demo uses raw Vulkan because slang-rhi main has no way to
// bind hidden synthetic resources (the synthesized `__slang_coverage`
// buffer) — its binding API is reflection-driven, and Slang's coverage
// IR pass synthesizes the buffer after reflection. PR #739 adds
// `bindSyntheticResource(IShaderProgram*, IShaderObject*, id, Binding)`
// which closes that gap.
//
// Once #739 merges and the slang-rhi submodule is bumped, the
// migration is:
//   1. Delete `vk_compute_demo.h` and `vk_compute_demo.cpp`.
//   2. In main.cpp, replace `vkdemo::Context` + Buffer/Pipeline calls
//      with slang-rhi's IDevice / IBuffer / IComputePipeline / etc.
//   3. Replace the raw vkUpdateDescriptorSets for the coverage buffer
//      with `bindSyntheticResource(...)`.
//   4. Switch the demo's CMakeLists.txt to use the `example()` helper
//      (which links slang-rhi via the standard slang examples convention).
//   5. Drop the `[[vk::binding]]` annotations on the user-visible
//      resources in the slang sources if you want — slang-rhi binds by
//      name and doesn't need them. (Leaving them in is harmless.)
//
// This file (header + cpp) is duplicated verbatim across the two
// shader-coverage example directories. Keeping per-demo copies
// (rather than a shared helper) reduces coupling and makes each
// demo a self-contained reference.

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkdemo
{

// A linear, host-coherent Vulkan buffer with its memory persistently
// mapped. All buffers in the demo are this shape — sized once, written
// from the host, dispatched against, read back. No staging, no
// transfer queues. Mapped memory pointer is owned by the `Context`
// that created it.
struct Buffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize size = 0;
    void* mapped = nullptr;
};

// A compute pipeline + the descriptor-set layouts it expects + the
// pipeline layout that ties them together. The shader module is kept
// alive for the lifetime of the pipeline; `destroyPipeline` releases
// everything in one shot.
struct ComputePipeline
{
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> setLayouts;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
};

// Demo-scope wrapper around a Vulkan instance / device / queue /
// command pool / descriptor pool. One `Context` per process. All
// member functions throw `std::runtime_error` (via the `check`
// helper) on Vulkan failure; main()s wrap their bodies in try/catch.
class Context
{
public:
    // `requireInt64Atomics = true` filters the physical-device pick to
    // require `shaderBufferInt64Atomics` so the 64-bit coverage path
    // works. Callers running with `-trace-coverage-counter-width 32`
    // (or with coverage off entirely) leave it false.
    void init(bool requireInt64Atomics = false);
    // Restored explicitly: the deleted copy constructor below suppresses the
    // implicit default ctor, but the demo uses `Context ctx; ctx.init();`.
    Context() = default;
    ~Context();
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    Buffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
    void destroyBuffer(Buffer& b);
    void upload(Buffer& b, const void* data, VkDeviceSize size);
    void download(Buffer& b, void* data, VkDeviceSize size);

    ComputePipeline createComputePipeline(
        const void* spirv,
        size_t spirvBytes,
        const std::vector<std::vector<VkDescriptorSetLayoutBinding>>& setBindings,
        const char* entryPointName);
    void destroyPipeline(ComputePipeline& p);

    VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout layout);
    void writeStorageBuffer(VkDescriptorSet set, uint32_t binding, const Buffer& buf);

    // Records and submits a single compute dispatch.
    // `sets` is indexed by descriptor-set number.
    void dispatch(
        const ComputePipeline& pipe,
        const std::vector<VkDescriptorSet>& sets,
        uint32_t x,
        uint32_t y,
        uint32_t z);

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDeviceMemoryProperties memProps = {};
    uint32_t queueFamilyIndex = 0;
    VkQueue queue = VK_NULL_HANDLE;
    VkCommandPool cmdPool = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
};

// Small utility helpers used by both Context::init and main(). Kept
// inline in the header because they're tiny and need to be reachable
// from main.cpp without dragging in the rest of the implementation
// unit.
inline void check(VkResult r, const char* what)
{
    if (r != VK_SUCCESS)
    {
        throw std::runtime_error(
            std::string("Vulkan error in ") + what + ": " + std::to_string(int(r)));
    }
}

inline uint32_t findMemoryType(
    const VkPhysicalDeviceMemoryProperties& props,
    uint32_t typeBits,
    VkMemoryPropertyFlags flags)
{
    for (uint32_t i = 0; i < props.memoryTypeCount; ++i)
    {
        if ((typeBits & (1u << i)) && (props.memoryTypes[i].propertyFlags & flags) == flags)
            return i;
    }
    throw std::runtime_error("no suitable Vulkan memory type");
}

} // namespace vkdemo
