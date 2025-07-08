#pragma once

#include "core/slang-common.h"
#include "slang-ir-insts-enum.h"
namespace Slang
{
const UInt kInvalidStableName = ~0;
UInt getOpcodeStableName(IROp op);
IROp getStableNameOpcode(UInt stableName);
} // namespace Slang
