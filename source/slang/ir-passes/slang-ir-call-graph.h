// slang-ir-call-graph.h
#pragma once

#include "slang/ir/slang-ir-clone.h"
#include "slang/ir/slang-ir-insts.h"

namespace Slang
{

void buildEntryPointReferenceGraph(
    Dictionary<IRInst*, HashSet<IRFunc*>>& referencingEntryPoints,
    IRModule* module);

HashSet<IRFunc*>* getReferencingEntryPoints(
    Dictionary<IRInst*, HashSet<IRFunc*>>& m_referencingEntryPoints,
    IRInst* inst);

} // namespace Slang
