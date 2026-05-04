// shader-coverage-inprocess-demo
//
// End-to-end Vulkan compute dispatch via raw Vulkan API (no slang-rhi)
// demonstrating Slang's `-trace-coverage` instrumentation. Compiles a
// multi-module Slang program in-process, queries
// `slang::ICoverageTracingMetadata` for the synthesized
// `__slang_coverage` buffer's `(set, binding)`, declares the slot in
// our own VkDescriptorSetLayout, dispatches the shader, reads counter
// values back, and emits an LCOV report.
//
// This is the canonical Tier-1 customer pattern: Slang C++ API for
// compile + their own Vulkan code for dispatch + bind the synthesized
// buffer through their own pipeline-layout machinery using the
// metadata-reported binding.

#include "core/slang-string-util.h"
#include "examples/example-base/example-base.h"
#include "examples/example-base/test-base.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "vulkan-api.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <string>
#include <vector>

using Slang::ComPtr;

static const ExampleResources resourceBase("shader-coverage-inprocess-demo");

// ----------------------------------------------------------------
// LCOV writer — line coverage only. Aggregates hits by `(file, line)`,
// filters slots without real source attribution, sorts file then
// line. Output format matches `slang-coverage-rt`'s
// `slang_coverage_save_lcov` byte-for-byte for the same inputs.
// ----------------------------------------------------------------
static int writeLcovReport(
    slang::ICoverageTracingMetadata* coverage,
    const uint32_t* hits,
    uint32_t hitsLen,
    const char* outputPath,
    const char* testName)
{
    uint32_t n = coverage->getCounterCount();
    if (hitsLen != n)
    {
        std::fprintf(stderr, "writeLcovReport: hits length %u != counter count %u\n", hitsLen, n);
        return -1;
    }

    std::map<std::string, std::map<uint32_t, uint64_t>> byFile;
    for (uint32_t i = 0; i < n; ++i)
    {
        slang::CoverageEntryInfo entry;
        if (SLANG_FAILED(coverage->getEntryInfo(i, &entry)))
            continue;
        if (!entry.file || !*entry.file || entry.line == 0)
            continue;
        byFile[entry.file][entry.line] += hits[i];
    }

    std::ofstream f(outputPath, std::ios::binary);
    if (!f)
    {
        std::fprintf(stderr, "writeLcovReport: failed to open '%s'\n", outputPath);
        return -1;
    }
    f << "TN:" << testName << "\n";
    for (const auto& filePair : byFile)
    {
        f << "SF:" << filePair.first << "\n";
        for (const auto& lineHits : filePair.second)
            f << "DA:" << lineHits.first << "," << lineHits.second << "\n";
        f << "end_of_record\n";
    }
    return 0;
}

// ----------------------------------------------------------------
// Demo class — orchestrates Slang compile, Vulkan dispatch, counter
// readback, LCOV emit.
// ----------------------------------------------------------------
struct ShaderCoverageInprocessExample : public TestBase
{
    // Number of workgroups to dispatch along X. With shader's
    // `[numthreads(1,1,1)]`, this equals the total invocations.
    static constexpr uint32_t kNumGroups = 64;

    VulkanAPI vkAPI;
    VkQueue queue = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;

    // User's outputBuffer (uint[kNumGroups]) and the synthesized
    // __slang_coverage buffer (uint[counterCount]).
    VkBuffer outputBuffer = VK_NULL_HANDLE;
    VkDeviceMemory outputBufferMemory = VK_NULL_HANDLE;
    VkBuffer coverageBuffer = VK_NULL_HANDLE;
    VkDeviceMemory coverageBufferMemory = VK_NULL_HANDLE;

    // Host-visible staging buffer, sized to the larger of the two
    // device-local buffers, used both for zero-initializing the
    // coverage buffer and for reading back counter values.
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    VkDeviceSize stagingSize = 0;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    // Coverage-specific.
    ComPtr<slang::IMetadata> metadata;
    slang::ICoverageTracingMetadata* coverage = nullptr;
    uint32_t counterCount = 0;
    uint32_t coverageBinding = 0; // binding within set 0
    uint32_t outputBinding = 0;   // user buffer binding within set 0

