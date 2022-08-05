// debug-shader-program.h
#pragma once
#include "debug-base.h"

namespace gfx
{
using namespace Slang;

namespace debug
{

class DebugShaderProgram : public DebugObject<IShaderProgram>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    IShaderProgram* getInterface(const Slang::Guid& guid);

    DebugShaderProgram(const IShaderProgram::Desc& desc);

public:
    Slang::ComPtr<slang::IComponentType> m_slangProgram;
};

} // namespace debug
} // namespace gfx
