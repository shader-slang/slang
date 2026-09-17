// slang-ir-call-graph.h
#pragma once

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"

namespace Slang
{

/// Maps every reachable IR value to the entry points that reference it.
///
/// The input may still contain specialized generic callees. The implementation follows both the
/// specialization expression and its resolved generic return value so callers can use the graph
/// before or after generic specialization.
void buildEntryPointReferenceGraph(
    Dictionary<IRInst*, HashSet<IRFunc*>>& referencingEntryPoints,
    IRModule* module);

HashSet<IRFunc*>* getReferencingEntryPoints(
    Dictionary<IRInst*, HashSet<IRFunc*>>& m_referencingEntryPoints,
    IRInst* inst);

} // namespace Slang
