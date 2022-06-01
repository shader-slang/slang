// vk-shader-program.h
#pragma once

#include "vk-base.h"
#include "vk-shader-object-layout.h"

namespace gfx
{

using namespace Slang;

namespace vk
{

class ShaderProgramImpl : public ShaderProgramBase
{
public:
    ShaderProgramImpl(DeviceImpl* device);

    ~ShaderProgramImpl();

    virtual void comFree() override;

    BreakableReference<DeviceImpl> m_device;

    Array<VkPipelineShaderStageCreateInfo, 8> m_stageCreateInfos;
    Array<String, 8> m_entryPointNames;
    Array<ComPtr<ISlangBlob>, 8> m_codeBlobs; //< To keep storage of code in scope
    Array<VkShaderModule, 8> m_modules;
    RefPtr<RootShaderObjectLayout> m_rootObjectLayout;

    VkPipelineShaderStageCreateInfo compileEntryPoint(
        const char* entryPointName,
        ISlangBlob* code,
        VkShaderStageFlagBits stage,
        VkShaderModule& outShaderModule);

    virtual Result createShaderModule(
        slang::EntryPointReflection* entryPointInfo, ComPtr<ISlangBlob> kernelCode) override;
};


} // namespace vk
} // namespace gfx
