// metal-helper-functions.h
#pragma once
#include "metal-base.h"

namespace gfx
{

using namespace Slang;

Result SLANG_MCALL getMetalAdapters(List<AdapterInfo>& outAdapters);
Result SLANG_MCALL createMetalDevice(const IDevice::Desc* desc, IDevice** outRenderer);

} // namespace gfx
