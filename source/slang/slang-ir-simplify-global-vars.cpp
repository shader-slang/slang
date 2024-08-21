// slang-ir-explicit-global-init.cpp

#include "slang-ir-simplify-global-vars.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-ir-util.h"

namespace Slang
{
    void tryToSimplifyUseOfGlobalVar(IRGlobalVar* globalVar, IRUse* use, List<IRInst*>& toDestroy)
    {
        auto user = use->getUser();
        switch (user->getOp())
        {
        case kIROp_Store:
            {
                auto store = as<IRStore>(user);
                if (store->getParent() && store->getParent()->getOp() == kIROp_Module)
                {
                    // Store in Global scope into a GlobalVar is equal to 'insertReturn(global->GetFirstBlock(), Store->GetVal())'
                    if (auto globalVal = as<IRGlobalValueWithCode>(store->getPtr()))
                    {
                        IRBuilder builder(store);
                        IRBlock* block = globalVal->getFirstBlock();
                        if (!block)
                        {
                            builder.setInsertInto(globalVal);
                            block = builder.emitBlock();
                        }
                        builder.setInsertInto(block);
                        builder.emitReturn(store->getVal());
                        toDestroy.add(user);
                    }
                }
                break;
            }
        default:
            break;
        }
    }

    void simplifyGlobalVars(IRModule* module)
    {
        for (auto inst : module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar)
                continue;

            // Find any trivial use which is supposed-to-be part of the
            // init block, hoist these.
            // Do some extra auixillary checks.
            List<IRInst*> toDestroy;
            bool onlyGlobalSideEffects = true;
            traverseUses(globalVar, [&](IRUse* use)
                {
                    tryToSimplifyUseOfGlobalVar(globalVar, use, toDestroy);
                    if (use->getUser()->getParent()->getOp() != kIROp_Module
                        && use->getUser()->mightHaveSideEffects())
                        onlyGlobalSideEffects = false;
                });
            for (auto i : toDestroy)
                i->removeAndDeallocate();

            // Check if our IRGlobalVar is an alias for another variable, if so, simplify
            if (!onlyGlobalSideEffects)
                continue;
            
            auto firstBlock = globalVar->getFirstBlock();
            if (!firstBlock)
                continue;

            // For an alias we must encounter a return op first
            bool fail = false;
            IRReturn* firstReturn = nullptr;

            for (auto localInst : firstBlock->getChildren())
            {
                if (localInst->getOp() != kIROp_Return)
                {
                    fail = true;
                    break;
                }
                else if (localInst->getOp() == kIROp_Return)
                {
                    firstReturn = as<IRReturn>(localInst);
                    break;
                }
            }

            if (fail || !firstReturn)
                continue;

            globalVar->replaceUsesWith(firstReturn->getVal());
            firstBlock->removeAndDeallocateAllDecorationsAndChildren();
            firstBlock->removeAndDeallocate();
        }
    }
}
