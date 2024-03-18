#include "slang-ir-check-unsupported-inst.h"

#include "slang-ir.h"
#include "slang-ir-util.h"

namespace Slang
{
    bool isCPUTarget(TargetRequest* targetReq);

    void checkRecursion(HashSet<IRFunc*>& checkedFuncs, IRFunc* func, DiagnosticSink* sink)
    {
        HashSet<IRFunc*> visitedFuncs;
        List<IRFunc*> workList;
        if (visitedFuncs.add(func) && checkedFuncs.add(func))
        {
            workList.add(func);
        }
        for (Index i = 0; i < workList.getCount(); i++)
        {
            func = workList[i];
            for (auto use = func->firstUse; use; use = use->nextUse)
            {
                auto callInst = as<IRCall>(use->getUser());
                if (!callInst)
                    continue;
                auto caller = as<IRFunc>(getParentFunc(callInst));
                if (!caller)
                    continue;
                if (visitedFuncs.contains(caller))
                {
                    sink->diagnose(callInst, Diagnostics::unsupportedRecursion, caller);
                    continue;
                }
                else if (checkedFuncs.add(caller))
                {
                    workList.add(caller);
                    visitedFuncs.add(caller);
                }
            }
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
