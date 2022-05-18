// d3d12-shader-program.cpp
#include "d3d12-shader-program.h"

namespace gfx
{
namespace d3d12
{

using namespace Slang;

Result ShaderProgramImpl::createShaderModule(
    slang::EntryPointReflection* entryPointInfo, ComPtr<ISlangBlob> kernelCode)
{
    ShaderBinary shaderBin;
    shaderBin.stage = entryPointInfo->getStage();
    shaderBin.entryPointInfo = entryPointInfo;
    shaderBin.code.addRange(
        reinterpret_cast<const uint8_t*>(kernelCode->getBufferPointer()),
        (Index)kernelCode->getBufferSize());
    m_shaders.add(_Move(shaderBin));
    return SLANG_OK;
}

} // namespace d3d12
} // namespace gfx
