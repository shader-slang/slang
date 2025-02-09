// slang-ir-legalize-system-values.h
#pragma once
#include "slang-ir-insts.h"

namespace Slang
{

void legalizeImplicitSystemValues(
    const Dictionary<IRInst*, HashSet<IRFunc*>>& entryPointReferenceGraph,
    const List<IRImplicitSystemValue*>& implicitSystemValueInstructions);

} // namespace Slang
