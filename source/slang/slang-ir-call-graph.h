#include "slang-ir-insts.h"
#include "slang-ir-clone.h"

namespace Slang
{

    void buildEntryPointReferenceGraph(Dictionary<IRInst*, HashSet<IRFunc*>>& referencingEntryPoints, IRModule* module);

    HashSet<IRFunc*>* getReferencingEntryPoints(Dictionary<IRInst*, HashSet<IRFunc*>>& m_referencingEntryPoints, IRInst* inst);

}
