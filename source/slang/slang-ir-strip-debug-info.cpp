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

    // DebugFunc and DebugLocation decorations also hold references to debug metadata.
    for (auto child : inst->getDecorationsAndChildren())
        findDebugInfo(child, debugInstructions);
}

void stripDebugInfo(IRModule* irModule)
{
    List<IRInst*> debugInstructions;
    findDebugInfo(irModule->getModuleInst(), debugInstructions);
    // Collection is pre-order; remove descendants before their owners so recursive removal
    // does not revisit collected children. This is not dependency order across functions:
    // instruction storage remains in the module arena while removal unlinks operand uses.
    while (debugInstructions.getCount())
    {
        auto inst = debugInstructions.getLast();
        debugInstructions.removeLast();
        inst->removeAndDeallocate();
    }
}
} // namespace Slang