    int initVulkanInstanceAndDevice();
    int compileShaderAndCreatePipeline();
    int createBuffers();
    int dispatchAndReadCounters();
    int run();

    ~ShaderCoverageInprocessExample();

private:
    int allocateBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memProps,
        VkBuffer& outBuffer,
        VkDeviceMemory& outMemory);
    int oneShotCommand(std::function<void(VkCommandBuffer)> fn);
};

// Convenience: allocate a buffer + memory + bind.
int ShaderCoverageInprocessExample::allocateBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memProps,
    VkBuffer& outBuffer,
    VkDeviceMemory& outMemory)
{
    VkBufferCreateInfo bci = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bci.size = size;
    bci.usage = usage;
    RETURN_ON_FAIL(vkAPI.vkCreateBuffer(vkAPI.device, &bci, nullptr, &outBuffer));

    VkMemoryRequirements reqs = {};
    vkAPI.vkGetBufferMemoryRequirements(vkAPI.device, outBuffer, &reqs);

    int memTypeIdx = vkAPI.findMemoryTypeIndex(reqs.memoryTypeBits, memProps);
    if (memTypeIdx < 0)
        return -1;

    VkMemoryAllocateInfo ai = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    ai.allocationSize = reqs.size;
    ai.memoryTypeIndex = (uint32_t)memTypeIdx;
    RETURN_ON_FAIL(vkAPI.vkAllocateMemory(vkAPI.device, &ai, nullptr, &outMemory));
    RETURN_ON_FAIL(vkAPI.vkBindBufferMemory(vkAPI.device, outBuffer, outMemory, 0));
    return 0;
}

// Run a one-shot command buffer (record via callback, submit, wait,
// free).
int ShaderCoverageInprocessExample::oneShotCommand(std::function<void(VkCommandBuffer)> fn)
{
    VkCommandBuffer cb;
    VkCommandBufferAllocateInfo cbai = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cbai.commandBufferCount = 1;
    cbai.commandPool = commandPool;
    cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    RETURN_ON_FAIL(vkAPI.vkAllocateCommandBuffers(vkAPI.device, &cbai, &cb));

    VkCommandBufferBeginInfo bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkAPI.vkBeginCommandBuffer(cb, &bi);
    fn(cb);
    vkAPI.vkEndCommandBuffer(cb);

    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    si.commandBufferCount = 1;
    si.pCommandBuffers = &cb;
    vkAPI.vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE);
    vkAPI.vkQueueWaitIdle(queue);
    vkAPI.vkFreeCommandBuffers(vkAPI.device, commandPool, 1, &cb);
    return 0;
}

int ShaderCoverageInprocessExample::initVulkanInstanceAndDevice()
{
    if (initializeVulkanDevice(vkAPI) != 0)
    {
        std::printf("Failed to load Vulkan.\n");
        return -1;
    }

    VkCommandPoolCreateInfo pci = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pci.queueFamilyIndex = vkAPI.queueFamilyIndex;
    RETURN_ON_FAIL(vkAPI.vkCreateCommandPool(vkAPI.device, &pci, nullptr, &commandPool));

    vkAPI.vkGetDeviceQueue(vkAPI.device, vkAPI.queueFamilyIndex, 0, &queue);
    return 0;
}

