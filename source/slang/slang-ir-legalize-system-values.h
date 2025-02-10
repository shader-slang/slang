// slang-ir-legalize-system-values.h
#pragma once
#include "slang-ir-insts.h"

namespace Slang
{

void legalizeImplicitSystemValues(
    const Dictionary<IRInst*, HashSet<IRFunc*>>& entryPointReferenceGraph,
    const Dictionary<IRInst*, HashSet<IRFunc*>>& functionReferenceGraph,
    const Dictionary<IRFunc*, HashSet<IRCall*>>& callReferenceGraph,
    const List<IRImplicitSystemValue*>& implicitSystemValueInstructions);

} // namespace Slang
