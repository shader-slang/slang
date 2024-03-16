// metal-shader-object-layout.cpp
#include "metal-shader-object-layout.h"

namespace gfx
{

using namespace Slang;

namespace metal
{
Result RootShaderObjectLayout::create(
    DeviceImpl* renderer,
    slang::IComponentType* program,
    slang::ProgramLayout* programLayout,
    RootShaderObjectLayout** outLayout)
{
    return SLANG_OK;
}

} // namespace metal
} // namespace gfx
