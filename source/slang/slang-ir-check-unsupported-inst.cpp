#include "slang-ir-check-unsupported-inst.h"

#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

void checkUnsupportedInst(TargetRequest* target, IRFunc* func, DiagnosticSink* sink)
{
    SLANG_UNUSED(target);
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            switch (inst->getOp())
            {
            case kIROp_GetArrayLength:
                sink->diagnose(
                    Diagnostics::AttemptToQuerySizeOfUnsizedArray{.location = inst->sourceLoc});
                break;
            }
        }
    }
}

void checkUnsupportedInst(IRModule* module, TargetRequest* target, DiagnosticSink* sink)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        switch (globalInst->getOp())
        {
        case kIROp_VectorType:
        case kIROp_MatrixType:
            {
                if (!as<IRBasicType>(globalInst->getOperand(0)) &&
                    !as<IRPackedFloatType>(globalInst->getOperand(0)))
                {
                    StringBuilder sb;
                    printDiagnosticArg(sb, globalInst);
                    sink->diagnose(Diagnostics::UnsupportedBuiltinType{
                        .type = sb.produceString(),
                        .location = findFirstUseLoc(globalInst)});
                }
                break;
            }
        case kIROp_Func:
            checkUnsupportedInst(target, as<IRFunc>(globalInst), sink);
            break;
        case kIROp_Generic:
            {
                auto generic = as<IRGeneric>(globalInst);
                auto innerFunc = as<IRFunc>(findGenericReturnVal(generic));
                if (innerFunc)
                    checkUnsupportedInst(target, innerFunc, sink);
                break;
            }
        default:
            break;
        }
    }
}

} // namespace Slang
