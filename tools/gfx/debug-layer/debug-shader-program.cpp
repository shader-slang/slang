// debug-shader-program.cpp
#include "debug-shader-program.h"

namespace gfx
{
using namespace Slang;

namespace debug
{

void* DebugShaderProgram::getInterface(const Slang::Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IShaderProgram)
        return static_cast<IShaderProgram*>(this);

    if (guid == GfxGUID::IID_IShaderProgramD3D12)
    {
        IShaderProgramD3D12* baseD3D12Program = nullptr;
        if (SLANG_SUCCEEDED(baseObject->queryInterface(guid, (void**)&baseD3D12Program)))
        {
            baseObject->release();
            return static_cast<IShaderProgramD3D12*>(this);
        }
    }

    return nullptr;
}

Result DebugShaderProgram::getRootSignature(void** outRootSignature)
{
    IShaderProgramD3D12* baseD3D12Program = nullptr;
    SLANG_RETURN_ON_FAIL(
        baseObject->queryInterface(GfxGUID::IID_IShaderProgramD3D12, (void**)&baseD3D12Program));
    Result result = baseD3D12Program->getRootSignature(outRootSignature);
    baseObject->release();
    return result;
}

slang::TypeReflection* DebugShaderProgram::findTypeByName(const char* name)
{
    return baseObject->findTypeByName(name);
}

} // namespace debug
} // namespace gfx
