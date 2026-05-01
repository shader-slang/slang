// debug-shader-program.h
#pragma once
#include "debug-base.h"

namespace gfx
{
using namespace Slang;

namespace debug
{

class DebugShaderProgram : public DebugObject<IShaderProgram>, public IShaderProgramD3D12
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    void* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Result SLANG_MCALL getRootSignature(void** outRootSignature) override;
    virtual SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL
    findTypeByName(const char* name) override;

public:
    Slang::ComPtr<slang::IComponentType> m_slangProgram;
};

} // namespace debug
} // namespace gfx
