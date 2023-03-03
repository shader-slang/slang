#include "slang-ir-autodiff-primal-hoist.h"

namespace Slang 
{

void AutodiffCheckpointPolicyBase::findInstsWithDiffUses()
{

    auto doesInstHaveDiffUses = [&](IRInst* inst)
    {
        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            IRBlock* userBlock = getBlock(use->getUser());

            if (inst->findDecoration<IRBackwardDerivativePrimalContextDecoration>())
                return true;

            if (userBlock->findDecoration<IRDifferentialInstDecoration>())
                return true;

            if (this->instsWithDiffUses.Contains(use->getUser()))
                return true;
            
            return false;
        }
    };

    List<IRBlock*> workList;
    for (auto block : func->getBlocks())
    {
        if (block->findDecoration<IRDifferentialInstDecoration>())
            continue;

        for (auto child : block->getChildren())
        {
            if (child->findDecoration<IRPrimalInstDecoration>() && 
                doesInstHaveDiffUses(child))
                this->instsWithDiffUses.Add(child);
        }
    }
}

bool containsOperand(IRInst* inst, IRInst* operand)
{
    for (UIndex ii = 0; ii < inst->getOperandCount(); ii++)
        if (inst->getOperand(ii) == inst)
            return true;
    
    return false;
}

// TODO: STOPPED HERE
// Current plan: 
// Call moveToDiffBlock() by re-traversing the blocks, checking if insts
// are in the recomputeSet
// _after_ calling apply() on _all_ *uses*. Use BlockSplitInfo
// to simply move the insts to top-of-differential-block
// 
// Then we traverse through the primal blocks _again_ looking for 
// insts in the invertSet, find the inverted-use-site (the specific use of 
// the specific operand for which we're inverting this inst), clone that
// inst and place it into the differential block _after_ the inverted-use-site
// For now, we'll assert out if there are multiple inverted-use-sites
//
// Then call ensurePrimalInstAvailability(inst) on all the insts in 
// storeSet. This will use a dominator tree to check if the inst can
// be accessed. If not, create a var based on the level of indexing.
// Any Load/GetElementPtr inst lowered into the diff blocks will be
// tagged as 'PrimalRecompute'
//  
// During transposition: insts marked 'PrimalInvert' will be inverted as we go,
// hoistPrimalOperands() will be called on each differential inst which will
// recursively pull any 'PrimalRecompute' operands into the reverse-mode blocks as needed,
// and load from inverse-buffer for 'PrimalInvert' insts.
// 

