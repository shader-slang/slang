#pragma once

#include "slang-compiler.h"

#include <cstdint>

namespace Slang
{
SlangResult disassembleSPIRV(const List<uint8_t>& spirv, String& outErr, String& outDis);
}
