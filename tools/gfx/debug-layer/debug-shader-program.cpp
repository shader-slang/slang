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
        if (!m_baseD3D12Program)
            baseObject->queryInterface(guid, (void**)m_baseD3D12Program.writeRef());
        if (m_baseD3D12Program)
            return static_cast<IShaderProgramD3D12*>(this);
    }

    return nullptr;
}

Result DebugShaderProgram::getRootSignature(void** outRootSignature)
{
    if (!outRootSignature)
        return SLANG_E_INVALID_ARG;

    *outRootSignature = nullptr;
    if (!m_baseD3D12Program)
    {
        SLANG_RETURN_ON_FAIL(baseObject->queryInterface(
            GfxGUID::IID_IShaderProgramD3D12,
            (void**)m_baseD3D12Program.writeRef()));
    }
    return m_baseD3D12Program->getRootSignature(outRootSignature);
}

slang::TypeReflection* DebugShaderProgram::findTypeByName(const char* name)
{
    return baseObject->findTypeByName(name);
}

} // namespace debug
} // namespace gfx
