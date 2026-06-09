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
//   1. Delete this file.
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
// This is duplicated verbatim across the two shader-coverage example
// directories. Keeping per-demo copies (rather than a shared helper)
// reduces coupling and makes each demo a self-contained reference.

#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkdemo
{

struct Buffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize size = 0;
    void* mapped = nullptr;
};

struct ComputePipeline
{
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> setLayouts;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
};

class Context
{
public:
    void init(bool requireInt64Atomics = false);
    ~Context();

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

inline void Context::init(bool requireInt64Atomics)
{
    // Instance: enable portability enumeration on macOS for MoltenVK.
    std::vector<const char*> instanceExts;
    uint32_t propertyCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);
    std::vector<VkExtensionProperties> props(propertyCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, props.data());
    bool hasPortabilityEnum = false;
    for (auto& p : props)
    {
        if (std::strcmp(p.extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0)
        {
            hasPortabilityEnum = true;
            instanceExts.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            break;
        }
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "shader-coverage-demo";
    appInfo.applicationVersion = 0;
    appInfo.pEngineName = "shader-coverage-demo";
    appInfo.engineVersion = 0;
    // Vulkan 1.2: the `-trace-coverage-counter-width 64` path emits SPIR-V 1.5
    // (Int64 + Int64Atomics on a StorageBuffer), which a 1.1 instance (max
    // SPIR-V 1.3) rejects. 1.2 also makes shaderBufferInt64Atomics a core
    // feature we can enable below. The 32-bit default path is unaffected.
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &appInfo;
    ci.enabledExtensionCount = (uint32_t)instanceExts.size();
    ci.ppEnabledExtensionNames = instanceExts.data();
    if (hasPortabilityEnum)
        ci.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    check(vkCreateInstance(&ci, nullptr, &instance), "vkCreateInstance");

    // Pick a physical device with a compute-capable queue family. When 64-bit
    // coverage counters are requested, require shaderBufferInt64Atomics — the
    // integrated GPU on many laptops (e.g. Intel UHD) exposes a compute queue
    // but not 64-bit buffer atomics, so a naive "first compute device" pick
    // silently selects a device that cannot run the instrumented shader.
    // Prefer a discrete GPU among the eligible candidates.
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
        throw std::runtime_error("no Vulkan physical devices found");
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    auto findComputeQueue = [](VkPhysicalDevice pd) -> int
    {
        uint32_t qCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &qCount, nullptr);
        std::vector<VkQueueFamilyProperties> queues(qCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &qCount, queues.data());
        for (uint32_t i = 0; i < qCount; ++i)
            if (queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
                return (int)i;
        return -1;
    };
    auto supportsBufferInt64Atomics = [](VkPhysicalDevice pd) -> bool
    {
        VkPhysicalDeviceShaderAtomicInt64Features a = {};
        a.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES;
        VkPhysicalDeviceFeatures2 f = {};
        f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        f.pNext = &a;
        vkGetPhysicalDeviceFeatures2(pd, &f);
        return f.features.shaderInt64 == VK_TRUE && a.shaderBufferInt64Atomics == VK_TRUE;
    };

    // Pick the best eligible device. Ineligible devices (no compute
    // queue, or missing the requested int64 atomics) are filtered out
    // by the `continue` arms; among the survivors we prefer discrete
    // GPUs (score 2) over integrated/other (score 1). `bestScore =
    // -1` only exists so the first eligible device always wins on
    // the strictly-greater comparison.
    bool picked = false;
    int bestScore = -1;
    for (auto pd : devices)
    {
        int qf = findComputeQueue(pd);
        if (qf < 0)
            continue;
        if (requireInt64Atomics && !supportsBufferInt64Atomics(pd))
            continue;
        VkPhysicalDeviceProperties props = {};
        vkGetPhysicalDeviceProperties(pd, &props);
        int score = (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) ? 2 : 1;
        if (score > bestScore)
        {
            bestScore = score;
            physical = pd;
            queueFamilyIndex = (uint32_t)qf;
            picked = true;
        }
    }
    if (!picked)
        throw std::runtime_error(
            requireInt64Atomics ? "no Vulkan device with a compute queue and "
                                  "shaderBufferInt64Atomics (needed for --counter-width=64); "
                                  "try --counter-width=32"
                                : "no Vulkan device with compute queue");

    vkGetPhysicalDeviceMemoryProperties(physical, &memProps);

    // Device: enable portability-subset on macOS if present.
    std::vector<const char*> deviceExts;
    uint32_t devExtCount = 0;
    vkEnumerateDeviceExtensionProperties(physical, nullptr, &devExtCount, nullptr);
    std::vector<VkExtensionProperties> devExts(devExtCount);
    vkEnumerateDeviceExtensionProperties(physical, nullptr, &devExtCount, devExts.data());
    for (auto& e : devExts)
    {
        if (std::strcmp(e.extensionName, "VK_KHR_portability_subset") == 0)
        {
            deviceExts.push_back("VK_KHR_portability_subset");
            break;
        }
    }

    // Enable the features the `-trace-coverage-counter-width 64` path needs:
    // `shaderInt64` (the SPIR-V Int64 capability) and `shaderBufferInt64Atomics`
    // (64-bit atomicAdd on the StorageBuffer-class `__slang_coverage` counters).
    // Both must be turned on at device-creation time or the shader module is
    // rejected and no counters are written. Querying first keeps the 32-bit
    // default path working on drivers that lack 64-bit atomics (e.g. MoltenVK).
    // Query via the standalone VkPhysicalDeviceShaderAtomicInt64Features struct:
    // some drivers (e.g. NVIDIA) surface shaderBufferInt64Atomics here but report
    // 0 for the same bit in the aggregated VkPhysicalDeviceVulkan12Features.
    VkPhysicalDeviceShaderAtomicInt64Features atomic64Supported = {};
    atomic64Supported.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES;
    VkPhysicalDeviceFeatures2 featuresSupported = {};
    featuresSupported.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    featuresSupported.pNext = &atomic64Supported;
    vkGetPhysicalDeviceFeatures2(physical, &featuresSupported);
    const bool haveBufferInt64Atomics = featuresSupported.features.shaderInt64 == VK_TRUE &&
                                        atomic64Supported.shaderBufferInt64Atomics == VK_TRUE;

    // Enable structs kept in scope until vkCreateDevice (chained via pNext).
    VkPhysicalDeviceShaderAtomicInt64Features atomic64Enable = {};
    atomic64Enable.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES;
    atomic64Enable.shaderBufferInt64Atomics = VK_TRUE;
    VkPhysicalDeviceFeatures2 featuresEnable = {};
    featuresEnable.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    featuresEnable.features.shaderInt64 = VK_TRUE;
    featuresEnable.pNext = &atomic64Enable;

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo qci = {};
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = queueFamilyIndex;
    qci.queueCount = 1;
    qci.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo dci = {};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    dci.enabledExtensionCount = (uint32_t)deviceExts.size();
    dci.ppEnabledExtensionNames = deviceExts.data();
    // When using VkPhysicalDeviceFeatures2 via pNext, pEnabledFeatures stays null.
    //
    // The four `(requireInt64Atomics, haveBufferInt64Atomics)` cases:
    //   - (true,  true):  device was selected for int64 atomics; attach the chain so they are
    //   enabled.
    //   - (true,  false): impossible — the selection loop above filtered this device out.
    //   - (false, true):  caller did not request 64-bit counters, but the device happens to support
    //   them.
    //                     Skipping the chain keeps the device-create surface minimal.
    //   - (false, false): caller did not request and device does not support; skip the chain.
    dci.pNext = haveBufferInt64Atomics ? &featuresEnable : nullptr;
    check(vkCreateDevice(physical, &dci, nullptr, &device), "vkCreateDevice");

    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

    VkCommandPoolCreateInfo cpi = {};
    cpi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpi.queueFamilyIndex = queueFamilyIndex;
    cpi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    check(vkCreateCommandPool(device, &cpi, nullptr, &cmdPool), "vkCreateCommandPool");

    // Descriptor pool sized for many dispatches × small per-dispatch buffers.
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize.descriptorCount = 1024;
    VkDescriptorPoolCreateInfo dpi = {};
    dpi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpi.maxSets = 256;
    dpi.poolSizeCount = 1;
    dpi.pPoolSizes = &poolSize;
    check(vkCreateDescriptorPool(device, &dpi, nullptr, &descPool), "vkCreateDescriptorPool");
}

inline Context::~Context()
{
    if (descPool)
        vkDestroyDescriptorPool(device, descPool, nullptr);
    if (cmdPool)
        vkDestroyCommandPool(device, cmdPool, nullptr);
    if (device)
        vkDestroyDevice(device, nullptr);
    if (instance)
        vkDestroyInstance(instance, nullptr);
}

inline Buffer Context::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage)
{
    Buffer b = {};
    b.size = size;

    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = size;
    bci.usage = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    check(vkCreateBuffer(device, &bci, nullptr, &b.buffer), "vkCreateBuffer");

    VkMemoryRequirements req = {};
    vkGetBufferMemoryRequirements(device, b.buffer, &req);

    VkMemoryAllocateInfo mai = {};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = req.size;
    mai.memoryTypeIndex = findMemoryType(
        memProps,
        req.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    check(vkAllocateMemory(device, &mai, nullptr, &b.memory), "vkAllocateMemory");
    check(vkBindBufferMemory(device, b.buffer, b.memory, 0), "vkBindBufferMemory");
    check(vkMapMemory(device, b.memory, 0, VK_WHOLE_SIZE, 0, &b.mapped), "vkMapMemory");
    return b;
}

inline void Context::destroyBuffer(Buffer& b)
{
    if (b.mapped)
    {
        vkUnmapMemory(device, b.memory);
        b.mapped = nullptr;
    }
    if (b.buffer)
    {
        vkDestroyBuffer(device, b.buffer, nullptr);
        b.buffer = VK_NULL_HANDLE;
    }
    if (b.memory)
    {
        vkFreeMemory(device, b.memory, nullptr);
        b.memory = VK_NULL_HANDLE;
    }
}

inline void Context::upload(Buffer& b, const void* data, VkDeviceSize size)
{
    if (size > b.size)
        throw std::runtime_error("upload size exceeds buffer size");
    std::memcpy(b.mapped, data, size_t(size));
}

inline void Context::download(Buffer& b, void* data, VkDeviceSize size)
{
    if (size > b.size)
        throw std::runtime_error("download size exceeds buffer size");
    std::memcpy(data, b.mapped, size_t(size));
}

inline ComputePipeline Context::createComputePipeline(
    const void* spirv,
    size_t spirvBytes,
    const std::vector<std::vector<VkDescriptorSetLayoutBinding>>& setBindings,
    const char* entryPointName)
{
    ComputePipeline p = {};

    VkShaderModuleCreateInfo smci = {};
    smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smci.codeSize = spirvBytes;
    smci.pCode = reinterpret_cast<const uint32_t*>(spirv);
    check(vkCreateShaderModule(device, &smci, nullptr, &p.shaderModule), "vkCreateShaderModule");

    for (auto& bindings : setBindings)
    {
        VkDescriptorSetLayoutCreateInfo dslci = {};
        dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dslci.bindingCount = (uint32_t)bindings.size();
        dslci.pBindings = bindings.data();
        VkDescriptorSetLayout dsl = VK_NULL_HANDLE;
        check(
            vkCreateDescriptorSetLayout(device, &dslci, nullptr, &dsl),
            "vkCreateDescriptorSetLayout");
        p.setLayouts.push_back(dsl);
    }

    VkPipelineLayoutCreateInfo plci = {};
    plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plci.setLayoutCount = (uint32_t)p.setLayouts.size();
    plci.pSetLayouts = p.setLayouts.data();
    check(vkCreatePipelineLayout(device, &plci, nullptr, &p.layout), "vkCreatePipelineLayout");

    VkComputePipelineCreateInfo cpci = {};
    cpci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    cpci.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    cpci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    cpci.stage.module = p.shaderModule;
    cpci.stage.pName = entryPointName;
    cpci.layout = p.layout;
    check(
        vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &cpci, nullptr, &p.pipeline),
        "vkCreateComputePipelines");
    return p;
}

inline void Context::destroyPipeline(ComputePipeline& p)
{
    if (p.pipeline)
        vkDestroyPipeline(device, p.pipeline, nullptr);
    if (p.layout)
        vkDestroyPipelineLayout(device, p.layout, nullptr);
    for (auto dsl : p.setLayouts)
        vkDestroyDescriptorSetLayout(device, dsl, nullptr);
    if (p.shaderModule)
        vkDestroyShaderModule(device, p.shaderModule, nullptr);
    p = {};
}

inline VkDescriptorSet Context::allocateDescriptorSet(VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo ai = {};
    ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    ai.descriptorPool = descPool;
    ai.descriptorSetCount = 1;
    ai.pSetLayouts = &layout;
    VkDescriptorSet set = VK_NULL_HANDLE;
    check(vkAllocateDescriptorSets(device, &ai, &set), "vkAllocateDescriptorSets");
    return set;
}

inline void Context::writeStorageBuffer(VkDescriptorSet set, uint32_t binding, const Buffer& buf)
{
    VkDescriptorBufferInfo dbi = {};
    dbi.buffer = buf.buffer;
    dbi.offset = 0;
    dbi.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet w = {};
    w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w.dstSet = set;
    w.dstBinding = binding;
    w.descriptorCount = 1;
    w.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    w.pBufferInfo = &dbi;

    vkUpdateDescriptorSets(device, 1, &w, 0, nullptr);
}

inline void Context::dispatch(
    const ComputePipeline& pipe,
    const std::vector<VkDescriptorSet>& sets,
    uint32_t x,
    uint32_t y,
    uint32_t z)
{
    VkCommandBufferAllocateInfo cbai = {};
    cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbai.commandPool = cmdPool;
    cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandBufferCount = 1;
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    check(vkAllocateCommandBuffers(device, &cbai, &cmd), "vkAllocateCommandBuffers");

    VkCommandBufferBeginInfo bi = {};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    check(vkBeginCommandBuffer(cmd, &bi), "vkBeginCommandBuffer");

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline);
    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        pipe.layout,
        0,
        (uint32_t)sets.size(),
        sets.data(),
        0,
        nullptr);
    vkCmdDispatch(cmd, x, y, z);

    check(vkEndCommandBuffer(cmd), "vkEndCommandBuffer");

    VkSubmitInfo si = {};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &cmd;
    check(vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE), "vkQueueSubmit");
    check(vkQueueWaitIdle(queue), "vkQueueWaitIdle");

    vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
}

} // namespace vkdemo
