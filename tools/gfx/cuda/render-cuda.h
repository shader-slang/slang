#pragma once

#include "../renderer-shared.h"

namespace gfx
{

SlangResult SLANG_MCALL createCUDADevice(const IDevice::Desc* desc, IDevice** outDevice);
}
