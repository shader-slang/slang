// vk-shader-program.cpp
#include "vk-shader-program.h"

#include "vk-device.h"
#include "vk-util.h"

#include "external/spirv-tools/include/spirv-tools/linker.hpp"

namespace gfx
{

using namespace Slang;

namespace vk
{

ShaderProgramImpl::ShaderProgramImpl(DeviceImpl* device)
    : m_device(device)
{
    for (auto& shaderModule : m_modules)
        shaderModule = VK_NULL_HANDLE;
}

ShaderProgramImpl::~ShaderProgramImpl()
{
    for (auto shaderModule : m_modules)
    {
        if (shaderModule != VK_NULL_HANDLE)
        {
            m_device->m_api.vkDestroyShaderModule(m_device->m_api.m_device, shaderModule, nullptr);
        }
    }
}

void ShaderProgramImpl::comFree()
{
    m_device.breakStrongReference();
}

VkPipelineShaderStageCreateInfo ShaderProgramImpl::compileEntryPoint(
    const char* entryPointName,
    ISlangBlob* code,
    VkShaderStageFlagBits stage,
    VkShaderModule& outShaderModule)
{
    char const* dataBegin = (char const*)code->getBufferPointer();
    char const* dataEnd = (char const*)code->getBufferPointer() + code->getBufferSize();

    // We need to make a copy of the code, since the Slang compiler
    // will free the memory after a compile request is closed.

    VkShaderModuleCreateInfo moduleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    moduleCreateInfo.pCode = (uint32_t*)code->getBufferPointer();
    moduleCreateInfo.codeSize = code->getBufferSize();

    VkShaderModule module;
    SLANG_VK_CHECK(m_device->m_api.vkCreateShaderModule(
        m_device->m_device,
        &moduleCreateInfo,
        nullptr,
        &module));
    outShaderModule = module;

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    shaderStageCreateInfo.stage = stage;

    shaderStageCreateInfo.module = module;
    shaderStageCreateInfo.pName = entryPointName;

    return shaderStageCreateInfo;
}

static ComPtr<ISlangBlob> linkUsingSPIRVTools(List<ComPtr<ISlangBlob> > kernelCodes)
{
    spvtools::Context context(SPV_ENV_UNIVERSAL_1_5);
    spvtools::LinkerOptions options;
    spvtools::MessageConsumer consumer = [](spv_message_level_t level,
                                        const char* source,
                                        const spv_position_t& position,
                                        const char* message)
    {
        printf("SPIRV-TOOLS: %s\n", message);
        printf("SPIRV-TOOLS: %s\n", source);
        printf("SPIRV-TOOLS: %zu:%zu\n", position.index, position.column);
    };
    context.SetMessageConsumer(consumer);
    std::vector<uint32_t*> binaries;
    std::vector<size_t> binary_sizes;
    for (auto kernelCode : kernelCodes)
    {
        binaries.push_back((uint32_t*)kernelCode->getBufferPointer());
        binary_sizes.push_back(kernelCode->getBufferSize() / sizeof(uint32_t));
    }

    std::vector<uint32_t> linked_binary;

    spvtools::Link(
        context,
        binaries.data(),
        binary_sizes.data(),
        binaries.size(),
        &linked_binary,
        options);

    // Create a blob to hold the linked binary
    ComPtr<ISlangBlob> linkedKernelCode;

    // Replace kernel code with linked binary
    // Creates a new blob with the linked binary
    linkedKernelCode = RawBlob::create(linked_binary.data(), linked_binary.size() * sizeof(uint32_t));

    return linkedKernelCode;
}

Result ShaderProgramImpl::createShaderModule(
    slang::EntryPointReflection* entryPointInfo,
    List<ComPtr<ISlangBlob>>& kernelCodes)
{
    ComPtr<ISlangBlob> linkedKernel = linkUsingSPIRVTools(kernelCodes);
    m_codeBlobs.add(linkedKernel);

    VkShaderModule shaderModule;
    auto realEntryPointName = entryPointInfo->getNameOverride();
    const char* spirvBinaryEntryPointName = "main";
    m_stageCreateInfos.add(compileEntryPoint(
        spirvBinaryEntryPointName,
        linkedKernel,
        (VkShaderStageFlagBits)VulkanUtil::getShaderStage(entryPointInfo->getStage()),
        shaderModule));
    m_entryPointNames.add(realEntryPointName);
    m_modules.add(shaderModule);
    return SLANG_OK;
}

} // namespace vk
} // namespace gfx
