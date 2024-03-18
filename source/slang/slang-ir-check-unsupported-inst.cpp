#include "slang-ir-check-unsupported-inst.h"

#include "slang-ir.h"
#include "slang-ir-util.h"

namespace Slang
{
    bool isCPUTarget(TargetRequest* targetReq);

    void checkRecursionImpl(HashSet<IRFunc*>& checkedFuncs, HashSet<IRFunc*>& callStack, IRFunc* func, DiagnosticSink* sink)
    {
        for (auto use = func->firstUse; use; use = use->nextUse)
        {
            auto callInst = as<IRCall>(use->getUser());
            if (!callInst)
                continue;
            auto caller = as<IRFunc>(getParentFunc(callInst));
            if (!caller)
                continue;
            if (checkedFuncs.add(caller))
            {
                if (!callStack.add(caller))
                {
                    sink->diagnose(callInst, Diagnostics::unsupportedRecursion, caller);
                    return;
                }
                checkRecursionImpl(checkedFuncs, callStack, caller, sink);
                callStack.remove(caller);
            }
        }
    }

    void checkRecursion(HashSet<IRFunc*>& checkedFuncs, IRFunc* func, DiagnosticSink* sink)
    {
        HashSet<IRFunc*> callStack;
        if (checkedFuncs.add(func))
        {
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
