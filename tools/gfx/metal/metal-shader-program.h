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
    ShaderProgramImpl(DeviceImpl* device);

    ~ShaderProgramImpl();

    virtual void comFree() override;

    BreakableReference<DeviceImpl> m_device;

    List<String> m_entryPointNames;
    List<ComPtr<ISlangBlob>> m_codeBlobs; //< To keep storage of code in scope
    List<MTL::Library*> m_modules;
    RefPtr<RootShaderObjectLayout> m_rootObjectLayout;

    virtual Result createShaderModule(
        slang::EntryPointReflection* entryPointInfo, ComPtr<ISlangBlob> kernelCode) override;
};


} // namespace metal
} // namespace gfx
