// slang-ir-uniform-usage.h
#pragma once

#include "slang-ir-insts.h"

namespace Slang
{

struct UniformUsageRange
{
    UInt byteOffset;
    UInt byteSize;
};

void collectUniformUsage(
    const IRModule* module,
    Dictionary<IRGlobalParam*, List<UniformUsageRange>>& outUsage);

} // namespace Slang