void AutodiffCheckpointPolicyBase::processFunc(IRGlobalValueWithCode* func, BlockSplitInfo* splitInfo)
{
    // Populate recompute/store/invert sets with insts, by applying the policy
    // to them.
    // 
    for (auto block : func->getBlocks())
    {
        // Skip parameter block.
        if (block == func->getFirstBlock())
            continue;

        if (!block->findDecoration<IRDifferentialInstDecoration>())
            continue;

        List<IRUse*> workList;
        HashSet<IRUse*> processedUses;

        auto addPrimalOperandsToWorkList = [&](IRInst* inst)
        {
            UIndex opIndex = 0;
            for (auto operand = inst->getOperands(); opIndex < inst->getOperandCount(); operand++, opIndex++)
            {   
                if (!operand->get()->findDecoration<IRDifferentialInstDecoration>())
                    workList.add(operand);
            }
        };

        for (auto child : block->getChildren())
        {
            if (!child->findDecoration<IRDifferentialInstDecoration>())
                continue;
            addPrimalOperandsToWorkList(child);
        }

        while (workList.getCount() > 0)
        {
            auto use = workList.getLast();
            workList.removeLast();

            if (processedUses.Contains(use))
                continue;

            processedUses.Add(use);

            HoistResult result = this->apply(use);

            if (result.mode == HoistResult::Mode::Store)
            {
                SLANG_ASSERT(!recomputeSet.Contains(result.instToStore));
                storeSet.Add(result.instToStore);
            }
            else if (result.mode == HoistResult::Mode::Recompute)
            {
                SLANG_ASSERT(!storeSet.Contains(result.instToRecompute));
                recomputeSet.Add(result.instToRecompute);

                if (!as<IRParam>(result.instToRecompute))
                {
                    if (as<IRVar>(result.instToRecompute))
                    {
                        // TODO: First immediate store and add to worklist
                    }
                    else
                    {
                        addPrimalOperandsToWorkList(result.instToRecompute);
                    }
                }
                else
                {
                    // TODO: Add all branch args to worklist.
                }
            }
            else if (result.mode == HoistResult::Mode::Invert)
            {
                SLANG_ASSERT(containsOperand(result.inversionInfo.instToInvert, use->getUser()));
                invertSet.Add(result.inversionInfo.instToInvert);
                invertInfoMap[use] = result.inversionInfo;
            }
        }
    }

    // Go back over the insts and move/clone them accoridngly.
    for (auto block : func->getBlocks())
    {
        // Skip parameter block.
        if (block == func->getFirstBlock())
            continue;

        if (!block->findDecoration<IRDifferentialInstDecoration>())
            continue;
        
        //IRBuilder builder(func->getModule());
        //builder.setInsertB();
        
        auto firstDiffInst = as<IRBlock>(splitInfo->diffBlockMap[block])->getFirstOrdinaryInst();

        auto firstParam = block->getFirstParam();

        IRBuilder invBuilder(func->getModule());

        for (auto child : block->getChildren())
        {
            if (recomputeSet.Contains(child))
            {
                if (!as<IRParam>(child))
                {
                    auto loc = IRInsertLoc::before(firstDiffInst);
                    child->insertAt(loc);
                }
                else
                {
                    auto loc = IRInsertLoc::before(firstParam);
                    child->insertAt(loc);
                }
            }
            else if (storeSet.Contains(child))
            {
                // Do nothing here..
            }
            else if (invertSet.Contains(child))
            {
                IRCloneEnv cloneEnv;

                // Handle this later
                // Have a 'pendingUses' set that
                // we fill as soon as we finish cloning.
                // TODO: We do need a way to detect circular inversion dependencies.
                // But we could leave all this for later.
                //
                
                invBuilder.setInsertAfter()
                auto clonedInst = cloneInst(&builder, cloneEnv, child);
            }
        }
    }
}

void DefaultCheckpointPolicy::preparePolicy(IRGlobalValueWithCode*)
{
    AutodiffCheckpointPolicyBase::findInstsWithDiffUses();
    return;
}



bool DefaultCheckpointPolicy::shouldStoreInst(IRInst* inst)
{
    return true;
}

bool DefaultCheckpointPolicy::shouldStoreCallContext(IRCall* callInst)
{

}

void maybeCheckpointPrimalInst(
    PrimalHoistContext* context,
    IRBuilder* primalBuilder,
    IRBuilder* diffBuilder,
    IRInst* primalInst)
{
    // Cases to consider on the primalInst
    // 1. primalInst is a global value: ignore entirely
    // 2. primalInst is never used in a differential block.
    //    -> In this case we move this inst to primal block and
    //       forget it
    // 3. primalInst is (transitively) used in a differential inst.
    //    3.a. primalInst is a value type.
    //         -> call checkpointPolicy->shouldStoreInst(inst).
    //            if true, move to primalBlock, and add IRPrimalValueStoreDecoration
    //                     push to context->storedInsts
    //            if false, move to diffBlock, and keep IRPrimalInstDecoration
    //                     push to context->recomputedInsts
    //    3.b. primalInst is a var type.
    //         3.b.i primalInst is an intermediate context var
    //               -> call checkpointPolicy->shouldStoreContext(call_inst)
    //                  same as 3.a (instead of shouldStoreInst)
    //         3.b.ii primalInst is regular var (likely used for out/inout parameter of calls)
    //               -> call checkpointPolicy->shouldStoreInst(store_inst)
    //                  same as 3.a (instead of shouldStoreInst)
    //
    // Finally, if we store the inst, call hoistPrimalInst(inst)
}

