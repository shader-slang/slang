// render-cpu.h
#pragma once

#include "../renderer-shared.h"

namespace gfx
{

SlangResult SLANG_MCALL createCPUDevice(const IDevice::Desc* desc, IDevice** outDevice);

}
