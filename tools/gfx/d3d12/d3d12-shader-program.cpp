// d3d12-shader-program.cpp
#include "d3d12-shader-program.h"

namespace gfx
{
namespace d3d12
{

using namespace Slang;

void* ShaderProgramImpl::getInterface(const Slang::Guid& guid)
{
    if (guid == GfxGUID::IID_IShaderProgramD3D12)
        return static_cast<IShaderProgramD3D12*>(this);
    return ShaderProgramBase::getInterface(guid);
}

Result ShaderProgramImpl::getRootSignature(void** outRootSignature)
{
    if (!outRootSignature)
        return SLANG_E_INVALID_ARG;

    *outRootSignature = nullptr;
    if (!m_rootObjectLayout || !m_rootObjectLayout->m_rootSignature)
        return SLANG_FAIL;

    auto rootSignature = m_rootObjectLayout->m_rootSignature.get();
    rootSignature->AddRef();
    *outRootSignature = rootSignature;
    return SLANG_OK;
}

Result ShaderProgramImpl::createShaderModule(
    slang::EntryPointReflection* entryPointInfo,
    List<ComPtr<ISlangBlob>>& kernelCodes)
{
    ShaderBinary shaderBin;
    shaderBin.stage = entryPointInfo->getStage();
    shaderBin.entryPointInfo = entryPointInfo;
    shaderBin.code.addRange(
        reinterpret_cast<const uint8_t*>(kernelCodes[0]->getBufferPointer()),
        (Index)kernelCodes[0]->getBufferSize());
    m_shaders.add(_Move(shaderBin));
    return SLANG_OK;
}

} // namespace d3d12
} // namespace gfx
