// cpu-helper-functions.h
#pragma once
#include "cpu-base.h"

namespace gfx
{
using namespace Slang;

Result SLANG_MCALL createCPUDevice(const IDevice::Desc* desc, IDevice** outDevice);

} // namespace gfx
