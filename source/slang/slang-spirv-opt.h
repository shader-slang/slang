#pragma once

#include <cstdint>
#include "slang-compiler.h"

namespace Slang
{
SlangResult optimizeSPIRV(const List<uint8_t>& spirv, String& outErr, List<uint8_t>& outSpv);
}

