// metal-shader-program.h
#pragma once

#include "metal-base.h"
#include "metal-shader-object-layout.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

class ShaderProgramImpl : public ShaderProgramBase
{
public:
    BreakableReference<DeviceImpl> m_device;

    List<String> m_entryPointNames;
    List<ComPtr<ISlangBlob>> m_codeBlobs; //< To keep storage of code in scope
    List<NS::SharedPtr<MTL::Library>> m_modules;
    RefPtr<RootShaderObjectLayoutImpl> m_rootObjectLayout;

    ShaderProgramImpl(DeviceImpl* device);
    ~ShaderProgramImpl();

    virtual Result createShaderModule(slang::EntryPointReflection* entryPointInfo, ComPtr<ISlangBlob> kernelCode) override;
};


} // namespace metal
} // namespace gfx