int ShaderCoverageInprocessExample::compileShaderAndCreatePipeline()
{
    // ---- Slang compile to SPIR-V with `-trace-coverage` on ----
    ComPtr<slang::IGlobalSession> globalSession;
    RETURN_ON_FAIL(slang::createGlobalSession(globalSession.writeRef()));

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::CompilerOptionEntry covOption = {};
    covOption.name = slang::CompilerOptionName::TraceCoverage;
    covOption.value.kind = slang::CompilerOptionValueKind::Int;
    covOption.value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.compilerOptionEntries = &covOption;
    sessionDesc.compilerOptionEntryCount = 1;

    ComPtr<slang::ISession> session;
    RETURN_ON_FAIL(globalSession->createSession(sessionDesc, session.writeRef()));

    // Load the entry-point module (which imports physics + math).
    slang::IModule* slangModule = nullptr;
    {
        ComPtr<slang::IBlob> diagBlob;
        Slang::String path = resourceBase.resolveResource("app.slang");
        slangModule = session->loadModule(path.getBuffer(), diagBlob.writeRef());
        diagnoseIfNeeded(diagBlob);
        if (!slangModule)
            return -1;
    }

    ComPtr<slang::IEntryPoint> entryPoint;
    slangModule->findEntryPointByName("main", entryPoint.writeRef());
    if (!entryPoint)
    {
        std::fprintf(stderr, "entry point 'main' not found\n");
        return -1;
    }

    Slang::List<slang::IComponentType*> components;
    components.add(slangModule);
    components.add(entryPoint);

    ComPtr<slang::IComponentType> composed;
    {
        ComPtr<slang::IBlob> diagBlob;
        SlangResult r = session->createCompositeComponentType(
            components.getBuffer(),
            components.getCount(),
            composed.writeRef(),
            diagBlob.writeRef());
        diagnoseIfNeeded(diagBlob);
        RETURN_ON_FAIL(r);
    }

    ComPtr<slang::IComponentType> linked;
    {
        ComPtr<slang::IBlob> diagBlob;
        SlangResult r = composed->link(linked.writeRef(), diagBlob.writeRef());
        diagnoseIfNeeded(diagBlob);
        RETURN_ON_FAIL(r);
    }

    ComPtr<slang::IBlob> spirvCode;
    {
        ComPtr<slang::IBlob> diagBlob;
        SlangResult r = linked->getEntryPointCode(0, 0, spirvCode.writeRef(), diagBlob.writeRef());
        diagnoseIfNeeded(diagBlob);
        RETURN_ON_FAIL(r);
    }

    // ---- Query coverage metadata ----
    {
        ComPtr<slang::IBlob> diagBlob;
        RETURN_ON_FAIL(
            linked->getEntryPointMetadata(0, 0, metadata.writeRef(), diagBlob.writeRef()));
    }
    coverage = (slang::ICoverageTracingMetadata*)metadata->castAs(
        slang::ICoverageTracingMetadata::getTypeGuid());
    if (!coverage)
    {
        std::fprintf(stderr, "no coverage tracing metadata on artifact\n");
        return -1;
    }
    counterCount = coverage->getCounterCount();

    slang::CoverageBufferInfo bufInfo;
    coverage->getBufferInfo(&bufInfo);
    if (bufInfo.binding < 0 || bufInfo.space != 0)
    {
        std::fprintf(
            stderr,
            "coverage buffer has unexpected binding (space=%d, binding=%d); demo expects space 0\n",
            bufInfo.space,
            bufInfo.binding);
        return -1;
    }
    coverageBinding = (uint32_t)bufInfo.binding;
    outputBinding = (coverageBinding == 0) ? 1 : 0;

    std::printf(
        "[in-process] %u counter slots; outputBuffer at binding %u, "
        "__slang_coverage at binding %u (space 0)\n",
        counterCount,
        outputBinding,
        coverageBinding);

    // ---- Vulkan: descriptor set layout, pipeline layout, pipeline ----
    VkDescriptorSetLayoutBinding bindings[2] = {};
    bindings[0].binding = outputBinding;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
    bindings[1].binding = coverageBinding;
    bindings[1].descriptorCount = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].stageFlags = VK_SHADER_STAGE_ALL;

    VkDescriptorSetLayoutCreateInfo dslci = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    dslci.bindingCount = 2;
    dslci.pBindings = bindings;
    RETURN_ON_FAIL(
        vkAPI.vkCreateDescriptorSetLayout(vkAPI.device, &dslci, nullptr, &descriptorSetLayout));

    VkPipelineLayoutCreateInfo plci = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &descriptorSetLayout;
    RETURN_ON_FAIL(vkAPI.vkCreatePipelineLayout(vkAPI.device, &plci, nullptr, &pipelineLayout));

    VkShaderModuleCreateInfo smci = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    smci.codeSize = spirvCode->getBufferSize();
    smci.pCode = (const uint32_t*)spirvCode->getBufferPointer();
    VkShaderModule shaderModule;
    RETURN_ON_FAIL(vkAPI.vkCreateShaderModule(vkAPI.device, &smci, nullptr, &shaderModule));

    VkComputePipelineCreateInfo cpci = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    cpci.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    cpci.stage.module = shaderModule;
    cpci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    cpci.stage.pName = "main";
    cpci.layout = pipelineLayout;
    RETURN_ON_FAIL(
        vkAPI.vkCreateComputePipelines(vkAPI.device, VK_NULL_HANDLE, 1, &cpci, nullptr, &pipeline));
    vkAPI.vkDestroyShaderModule(vkAPI.device, shaderModule, nullptr);

    return 0;
}

