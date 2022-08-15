// debug-shader-program.cpp
#include "debug-shader-program.h"

namespace gfx
{
using namespace Slang;

namespace debug
{

DebugShaderProgram::DebugShaderProgram(const IShaderProgram::Desc& desc)
{
    m_slangProgram = desc.slangGlobalScope;
}

} // namespace debug
} // namespace gfx
