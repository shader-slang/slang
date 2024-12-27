#include "slang-ir-remove-trivial-local-var.h"

#include "slang-ir-util.h"

namespace Slang
{

// If we see a local var `v` that has only one store: `v = x`, and all other uses are loads,
// and `x` is a read-only global param or a param, then we can replace all uses of `v` with `x`.
// This is a targeted cleanup to remove unnecessary load vars from `constref` parameters introduced
// during lowering.
//
void removeTrivialLocalVar(IRFunc* func)
{
    List<IRInst*> accessChainWorkList;
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getModifiableChildren())
        {
            if (inst->getOp() != kIROp_Var)
                continue;
            IRInst* storeInst = nullptr;
            bool isTrivial = true;
            accessChainWorkList.clear();
            accessChainWorkList.add(inst);
            // Check all uses of var and access chains from var, to see if there
            // are any non-trivial stores.
            for (Index i = 0; i < accessChainWorkList.getCount(); i++)
            {
                auto item = accessChainWorkList[i];
                for (auto use = item->firstUse; use; use = use->nextUse)
                {
                    auto user = use->getUser();
                    if (auto store = as<IRStore>(user))
                    {
                        if (store->getPtrUse() != use)
                            continue;
                        // If there are more than one stores or partial stores,
                        // this var is not trivial.
                        if (storeInst || i != 0)
                        {
                            isTrivial = false;
                            break;
                        }
                        storeInst = store;
                    }
                    else if (auto load = as<IRLoad>(user))
                    {
                    }
                    else if (as<IRGetElementPtr>(user) || as<IRFieldAddress>(user))
                    {
                        accessChainWorkList.add(user);
                    }
                    else
                    {
                        // If we see an unknown use, treat it as a non-trivial store case.
                        isTrivial = false;
                        break;
                    }
                }
                if (!isTrivial)
                    break;
            }
            if (!isTrivial)
                continue;
            auto val = as<IRLoad>(storeInst->getOperand(1));
            if (!val)
                continue;
            auto src = val->getPtr();
            switch (src->getOp())
            {
            case kIROp_Param:
            case kIROp_GlobalParam:
                break;
            default:
                continue;
            }
            bool isSrcReadOnly = true;
            for (auto use = src->firstUse; use; use = use->nextUse)
            {
                if (use->getUser()->getOp() == kIROp_Load)
                    continue;
                if (as<IRDecoration>(use->getUser()))
                    continue;
                isSrcReadOnly = false;
                break;
            }
            if (!isSrcReadOnly)
                continue;
            inst->replaceUsesWith(src);
            inst->removeAndDeallocate();
        }
    }
}

void removeTrivialLocalVar(IRGlobalValueWithCode* code)
{
    if (auto func = as<IRFunc>(code))
        removeTrivialLocalVar(func);
    else if (auto gen = as<IRGeneric>(code))
    {
        auto inner = findGenericReturnVal(gen);
        if (auto code = as<IRGlobalValueWithCode>(inner))
            removeTrivialLocalVar(code);
    }
}

// If we see a local variable that is stored once and never actually written to,
// then we can remove it and replace all uses of it with the source address.
void removeTrivialLocalVar(IRModule* module)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (auto func = as<IRGlobalValueWithCode>(globalInst))
        {
            removeTrivialLocalVar(func);
        }
    }
}
} // namespace Slang
