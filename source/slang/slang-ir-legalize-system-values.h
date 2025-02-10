// slang-ir-legalize-system-values.h
#pragma once
#include "slang-ir-insts.h"

namespace Slang
{

void legalizeImplicitSystemValues(
    IRModule* module,
    const Dictionary<IRInst*, HashSet<IRFunc*>>& functionReferenceGraph,
    const Dictionary<IRFunc*, HashSet<IRCall*>>& callReferenceGraph,
    const List<IRImplicitSystemValue*>& implicitSystemValueInstructions);

} // namespace Slang