int ShaderCoverageInprocessExample::createBuffers()
{
    const VkDeviceSize outputSize = sizeof(uint32_t) * kNumGroups;
    const VkDeviceSize coverageSize = sizeof(uint32_t) * counterCount;

    // Device-local outputBuffer (user's RWStructuredBuffer<uint>).
    RETURN_ON_FAIL(allocateBuffer(
        outputSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        outputBuffer,
        outputBufferMemory));

    // Device-local coverage counter buffer.
    RETURN_ON_FAIL(allocateBuffer(
        coverageSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        coverageBuffer,
        coverageBufferMemory));

    // Host-visible staging buffer, sized to the larger of the two.
    stagingSize = std::max(outputSize, coverageSize);
    RETURN_ON_FAIL(allocateBuffer(
        stagingSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingMemory));

    // Zero-initialize coverage counter buffer via staging copy. (The
    // shader uses atomicAdd from zero.)
    {
        void* mapped = nullptr;
        vkAPI.vkMapMemory(vkAPI.device, stagingMemory, 0, coverageSize, 0, &mapped);
        std::memset(mapped, 0, (size_t)coverageSize);
        vkAPI.vkUnmapMemory(vkAPI.device, stagingMemory);

        RETURN_ON_FAIL(oneShotCommand(
            [&](VkCommandBuffer cb)
            {
                VkBufferCopy copy = {};
                copy.size = coverageSize;
                vkAPI.vkCmdCopyBuffer(cb, stagingBuffer, coverageBuffer, 1, &copy);
            }));
    }

    return 0;
}

