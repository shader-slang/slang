// render-d3d11.h
#pragma once

#include "../renderer-shared.h"

namespace gfx
{

SlangResult SLANG_MCALL createD3D11Device(const IDevice::Desc* desc, IDevice** outDevice);

} // gfx
