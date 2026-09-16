// slang-ir-call-graph.h
#pragma once

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"

namespace Slang
{

void buildEntryPointReferenceGraph(
    Dictionary<IRInst*, HashSet<IRFunc*>>& referencingEntryPoints,
    IRModule* module);

/// Builds the same reference graph using an explicit set of physical entry-point roots.
///
/// Some target ABIs have compiler-generated functions that behave as entry points without carrying
/// `IREntryPointDecoration`. Metal intersection functions are one example. Supplying those roots
/// explicitly lets target-specific lowering reuse the ordinary transitive call-graph analysis
/// without changing how the functions are classified by the rest of the compiler.
void buildEntryPointReferenceGraph(
    Dictionary<IRInst*, HashSet<IRFunc*>>& referencingEntryPoints,
    List<IRFunc*> const& entryPoints);

HashSet<IRFunc*>* getReferencingEntryPoints(
    Dictionary<IRInst*, HashSet<IRFunc*>>& m_referencingEntryPoints,
    IRInst* inst);

} // namespace Slang
