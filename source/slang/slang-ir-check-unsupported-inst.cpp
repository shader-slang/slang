#include "slang-ir-check-unsupported-inst.h"

#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-target.h"

namespace Slang
{

void checkUnsupportedInst(TargetRequest* target, IRFunc* func, DiagnosticSink* sink)
{
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            switch (inst->getOp())
            {
            case kIROp_GetArrayLength:
                {
                    // GetArrayLength is supported for SPIRV and GLSL targets
                    bool isSupported = false;
                    if (target)
                    {
                        auto codeGenTarget = target->getTarget();
                        if (codeGenTarget == CodeGenTarget::SPIRV || 
                            codeGenTarget == CodeGenTarget::SPIRVAssembly ||
                            codeGenTarget == CodeGenTarget::GLSL)
                        {
                            isSupported = true;
                        }
                    }
                    
                    if (!isSupported)
                    {
                        sink->diagnose(inst, Diagnostics::attemptToQuerySizeOfUnsizedArray);
                    }
                    break;
                }
            }
        }
    }
}

void checkUnsupportedInst(TargetRequest* target, IRModule* module, DiagnosticSink* sink)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        switch (globalInst->getOp())
        {
        case kIROp_VectorType:
        case kIROp_MatrixType:
            {
                if (!as<IRBasicType>(globalInst->getOperand(0)))
                {
                    sink->diagnose(
                        findFirstUseLoc(globalInst),
                        Diagnostics::unsupportedBuiltinType,
                        globalInst);
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
