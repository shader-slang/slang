#pragma once

#include <cstdint>
#include "slang-compiler.h"

namespace Slang
{
SlangResult disassembleSPIRV(const List<uint8_t>& spirv, String& outErr, String& outDis);
}