// TODO: Should be called _after_ all insts are processed in unzip().
void hoistPrimalInst(
    PrimalHoistContext* context,
    BlockSplitInfo* splitInfo,
    IRBuilder* primalBuilder,
    IRBuilder* diffBuilder,
    IRInst* primalInst)
{
    // Check if inst is available in the diff block. If not, hoist to 
    // a variable. The exact type of the variable can vary depending on
    // whether the inst is generated inside a loop.
    //

    auto firstBlock = splitInfo->primalBlockMap[context->func->getFirstBlock()];
    if (!firstBlock)
        return;
    Dictionary<IRInst*, IRVar*> instVars;
    Dictionary<IRBlock*, IRCloneEnv> cloneEnvs;
    auto storeInstAsLocalVar = [&](IRInst* inst)
    {
        IRVar* var = nullptr;
        if (instVars.TryGetValue(inst, var))
            return var;
        IRBuilder builder(diffPropFunc);
        builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());
        var = builder.emitVar(inst->getDataType());
        builder.emitStore(var, builder.emitDefaultConstruct(inst->getDataType()));

        setInsertAfterOrdinaryInst(&builder, inst);
        builder.emitStore(var, inst);
        instVars[inst] = var;
        return var;
    };

    IRBuilder builder(diffPropFunc);
    List<IRInst*> workList;
    for (auto block : diffPropFunc->getBlocks())
    {
        if (!block->findDecoration<IRDifferentialInstDecoration>())
            continue;
        cloneEnvs[block] = IRCloneEnv();
        for (auto inst : block->getChildren())
        {
            workList.add(inst);
        }
    }

    for (Index i = 0; i < workList.getCount(); i++)
    {
        auto inst = workList[i];
        for (UInt j = 0; j < inst->getOperandCount(); j++)
        {
            auto operand = inst->getOperand(j);
            if (operand->getOp() == kIROp_Block)
                continue;
            auto operandParent = inst->getOperand(j)->getParent();
            if (!operandParent)
                continue;
            if (operandParent->parent != diffPropFunc)
                continue;
            if (domTree->dominates(operandParent, inst->parent))
                continue;

            // The def site of the operand does not dominate the use.
            // We need to insert a local variable to store this var.

            IRInst* operandReplacement = nullptr;
            if (canInstBeStored(operand))
            {
                auto var = storeInstAsLocalVar(operand);
                builder.setInsertBefore(inst);
                operandReplacement = builder.emitLoad(var);
            }
            else if (operand->getOp() == kIROp_Var)
            {
                // Var can just be hoisted to first block.
                operand->insertBefore(firstBlock->getFirstOrdinaryInst());
            }
            else
            {
                // For all other insts, we need to copy it to right before this inst.
                // Before actually copying it, check if we have already copied it to
                // any blocks that dominates this block.
                auto dom = as<IRBlock>(inst->getParent());
                while (dom)
                {
                    auto subCloneEnv = cloneEnvs.TryGetValue(dom);
                    if (!subCloneEnv) break;
                    if (subCloneEnv->mapOldValToNew.TryGetValue(operand, operandReplacement))
                    {
                        break;
                    }
                    dom = domTree->getImmediateDominator(dom);
                }
                // We have not found an existing clone in dominators, so we need to copy it
                // to this block.
                if (!operandReplacement)
                {
                    auto subCloneEnv = cloneEnvs.TryGetValue(as<IRBlock>(inst->getParent()));
                    builder.setInsertBefore(inst);
                    operandReplacement = cloneInst(subCloneEnv, &builder, operand);
                    workList.add(operandReplacement);
                }
            }
            if (operandReplacement)
                builder.replaceOperand(inst->getOperands() + j, operandReplacement);
        }
    }
}

};