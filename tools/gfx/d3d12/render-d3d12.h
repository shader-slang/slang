// render-d3d12.h
#pragma once

#include "../renderer-shared.h"

namespace gfx
{

SlangResult SLANG_MCALL createD3D12Device(const IDevice::Desc* desc, IDevice** outDevice);

} // gfx
