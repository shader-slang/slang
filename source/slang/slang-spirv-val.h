#pragma once

#include <cstdint>
#include "slang-compiler.h"

namespace Slang
{
SlangResult debugValidateSPIRV(const List<uint8_t>& spirv);
}

