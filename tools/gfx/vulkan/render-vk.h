// render-vk.h
#pragma once

#include <cstdint>
#include "../renderer-shared.h"

namespace gfx {

SlangResult SLANG_MCALL createVKDevice(const IDevice::Desc* desc, IDevice** outDevice);

} // gfx
