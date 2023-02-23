#include "slang-ir-propagate-func-properties.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"


namespace Slang
{
bool propagateFuncProperties(IRModule* module)
{
    bool result = false;
    List<IRFunc*> workList;
    HashSet<IRFunc*> workListSet;

    auto addToWorkList = [&](IRFunc* f)
    {
        // Never propagate to functions without a body.
        if (f->getFirstBlock() == nullptr)
            return;
        if (workListSet.Add(f))
            workList.add(f);
    };
    auto addCallersToWorkList = [&](IRFunc* f)
    {
        if (auto g = findOuterGeneric(f))
        {
            for (auto use = g->firstUse; use; use = use->nextUse)
            {
                if (use->getUser()->getOp() == kIROp_Specialize)
                {
                    auto specialize = use->getUser();
                    for (auto iuse = specialize->firstUse; iuse; iuse = iuse->nextUse)
                    {
                        if (auto userFunc = getParentFunc(iuse->getUser()))
                            addToWorkList(userFunc);
                    }
                }
            }
            return;
        }
        for (auto use = f->firstUse; use; use = use->nextUse)
        {
            if (use->getUser()->getOp() == kIROp_Call)
            {
                if (auto userFunc = getParentFunc(use->getUser()))
                    addToWorkList(userFunc);
            }
        }
    };
    for (;;)
    {
        bool changed = false;
        workList.clear();
        workListSet.Clear();

        // Add side effect free functions and their transitive callers to work list.
        for (auto inst : module->getGlobalInsts())
        {
            auto genericInst = as<IRGeneric>(inst);
            if (genericInst)
            {
                inst = findGenericReturnVal(genericInst);
            }
            if (auto func = as<IRFunc>(inst))
            {
                if (func->findDecoration<IRReadNoneDecoration>())
                {
                    addCallersToWorkList(func);
                }
            }
        }

        // Add remaining functions to work list.
        for (auto inst : module->getGlobalInsts())
        {
            auto genericInst = as<IRGeneric>(inst);
            if (genericInst)
            {
                inst = findGenericReturnVal(genericInst);
            }
            if (auto func = as<IRFunc>(inst))
            {
                addToWorkList(func);
            }
        }

        IRBuilder builder(module);

        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto f = workList[i];
            bool hasSideEffectCall = false;
            if (f->findDecoration<IRReadNoneDecoration>())
                continue;
            for (auto block : f->getBlocks())
            {
                for (auto inst : block->getChildren())
                {
                    if (auto call = as<IRCall>(inst))
                    {
                        auto callee = getResolvedInstForDecorations(call->getCallee());
                        switch (callee->getOp())
                        {
                        default:
                            // We are calling an unknown function, so we have to assume
                            // there are side effects in the call.
                            hasSideEffectCall = true;
                            break;
                        case kIROp_Func:
                            if (!callee->findDecoration<IRReadNoneDecoration>())
                            {
                                hasSideEffectCall = true;
                                break;
                            }
                        }
                    }
                    // Are any operands defined in global scope?
                    for (UInt o = 0; o < inst->getOperandCount(); o++)
                    {
                        auto operand = inst->getOperand(o);
                        if (getParentFunc(operand) == f)
                            continue;
                        if (as<IRConstant>(operand))
                            continue;
                        if (as<IRType>(operand))
                            continue;
                        switch (operand->getOp())
                        {
                        case kIROp_Specialize:
                        case kIROp_LookupWitness:
                        case kIROp_StructKey:
                        case kIROp_WitnessTable:
                        case kIROp_WitnessTableEntry:
                        case kIROp_undefined:
                        case kIROp_Func:
                            continue;
                        default:
                            break;
                        }
                        hasSideEffectCall = true;
                        break;
                    }
                }
                if (hasSideEffectCall)
                    break;
            }
            if (!hasSideEffectCall)
            {
                builder.addDecoration(f, kIROp_ReadNoneDecoration);
                addCallersToWorkList(f);
                changed = true;
            }
        }
        result |= changed;
        if (!changed)
            break;
    }
    return result;
}
}
