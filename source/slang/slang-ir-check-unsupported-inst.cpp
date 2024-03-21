#include "slang-ir-check-unsupported-inst.h"

#include "slang-ir.h"
#include "slang-ir-util.h"

namespace Slang
{
    bool isCPUTarget(TargetRequest* targetReq);

    bool checkRecursionImpl(HashSet<IRFunc*>& checkedFuncs, HashSet<IRFunc*>& callStack, IRFunc* func, DiagnosticSink* sink)
    {
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                auto callInst = as<IRCall>(inst);
                if (!callInst)
                    continue;
                auto callee = as<IRFunc>(callInst->getCallee());
                if (!callee)
                    continue;
                if (!callStack.add(callee))
                {
                    sink->diagnose(callInst, Diagnostics::unsupportedRecursion, callee);
                    return false;
                }
                if (checkedFuncs.add(callee))
                    checkRecursionImpl(checkedFuncs, callStack, callee, sink);
                callStack.remove(callee);
            }
        }
        return true;
    }

    void checkRecursion(HashSet<IRFunc*>& checkedFuncs, IRFunc* func, DiagnosticSink* sink)
    {
        HashSet<IRFunc*> callStack;
        if (checkedFuncs.add(func))
        {
            callStack.add(func);
            checkRecursionImpl(checkedFuncs, callStack, func, sink);
        }
    }

    void checkUnsupportedInst(TargetRequest* target, IRModule* module, DiagnosticSink* sink)
    {
        HashSet<IRFunc*> checkedFuncsForRecursionDetection;

        for (auto globalInst : module->getGlobalInsts())
        {
            switch (globalInst->getOp())
            {
            case kIROp_VectorType:
            case kIROp_MatrixType:
                {
                    if (!as<IRBasicType>(globalInst->getOperand(0)))
                    {
                        sink->diagnose(findFirstUseLoc(globalInst), Diagnostics::unsupportedBuiltinType, globalInst);
                    }
                    break;
                }
            case kIROp_Func:
                if (!isCPUTarget(target))
                    checkRecursion(checkedFuncsForRecursionDetection, as<IRFunc>(globalInst), sink);
            default:
                break;
            }
        }
    }

}
