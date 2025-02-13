// slang-ir-legalize-system-values.h
#pragma once
#include "slang-ir-call-graph.h"
#include "slang-ir-insts.h"

namespace Slang
{

void legalizeImplicitSystemValues(
    IRModule* module,
    const CallGraph& callGraph,
    const List<IRImplicitSystemValue*>& implicitSystemValueInstructions);

} // namespace Slang
