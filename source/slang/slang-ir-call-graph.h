#include "slang-ir-clone.h"
#include "slang-ir-insts.h"

namespace Slang
{

void buildEntryPointReferenceGraph(
    Dictionary<IRInst*, HashSet<IRFunc*>>& referencingEntryPoints,
    IRModule* module,
    Dictionary<IRInst*, HashSet<IRFunc*>>* referencingFunctions = nullptr,
    Dictionary<IRFunc*, HashSet<IRCall*>>* referencingCalls = nullptr);

HashSet<IRFunc*>* getReferencingEntryPoints(
    Dictionary<IRInst*, HashSet<IRFunc*>>& m_referencingEntryPoints,
    IRInst* inst);


/*
class FunctionCallGraph
{
public:
    const HashSet<IRFunc*>* getReferencingFunctions(IRInst* inst) const;
    const HashSet<IRCall*>* getFunctionCalls(IRFunc* func) const;

private:
    Dictionary<IRInst*, HashSet<IRFunc*>> m_referencingFunctions;
    Dictionary<IRFunc*, HashSet<IRCall*>> m_functionCalls;
};
*/


} // namespace Slang
