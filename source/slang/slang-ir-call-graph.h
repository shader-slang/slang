#pragma once

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"

namespace Slang
{

struct CallGraph
{
public:
    CallGraph() = default;
    explicit CallGraph(IRModule* module);

    void build(IRModule* module);

    /// Retrieves the set of entry points that transitively invoke the given instruction.
    /// Returns nullptr if the instruction has no referencing entry points.
    const HashSet<IRFunc*>* getReferencingEntryPoints(IRInst* inst) const;

    /// Retrieves the set of functions that directly contain the given instruction in their body.
    /// Returns nullptr if the instruction is not referenced by any function.
    const HashSet<IRFunc*>* getReferencingFunctions(IRInst* inst) const;

    /// Retrieves the set of calls that invoke the given function.
    /// Returns nullptr if the function is never called.
    const HashSet<IRCall*>* getReferencingCalls(IRFunc* func) const;

    const Dictionary<IRInst*, HashSet<IRFunc*>>& getReferencingEntryPointsMap() const;

private:
    void registerInstructionReference(IRInst* inst, IRFunc* entryPoint, IRFunc* parentFunc);
    void registerCallReference(IRFunc* func, IRCall* call);

    Dictionary<IRInst*, HashSet<IRFunc*>> m_referencingEntryPoints;
    Dictionary<IRInst*, HashSet<IRFunc*>> m_referencingFunctions;
    Dictionary<IRFunc*, HashSet<IRCall*>> m_referencingCalls;
};

} // namespace Slang
