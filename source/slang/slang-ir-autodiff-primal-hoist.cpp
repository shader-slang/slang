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
// _after_ calling apply() on _all_ insts. Use BlockSplitInfo
// to simply move the insts to top-of-diifferential-block
// 
// Then we traverse through the primal blocks _again_ looking for 
// insts in the invertSet, make a _clone_ of these and insert them into
// the diff block. Use hoistBeforeDiffBlockUses() for this. TODO: But we also have
// to insert _after_ operands are available created.. 
// Find the block which is _dominated_ by all operand sites.
//
// Then call ensurePrimalInstAvailability(inst) on all the insts in 
// storeSet. This will use a dominator tree to check if the inst can
// be accessed. If not, create a var based on the level of indexing.
// Any Load/GetElementPtr inst lowered into the diff blocks will be
// tagged as 'PrimalRecompute'
// 
// 

void AutodiffCheckpointPolicyBase::processFunc(IRGlobalValueWithCode* func)
{
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
                SLANG_ASSERT(!recomputedInstSet.Contains(result.valueSrcInst));
                storedInsts.add(result.valueSrcInst);
                storedInstSet.Add(result.valueSrcInst);
            }
            else if (result.mode == HoistResult::Mode::Recompute)
            {
                SLANG_ASSERT(!storedInstSet.Contains(result.valueSrcInst));
                recomputedInsts.add(result.valueSrcInst);
                recomputedInstSet.Add(result.valueSrcInst);

                addPrimalOperandsToWorkList(result.valueSrcInst);

                // How do we actually move the recompute inst to the right place?
                moveInstToBeforeDiffUses()
            }
            else if (result.mode == HoistResult::Mode::Invert)
            {
                SLANG_ASSERT(containsOperand(result.valueSrcInst, use));
                invertedInsts.add(result.valueSrcInst);
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