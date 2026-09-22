#include "slang-ir-strip-debug-info.h"

#include "slang-ir-insts.h"

namespace Slang
{
static void findDebugInfo(IRInst* inst, List<IRInst*>& debugInstructions)
{
    switch (inst->getOp())
    {
    case kIROp_DebugValue:
    case kIROp_DebugVar:
    case kIROp_DebugLine:
    case kIROp_DebugLocationDecoration:
    case kIROp_DebugSource:
    case kIROp_DebugInlinedAt:
    case kIROp_DebugScope:
    case kIROp_DebugNoScope:
    case kIROp_DebugLexicalBlock:
    case kIROp_DebugFuncDecoration:
    case kIROp_DebugInlinedVariable:
    case kIROp_DebugFunction:
    case kIROp_DebugBuildIdentifier:
    case kIROp_DebugCompilationUnit:
        debugInstructions.add(inst);
        break;
    default:
        break;
    }

    for (auto child : inst->getDecorationsAndChildren())
        findDebugInfo(child, debugInstructions);
}

void stripDebugInfo(IRModule* irModule)
{
    List<IRInst*> debugInstructions;
    findDebugInfo(irModule->getModuleInst(), debugInstructions);
    while (debugInstructions.getCount())
    {
        auto inst = debugInstructions.getLast();
        debugInstructions.removeLast();
        inst->removeAndDeallocate();
    }
}
} // namespace Slang