int ShaderCoverageInprocessExample::dispatchAndReadCounters()
{
    // Descriptor pool + set with both buffers wired.
    VkDescriptorPoolCreateInfo dpci = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    VkDescriptorPoolSize poolSizes[] = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}};
    dpci.maxSets = 1;
    dpci.poolSizeCount = 1;
    dpci.pPoolSizes = poolSizes;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    RETURN_ON_FAIL(vkAPI.vkCreateDescriptorPool(vkAPI.device, &dpci, nullptr, &descriptorPool));

    VkDescriptorSetAllocateInfo dsai = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    dsai.descriptorPool = descriptorPool;
    dsai.descriptorSetCount = 1;
    dsai.pSetLayouts = &descriptorSetLayout;
    VkDescriptorSet descriptorSet;
    RETURN_ON_FAIL(vkAPI.vkAllocateDescriptorSets(vkAPI.device, &dsai, &descriptorSet));

    VkDescriptorBufferInfo bufInfos[2] = {};
    bufInfos[0].buffer = outputBuffer;
    bufInfos[0].range = sizeof(uint32_t) * kNumGroups;
    bufInfos[1].buffer = coverageBuffer;
    bufInfos[1].range = sizeof(uint32_t) * counterCount;

    VkWriteDescriptorSet writes[2] = {};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = descriptorSet;
    writes[0].dstBinding = outputBinding;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writes[0].pBufferInfo = &bufInfos[0];
    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = descriptorSet;
    writes[1].dstBinding = coverageBinding;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writes[1].pBufferInfo = &bufInfos[1];
    vkAPI.vkUpdateDescriptorSets(vkAPI.device, 2, writes, 0, nullptr);

    // Record + submit dispatch.
    RETURN_ON_FAIL(oneShotCommand(
        [&](VkCommandBuffer cb)
        {
            vkAPI.vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
            vkAPI.vkCmdBindDescriptorSets(
                cb,
                VK_PIPELINE_BIND_POINT_COMPUTE,
                pipelineLayout,
                0,
                1,
                &descriptorSet,
                0,
                nullptr);
            vkAPI.vkCmdDispatch(cb, kNumGroups, 1, 1);
        }));

    // Copy coverage buffer back to staging.
    const VkDeviceSize coverageSize = sizeof(uint32_t) * counterCount;
    RETURN_ON_FAIL(oneShotCommand(
        [&](VkCommandBuffer cb)
        {
            VkBufferCopy copy = {};
            copy.size = coverageSize;
            vkAPI.vkCmdCopyBuffer(cb, coverageBuffer, stagingBuffer, 1, &copy);
        }));

    // Read counters from staging.
    std::vector<uint32_t> hits(counterCount, 0);
    {
        void* mapped = nullptr;
        vkAPI.vkMapMemory(vkAPI.device, stagingMemory, 0, coverageSize, 0, &mapped);
        std::memcpy(hits.data(), mapped, (size_t)coverageSize);
        vkAPI.vkUnmapMemory(vkAPI.device, stagingMemory);
    }

    uint64_t totalHits = 0;
    for (uint32_t v : hits)
        totalHits += v;
    std::printf(
        "[in-process] dispatched %u workgroups; counter buffer total hits = %llu\n",
        (unsigned)kNumGroups,
        (unsigned long long)totalHits);

    if (totalHits == 0)
    {
        std::fprintf(stderr, "WARNING: zero hits — atomic writes did not reach the buffer\n");
        return -1;
    }

    // Emit LCOV.
    if (writeLcovReport(coverage, hits.data(), counterCount, "coverage.lcov", "in_process") != 0)
        return -1;
    std::printf("[in-process] wrote coverage.lcov\n");
    std::printf("  next: python tools/coverage-html/slang-coverage-html.py "
                "coverage.lcov --output-dir coverage-html/\n");

    vkAPI.vkDestroyDescriptorPool(vkAPI.device, descriptorPool, nullptr);
    return 0;
}

int ShaderCoverageInprocessExample::run()
{
    if (int r = initVulkanInstanceAndDevice())
        return (vkAPI.device == VK_NULL_HANDLE) ? 0 : r;
    RETURN_ON_FAIL(compileShaderAndCreatePipeline());
    RETURN_ON_FAIL(createBuffers());
    RETURN_ON_FAIL(dispatchAndReadCounters());
    return 0;
}

ShaderCoverageInprocessExample::~ShaderCoverageInprocessExample()
{
    if (vkAPI.device == VK_NULL_HANDLE)
        return;

    if (pipeline)
        vkAPI.vkDestroyPipeline(vkAPI.device, pipeline, nullptr);
    if (pipelineLayout)
        vkAPI.vkDestroyPipelineLayout(vkAPI.device, pipelineLayout, nullptr);
    if (descriptorSetLayout)
        vkAPI.vkDestroyDescriptorSetLayout(vkAPI.device, descriptorSetLayout, nullptr);

    if (outputBuffer)
        vkAPI.vkDestroyBuffer(vkAPI.device, outputBuffer, nullptr);
    if (outputBufferMemory)
        vkAPI.vkFreeMemory(vkAPI.device, outputBufferMemory, nullptr);
    if (coverageBuffer)
        vkAPI.vkDestroyBuffer(vkAPI.device, coverageBuffer, nullptr);
    if (coverageBufferMemory)
        vkAPI.vkFreeMemory(vkAPI.device, coverageBufferMemory, nullptr);
    if (stagingBuffer)
        vkAPI.vkDestroyBuffer(vkAPI.device, stagingBuffer, nullptr);
    if (stagingMemory)
        vkAPI.vkFreeMemory(vkAPI.device, stagingMemory, nullptr);

    if (commandPool)
        vkAPI.vkDestroyCommandPool(vkAPI.device, commandPool, nullptr);
}

int exampleMain(int argc, char** argv)
{
    ShaderCoverageInprocessExample example;
    example.parseOption(argc, argv);
    return example.run();
}
