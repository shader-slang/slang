#include "slang-ir-autodiff-primal-hoist.h"
#include "slang-ir-autodiff-region.h"
#include "slang-ir-simplify-cfg.h"

namespace Slang
{

void applyCheckpointSet(
    CheckpointSetInfo* checkpointInfo,
    IRGlobalValueWithCode* func,
    HoistedPrimalsInfo* hoistInfo,
    HashSet<IRUse*>& pendingUses,
    Dictionary<IRBlock*, IRBlock*>& mapPrimalBlockToRecomputeBlock,
    IROutOfOrderCloneContext* cloneCtx,
    Dictionary<IRBlock*, List<IndexTrackingInfo>>& blockIndexInfo);

bool containsOperand(IRInst* inst, IRInst* operand)
{
    for (UIndex ii = 0; ii < inst->getOperandCount(); ii++)
        if (inst->getOperand(ii) == operand)
            return true;

    return false;
}

static bool isDifferentialInst(IRInst* inst)
{
    auto parent = inst->getParent();
    if (parent->findDecoration<IRDifferentialInstDecoration>())
        return true;
    return inst->findDecoration<IRDifferentialInstDecoration>() != nullptr;
}

static bool isDifferentialBlock(IRBlock* block)
{
    return block->findDecoration<IRDifferentialInstDecoration>();
}

static IRBlock* getLoopRegionBodyBlock(IRLoop* loop)
{
    auto condBlock = as<IRBlock>(loop->getTargetBlock());
    // We assume the loop body always sit at the true side of the if-else.
    if (auto ifElse = as<IRIfElse>(condBlock->getTerminator()))
    {
        return ifElse->getTrueBlock();
    }
    return nullptr;
}

static IRBlock* tryGetSubRegionEndBlock(IRInst* terminator)
{
    auto loop = as<IRLoop>(terminator);
    if (!loop)
        return nullptr;
    return loop->getBreakBlock();
}

static Dictionary<IRBlock*, IRBlock*> createPrimalRecomputeBlocks(
    IRGlobalValueWithCode* func,
    Dictionary<IRBlock*, List<IndexTrackingInfo>>& indexedBlockInfo,
    IROutOfOrderCloneContext* cloneCtx)
{
    IRBlock* firstDiffBlock = nullptr;
    for (auto block : func->getBlocks())
    {
        if (isDifferentialBlock(block))
        {
            firstDiffBlock = block;
            break;
        }
    }
    if (!firstDiffBlock)
        return Dictionary<IRBlock*, IRBlock*>();

    Dictionary<IRLoop*, IRLoop*> mapPrimalLoopToDiffLoop;
    for (auto block : func->getBlocks())
    {
        if (isDifferentialBlock(block))
        {
            if (auto diffLoop = as<IRLoop>(block->getTerminator()))
            {
                if (auto diffDecor = diffLoop->findDecoration<IRDifferentialInstDecoration>())
                {
                    mapPrimalLoopToDiffLoop[as<IRLoop>(diffDecor->getPrimalInst())] = diffLoop;
                }
            }
        }
    }

    IRBuilder builder(func);
    Dictionary<IRBlock*, IRBlock*> recomputeBlockMap;

    // Create the first recompute block right before the first diff block,
    // and change all jumps into the diff block to the recompute block instead.
    auto createRecomputeBlock = [&](IRBlock* primalBlock)
    {
        auto recomputeBlock = builder.createBlock();
        recomputeBlock->insertAtEnd(func);
        builder.addDecoration(recomputeBlock, kIROp_RecomputeBlockDecoration);
        recomputeBlockMap.add(primalBlock, recomputeBlock);
        indexedBlockInfo[recomputeBlock] = indexedBlockInfo[primalBlock].getValue();
        return recomputeBlock;
    };
    
    auto firstRecomputeBlock = createRecomputeBlock(func->getFirstBlock());
    firstRecomputeBlock->insertBefore(firstDiffBlock);
    moveParams(firstRecomputeBlock, firstDiffBlock);
    firstDiffBlock->replaceUsesWith(firstRecomputeBlock);

    struct WorkItem
    {
        // The first primal block in this region.
        IRBlock* primalBlock;

        // The recompute block created for the first primal block in this region.
        IRBlock* recomptueBlock;

        // The end of primal block in tihs region.
        IRBlock* regionEndBlock; 

        // The first diff block in this region.
        IRBlock* firstDiffBlock;
    };

    List<WorkItem> workList;
    WorkItem firstWorkItem = { func->getFirstBlock(), firstRecomputeBlock, firstRecomputeBlock, firstDiffBlock };
    workList.add(firstWorkItem);

    recomputeBlockMap[func->getFirstBlock()] = firstRecomputeBlock;

    for (Index i = 0; i < workList.getCount(); i++)
    {
        auto workItem = workList[i];
        auto primalBlock = workItem.primalBlock;
        auto recomputeBlock = workItem.recomptueBlock;

        List<IndexTrackingInfo>* thisBlockIndexInfo = indexedBlockInfo.tryGetValue(primalBlock);
        if (!thisBlockIndexInfo)
            continue;

        builder.setInsertInto(recomputeBlock);
        if (auto subRegionEndBlock = tryGetSubRegionEndBlock(primalBlock->getTerminator()))
        {
            // The terminal inst of primalBlock marks the start of a sub loop region?
            // We need to queue work for both the next region after the loop at the current level,
            // and for the sub region for the next level.
            if (subRegionEndBlock == workItem.regionEndBlock)
            {
                // We have reached the end of top-level region, jump to first diff block.
                builder.emitBranch(workItem.firstDiffBlock);
            }
            else
            {
                // Have we already created a recompute block for this target?
                // If so, use it.
                IRBlock* existingRecomputeBlock = nullptr;
                if (recomputeBlockMap.tryGetValue(subRegionEndBlock, existingRecomputeBlock))
                {
                    builder.emitBranch(existingRecomputeBlock);
                }
                else
                {
                    // Queue work for the next region after the subregion at this level.
                    auto nextRegionRecomputeBlock = createRecomputeBlock(subRegionEndBlock);
                    nextRegionRecomputeBlock->insertAfter(recomputeBlock);
                    builder.emitBranch(nextRegionRecomputeBlock);

                    {
                        WorkItem newWorkItem = {
                            subRegionEndBlock,
                            nextRegionRecomputeBlock,
                            workItem.regionEndBlock,
                            workItem.firstDiffBlock };
                        workList.add(newWorkItem);
                    }
                }
            }
            // Queue work for the subregion.
            auto loop = as<IRLoop>(primalBlock->getTerminator());
            auto bodyBlock = getLoopRegionBodyBlock(loop);
            auto diffLoop = mapPrimalLoopToDiffLoop[loop].getValue();
            auto diffBodyBlock = getLoopRegionBodyBlock(diffLoop);
            auto bodyRecomputeBlock = createRecomputeBlock(bodyBlock);
            bodyRecomputeBlock->insertBefore(diffBodyBlock);
            diffBodyBlock->replaceUsesWith(bodyRecomputeBlock);
            moveParams(bodyRecomputeBlock, diffBodyBlock);
            {
                // After CFG normalization, the loop body will contain only jumps to the
                // beginning of the loop.
                // If we see such a jump, it means we have reached the end of current
                // region in the loop.
                // Therefore, we set the regionEndBlock for the sub-region as loop's target
                // block.
                WorkItem newWorkItem = {
                    bodyBlock, bodyRecomputeBlock, loop->getTargetBlock(), diffBodyBlock};
                workList.add(newWorkItem);
            }
        }
        else
        {
            // This is a normal control flow, just copy the CFG structure.
            auto terminator = primalBlock->getTerminator();
            IRInst* newTerminator = nullptr;
            switch (terminator->getOp())
            {
            case kIROp_Switch:
            case kIROp_ifElse:
                newTerminator = cloneCtx->cloneInstOutOfOrder(&builder, primalBlock->getTerminator());
                break;
            case kIROp_unconditionalBranch:
                newTerminator = builder.emitBranch(as<IRUnconditionalBranch>(terminator)->getTargetBlock());
                break;
            default:
                SLANG_UNREACHABLE("terminator type");
            }

            // Modify jump targets in newTerminator to point to the right recompute block or firstDiffBlock.
            for (UInt op = 0; op < newTerminator->getOperandCount(); op++)
            {
                auto target = as<IRBlock>(newTerminator->getOperand(op));
                if (!target)
                    continue;
                if (target == workItem.regionEndBlock)
                {
                    // This jump target is the end of the current region, we will jump to
                    // firstDiffBlock instead.
                    newTerminator->setOperand(op, workItem.firstDiffBlock);
                    continue;
                }

                // Have we already created a recompute block for this target?
                // If so, use it.
                IRBlock* existingRecomputeBlock = nullptr;
                if (recomputeBlockMap.tryGetValue(target, existingRecomputeBlock))
                {
                    newTerminator->setOperand(op, existingRecomputeBlock);
                    continue;
                }

                // This jump target is a normal part of control flow, clone the next block.
                auto targetRecomputeBlock = createRecomputeBlock(target);
                targetRecomputeBlock->insertBefore(workItem.firstDiffBlock);

                newTerminator->setOperand(op, targetRecomputeBlock);

                // Queue work for the successor.
                WorkItem newWorkItem = {
                    target,
                    targetRecomputeBlock,
                    workItem.regionEndBlock,
                    workItem.firstDiffBlock};
                workList.add(newWorkItem);
            }
        }
    }
    // After this pass, all primal blocks except the condition block and the false block of a loop
    // will have a corresponding recomputeBlock.
    return recomputeBlockMap;
}

RefPtr<HoistedPrimalsInfo> AutodiffCheckpointPolicyBase::processFunc(
    IRGlobalValueWithCode* func,
    Dictionary<IRBlock*, IRBlock*>& mapDiffBlockToRecomputeBlock,
    IROutOfOrderCloneContext* cloneCtx,
    Dictionary<IRBlock*, List<IndexTrackingInfo>>& blockIndexInfo)
{
    collectInductionValues(func);

    RefPtr<CheckpointSetInfo> checkpointInfo = new CheckpointSetInfo();

    RefPtr<IRDominatorTree> domTree = computeDominatorTree(func);

    List<UseOrPseudoUse> workList;
    HashSet<UseOrPseudoUse> processedUses;
    HashSet<IRUse*> usesToReplace;

    auto addPrimalOperandsToWorkList = [&](IRInst* inst)
    {
        UIndex opIndex = 0;
        for (auto operand = inst->getOperands(); opIndex < inst->getOperandCount(); operand++, opIndex++)
        {   
            if (!isDifferentialInst(operand->get()) &&
                !as<IRFunc>(operand->get()) &&
                !as<IRBlock>(operand->get()) &&
                !(as<IRModuleInst>(operand->get()->getParent())) &&
                !isDifferentialBlock(getBlock(operand->get())))
                workList.add(operand);
        }

        // Is the type itself computed within our function? 
        // If so, we'll need to consider that too (this is for existential types, specialize insts, etc)
        // TODO: We might not really need to query the checkpointing algorithm for these
        // since they _have_ to be classified as 'recompute' 
        //
        if (inst->getDataType() && (getParentFunc(inst->getDataType()) == func))
        {
            if (!isDifferentialBlock(getBlock(inst->getDataType())))
                workList.add(&inst->typeUse);
        }
    };

    // Populate recompute/store/invert sets with insts, by applying the policy
    // to them.
    // 
    for (auto block : func->getBlocks())
    {
        // Skip parameter block.
        if (block == func->getFirstBlock())
            continue;

        if (!isDifferentialBlock(block))
            continue;

        for (auto child : block->getChildren())
        {   
            // Special case: Ignore the primals used to construct the return pair.
            if (as<IRMakeDifferentialPair>(child) &&
                as<IRReturn>(child->firstUse->getUser()))
            {
                // quick check
                SLANG_RELEASE_ASSERT(child->firstUse->nextUse == nullptr);
                continue;
            }

            addPrimalOperandsToWorkList(child);

            // We'll be conservative with the decorations we consider as differential uses
            // of a primal inst, in order to avoid weird behaviour with some decorations 
            // 
            for (auto decoration : child->getDecorations())
            {
                if (auto primalCtxDecoration = as<IRBackwardDerivativePrimalContextDecoration>(decoration))
                    workList.add(&primalCtxDecoration->primalContextVar);
                else if (auto loopExitDecoration = as<IRLoopExitPrimalValueDecoration>(decoration))
                    workList.add(&loopExitDecoration->exitVal);
            }
        }

        addPrimalOperandsToWorkList(block->getTerminator());
    }
    
    while (workList.getCount() > 0)
    {
        auto use = workList.getLast();
        workList.removeLast();

        if (processedUses.contains(use))
            continue;

        processedUses.add(use);

        HoistResult result = this->classify(use);

        if (result.mode == HoistResult::Mode::Store)
        {
            SLANG_ASSERT(!checkpointInfo->recomputeSet.contains(result.instToStore));
            checkpointInfo->storeSet.add(result.instToStore);
        }
        else if (result.mode == HoistResult::Mode::Recompute)
        {
            SLANG_ASSERT(!checkpointInfo->storeSet.contains(result.instToRecompute));
            checkpointInfo->recomputeSet.add(result.instToRecompute);

            if (isDifferentialInst(use.user) && use.irUse)
                usesToReplace.add(use.irUse);

            if (auto param = as<IRParam>(result.instToRecompute))
            {
                if (auto inductionInfo = inductionValueInsts.tryGetValue(param))
                {
                    checkpointInfo->loopInductionInfo.addIfNotExists(param, *inductionInfo);
                    continue;
                }

                // Add in the branch-args of every predecessor block.
                auto paramBlock = as<IRBlock>(param->getParent());
                UIndex paramIndex = 0;
                for (auto _param : paramBlock->getParams())
                {
                    if (_param == param) break;
                    paramIndex ++;
                }

                for (auto predecessor : paramBlock->getPredecessors())
                {
                    // If we hit this, the checkpoint policy is trying to recompute 
                    // values across a loop region boundary (we don't currently support this,
                    // and in general this is quite inefficient in both compute & memory)
                    // 
                    SLANG_RELEASE_ASSERT(!domTree->dominates(paramBlock, predecessor));

                    auto branchInst = as<IRUnconditionalBranch>(predecessor->getTerminator());
                    SLANG_ASSERT(branchInst->getOperandCount() > paramIndex);

                    workList.add(&branchInst->getArgs()[paramIndex]);
                }
            }
            else
            {
                if (auto var = as<IRVar>(result.instToRecompute))
                {
                    for (auto varUse = var->firstUse; varUse; varUse = varUse->nextUse)
                    {
                        switch (varUse->getUser()->getOp())
                        {
                        case kIROp_Store:
                        case kIROp_Call:
                            // When we have a var and a store/call insts that writes to the var,
                            // we treat as if there is a pseudo-use of the store/call to compute
                            // the var inst, i.e. the var depends on the store/call, despite
                            // the IR's def-use chain doesn't reflect this.
                            workList.add(UseOrPseudoUse(var, varUse->getUser()));
                            break;
                        }
                    }
                }
                else
                {
                    addPrimalOperandsToWorkList(result.instToRecompute);
                }
            }
        }
    }

    // If a var or call is in recomputeSet, move any var/calls associated with the same call to
    // recomputeSet.
    List<IRInst*> instWorkList;
    HashSet<IRInst*> instWorkListSet;
    for (auto inst : checkpointInfo->recomputeSet)
    {
        switch (inst->getOp())
        {
        case kIROp_Call:
        case kIROp_Var:
            instWorkList.add(inst);
            instWorkListSet.add(inst);
            break;
        }
    }
    for (Index i = 0; i < instWorkList.getCount(); i++)
    {
        auto inst = instWorkList[i];
        if (auto var = as<IRVar>(inst))
        {
            for (auto use = var->firstUse; use; use = use->nextUse)
            {
                if (auto callUser = as<IRCall>(use->getUser()))
                {
                    checkpointInfo->recomputeSet.add(callUser);
                    checkpointInfo->storeSet.remove(callUser);
                    if (instWorkListSet.add(callUser))
                        instWorkList.add(callUser);
                }
                else if (auto storeUser = as<IRStore>(use->getUser()))
                {
                    checkpointInfo->recomputeSet.add(storeUser);
                    checkpointInfo->storeSet.remove(storeUser);
                    if (instWorkListSet.add(callUser))
                        instWorkList.add(callUser);
                }
            }
        }
        else if (auto call = as<IRCall>(inst))
        {
            for (UInt j = 0; j < call->getArgCount(); j++)
            {
                if (auto varArg = as<IRVar>(call->getArg(j)))
                {
                    checkpointInfo->recomputeSet.add(varArg);
                    checkpointInfo->storeSet.remove(varArg);
                    if (instWorkListSet.add(varArg))
                        instWorkList.add(varArg);
                }
            }
        }
    }

    RefPtr<HoistedPrimalsInfo> hoistInfo = new HoistedPrimalsInfo();
    applyCheckpointSet(checkpointInfo, func, hoistInfo, usesToReplace, mapDiffBlockToRecomputeBlock, cloneCtx, blockIndexInfo);
    return hoistInfo;
}

void AutodiffCheckpointPolicyBase::collectInductionValues(IRGlobalValueWithCode* func)
{
    // Collect loop induction values.
    // There are two special phi insts we want to handle differently in our
    // checkpointing policy:
    // 1. a bool execution flag inserted as the result of CFG normalization,
    //    that is always true as long as the loop is still active.
    // 2. the original induction variable that can be replaced with the loop
    //    counter we inserted during createPrimalRecomputeBlocks().

    for (auto block : func->getBlocks())
    {
        auto loopInst = as<IRLoop>(block->getTerminator());
        if (!loopInst)
            continue;
        auto targetBlock = loopInst->getTargetBlock();
        auto ifElse = as<IRIfElse>(targetBlock->getTerminator());
        Int paramIndex = -1;
        Int conditionParamIndex = -1;
        // First, we are going to collect all the bool execution flags from loops.
        // These are very easy to identify: they are a phi param defined in
        // targetBlock, and used as the condition value in the condtion block.
        for (auto param : targetBlock->getParams())
        {
            paramIndex++;
            if (!param->getDataType())
                continue;
            if (param->getDataType()->getOp() == kIROp_BoolType)
            {
                if (ifElse->getCondition() == param)
                {
                    // The bool param is used as the condition of the if-else inside the loop,
                    // this param will always be true during the loop, and we don't need to store it.
                    LoopInductionValueInfo info;
                    info.kind = LoopInductionValueInfo::Kind::AlwaysTrue;
                    inductionValueInsts[param] = info;
                    conditionParamIndex = paramIndex;
                }
            }
        }
        if (conditionParamIndex == -1)
            continue;

        // Next, we try to identify the original induction variables, if they exist.
        // These are trickier, and we have to hard code the complex pattern that
        // we can recognize.
        // This pattern matching logic is ugly and fragile against changes to cfg
        // normalization, but it is the easiest way to do it right now.
        // Basically, we are looking for this pattern:
        // loop(..., i=initVal)
        // {
        // targetBlock:
        //      ...
        //      param int i;
        //      param bool condition;
        //      ...
        //      branch condtionBlock;
        // conditionBlock:
        //      if (condition)
        //      {
        //      }
        //      else
        //      {
        //          break;
        //      }
        // //  ...
        // someBodyBlock:
        //     ...
        //     if (condition)
        //     {
        //         ...
        //         // Check condition 1: i is used by an `add`
        //         // Check condition 2: parent of (i+1) is a branch target of if(condition)
        //         // Check condition 3: branches to parentBlock with i1 = i + 1.
        //         goto parentBlock(i + 1); 
        //     }
        //     else
        //         goto parentBlock(other);
        //  parentBlock:
        //     // Check condition 4: parentBlock branches to finalBlock.
        //     param int i1;
        //     goto finalBlock;
        //  finalBlock:
        //     // Check condition 5: finalBlock branches to targetBlock with new i = i1.
        //     goto loopHeader(i1);
        // }
        //
        paramIndex = -1;
        for (auto param : targetBlock->getParams())
        {
            paramIndex++;
            if (!param->getDataType())
                continue;
            if (isScalarIntegerType(param->getDataType()))
            {
                // If the param is always equal to the loop index, we don't need to store it.
                IRInst* addUse = nullptr;
                for (auto use = param->firstUse; use && !addUse; use = use->nextUse)
                {
                    auto user = use->getUser();
                    if (user->getOp() != kIROp_Add)
                        continue;
                    auto intLit = as<IRIntLit>(use->getUser()->getOperand(1));
                    if (!intLit)
                        continue;
                    if (intLit->getValue() != 1)
                        continue;

                    // The add inst's parent block is behind a `ifelse(loopCondition)`.
                    auto addInstBlock = as<IRBlock>(user->getParent());
                    if (!addInstBlock)
                        continue;
                    auto predecessors = addInstBlock->getPredecessors();
                    if (predecessors.getCount() != 1)
                        continue;
                    auto parentIfElse = as<IRIfElse>(predecessors.b->getUser());
                    if (!parentIfElse)
                        continue;
                    auto parentCondition = parentIfElse->getCondition();

                    auto branch = as<IRUnconditionalBranch>(addInstBlock->getTerminator());
                    if (!branch)
                        continue;

                    // The add inst should be used as a branchArg.
                    UInt argIndex = 0;
                    for (UInt i = 0; i < branch->getArgCount(); i++)
                    {
                        if (branch->getArg(i) == user)
                        {
                            addUse = user;
                            argIndex = i;
                            break;
                        }
                    }
                    if (!addUse)
                        continue;
                    auto branchTarget1 = branch->getTargetBlock();
                    auto branchParam = branchTarget1->getFirstParam();
                    for (UInt i = 0; i < argIndex; i++)
                        if (branchParam)
                            branchParam = branchParam->getNextParam();
                    if (!branchParam)
                        continue;

                    // The branchParam is used as argument to branch back to loop header.
                    auto branch2 = as<IRUnconditionalBranch>(branchTarget1->getTerminator());
                    if (!branch2)
                        continue;
                    if (branch2->getTargetBlock() != targetBlock)
                        continue;
                    argIndex = 0;
                    for (UInt i = 0; i < branch2->getArgCount(); i++)
                    {
                        if (branch2->getArg(i) == branchParam)
                        {
                            argIndex = i;
                            break;
                        }
                    }
                    if (argIndex != (UInt)paramIndex)
                        continue;

                    // parentCondition is also used as the new condition in the back jump.
                    if (conditionParamIndex < 0 || (UInt)conditionParamIndex >= branch2->getArgCount() ||
                        branch2->getArg((UInt)conditionParamIndex) != parentCondition)
                        continue;

                    // The use of the add inst matches all of our conditions as an induction value
                    // that is equivalent to loop counter.
                    LoopInductionValueInfo info;
                    info.kind = LoopInductionValueInfo::Kind::EqualsToCounter;
                    info.loopInst = loopInst;
                    info.counterOffset = loopInst->getArg(paramIndex);
                    inductionValueInsts[param] = info;
                    break;
                }
            }
        }
    }
}

void applyToInst(
    IRBuilder* builder,
    CheckpointSetInfo* checkpointInfo,
    HoistedPrimalsInfo* hoistInfo,
    IROutOfOrderCloneContext* cloneCtx,
    Dictionary<IRBlock*, List<IndexTrackingInfo>>& blockIndexInfo,
    IRInst* inst)
{
    // Early-out..
    if (checkpointInfo->storeSet.contains(inst))
    {
        hoistInfo->storeSet.add(inst);
        return;
    }


    bool isInstRecomputed = checkpointInfo->recomputeSet.contains(inst);
    if (isInstRecomputed)
    {
        if (as<IRParam>(inst))
        {
            // Can completely ignore first block parameters
            if (getBlock(inst) == getBlock(inst)->getParent()->getFirstBlock())
            {    
                return;
            }
            // If this is loop condition, it is always true in reverse blocks.
            LoopInductionValueInfo inductionValueInfo;
            if (checkpointInfo->loopInductionInfo.tryGetValue(inst, inductionValueInfo))
            {
                IRInst* replacement = nullptr;
                if (inductionValueInfo.kind == LoopInductionValueInfo::Kind::AlwaysTrue)
                {
                    replacement = builder->getBoolValue(true);
                }
                else if (inductionValueInfo.kind == LoopInductionValueInfo::Kind::EqualsToCounter)
                {
                    auto indexInfo = blockIndexInfo.tryGetValue(inductionValueInfo.loopInst->getTargetBlock());
                    SLANG_ASSERT(indexInfo);
                    SLANG_ASSERT(indexInfo->getCount() != 0);
                    replacement = indexInfo->getFirst().diffCountParam;
                    if (inductionValueInfo.counterOffset)
                    {
                        setInsertAfterOrdinaryInst(builder, replacement);
                        replacement = builder->emitAdd(
                            replacement->getDataType(),
                            replacement,
                            inductionValueInfo.counterOffset);
                    }
                }
                SLANG_ASSERT(replacement);
                cloneCtx->cloneEnv.mapOldValToNew[inst] = replacement;
                cloneCtx->registerClonedInst(builder, inst, replacement);
                return;
            }
        }

        auto recomputeInst = cloneCtx->cloneInstOutOfOrder(builder, inst);
        hoistInfo->recomputeSet.add(recomputeInst);
    }

    bool isInstInverted = checkpointInfo->invertSet.contains(inst);
    if (isInstInverted)
    {
        InversionInfo info = checkpointInfo->invInfoMap[inst];
        auto clonedInstToInvert = cloneCtx->cloneInstOutOfOrder(builder, info.instToInvert);

        // Process operand set for the inverse inst.
        List<IRInst*> newOperands;
        for (auto operand : info.requiredOperands)
        {
            if (cloneCtx->cloneEnv.mapOldValToNew.containsKey(operand))
                newOperands.add(cloneCtx->cloneEnv.mapOldValToNew[operand]);
            else
                newOperands.add(operand);
        }

        info.requiredOperands = newOperands;

        hoistInfo->invertInfoMap[clonedInstToInvert] = info;
        hoistInfo->instsToInvert.add(clonedInstToInvert);
        hoistInfo->invertSet.add(cloneCtx->cloneInstOutOfOrder(builder, inst));
    }
}

static IRBlock* getParamPreludeBlock(IRGlobalValueWithCode* func)
{
    return func->getFirstBlock()->getNextBlock();
}

void applyCheckpointSet(
    CheckpointSetInfo* checkpointInfo,
    IRGlobalValueWithCode* func,
    HoistedPrimalsInfo* hoistInfo,
    HashSet<IRUse*>& pendingUses,
    Dictionary<IRBlock*, IRBlock*>& mapPrimalBlockToRecomputeBlock,
    IROutOfOrderCloneContext* cloneCtx,
    Dictionary<IRBlock*, List<IndexTrackingInfo>>& blockIndexInfo)
{
    for (auto use : pendingUses)
        cloneCtx->pendingUses.add(use);

    // Go back over the insts and move/clone them accoridngly.
    auto paramPreludeBlock = getParamPreludeBlock(func);
    for (auto block : func->getBlocks())
    {
        // Skip parameter block and the param prelude block.
        if (block == func->getFirstBlock() || block == paramPreludeBlock)
            continue;

        if (isDifferentialBlock(block))
            continue;
        
        if (block->findDecoration<IRRecomputeBlockDecoration>())
            continue;

        IRBlock* recomputeBlock = block;
        mapPrimalBlockToRecomputeBlock.tryGetValue(block, recomputeBlock);
        auto recomputeInsertBeforeInst = recomputeBlock->getFirstOrdinaryInst();

        IRBuilder builder(func->getModule());
        UIndex ii = 0;
        for (auto param : block->getParams())
        {
            builder.setInsertBefore(recomputeInsertBeforeInst);
            bool isRecomputed = checkpointInfo->recomputeSet.contains(param);
            bool isInverted = checkpointInfo->invertSet.contains(param);
            bool loopInductionInfo = checkpointInfo->loopInductionInfo.tryGetValue(param);
            if (!isRecomputed && !isInverted)
                continue;

            if (!loopInductionInfo)
            {
                SLANG_RELEASE_ASSERT(
                    recomputeBlock != block &&
                    "recomputed param should belong to block that has recompute block.");
            }

            // Apply checkpoint rule to the parameter itself.
            applyToInst(&builder, checkpointInfo, hoistInfo, cloneCtx, blockIndexInfo, param);

            if (loopInductionInfo)
                continue;

            // Copy primal branch-arg for predecessor blocks.
            HashSet<IRBlock*> predecessorSet;
            for (auto predecessor : block->getPredecessors())
            {
                if (predecessorSet.contains(predecessor))
                    continue;
                predecessorSet.add(predecessor);
                
                auto primalPhiArg = as<IRUnconditionalBranch>(predecessor->getTerminator())->getArg(ii);
                auto recomputePredecessor = mapPrimalBlockToRecomputeBlock[predecessor].getValue();

                // For now, find the primal phi argument in this predecessor,
                // and stick it into the recompute predecessor's branch inst. We
                // will use a patch-up pass in the end to replace all these
                // arguments to their recomputed versions if they exist.
                
                if (isRecomputed)
                {
                    IRInst* terminator = recomputeBlock->getTerminator();
                    addPhiOutputArg(&builder,
                        recomputePredecessor,
                        terminator,
                        primalPhiArg);
                }
                else if (isInverted)
                {
                    IRInst* terminator = recomputeBlock->getTerminator();
                    addPhiOutputArg(&builder,
                        recomputePredecessor,
                        terminator,
                        primalPhiArg);
                }
            }
            ii++;
        }


        for (auto child : block->getChildren())
        {
            // Determine the insertion point for the recomputeInst.
            // Normally we insert recomputeInst into the block's corresponding recomputeBlock.
            // The exception is a load(inoutParam), in which case we insert the recomputed load
            // at the right beginning of the function to correctly receive the initial parameter
            // value. We can't just insert the load at recomputeBlock because at that point the
            // primal logic may have already updated the param with a new value, and instead we
            // want the original value.
            builder.setInsertBefore(recomputeInsertBeforeInst);    
            if (auto load = as<IRLoad>(child))
            {
                if (load->getPtr()->getOp() == kIROp_Param &&
                    load->getPtr()->getParent() == func->getFirstBlock())
                {
                    builder.setInsertBefore(getParamPreludeBlock(func)->getTerminator());
                }
            }
            applyToInst(&builder, checkpointInfo, hoistInfo, cloneCtx, blockIndexInfo, child);
        }
    }

    // Go through phi arguments in recompute blocks and replace them to
    // recomputed insts if they exist.
    for (auto block : func->getBlocks())
    {
        if (!block->findDecoration<IRRecomputeBlockDecoration>())
            continue;
        auto terminator = block->getTerminator();
        for (UInt i = 0; i < terminator->getOperandCount(); i++)
        {
            auto arg = terminator->getOperand(i);
            if (as<IRBlock>(arg))
                continue;
            if (auto recomputeArg = cloneCtx->cloneEnv.mapOldValToNew.tryGetValue(arg))
            {
                terminator->setOperand(i, *recomputeArg);
            }
        }
    }
}

IRType* getTypeForLocalStorage(
    IRBuilder* builder,
    IRType* storageType,
    const List<IndexTrackingInfo>& defBlockIndices)
{
    for (auto& index : defBlockIndices)
    {
        SLANG_ASSERT(index.status == IndexTrackingInfo::CountStatus::Static);
        SLANG_ASSERT(index.maxIters >= 0);

        storageType = builder->getArrayType(
            storageType,
            builder->getIntValue(
                builder->getUIntType(),
                index.maxIters + 1));
    }

    return storageType;
}

IRVar* emitIndexedLocalVar(
    IRBlock* varBlock,
    IRType* baseType,
    const List<IndexTrackingInfo>& defBlockIndices)
{
    SLANG_RELEASE_ASSERT(!as<IRPtrTypeBase>(baseType));

    IRBuilder varBuilder(varBlock->getModule());
    varBuilder.setInsertBefore(varBlock->getFirstOrdinaryInst());

    IRType* varType = getTypeForLocalStorage(&varBuilder, baseType, defBlockIndices);

    auto var = varBuilder.emitVar(varType);
    varBuilder.emitStore(var, varBuilder.emitDefaultConstruct(varType));

    return var;
}

IRInst* emitIndexedStoreAddressForVar(
    IRBuilder* builder,
    IRVar* localVar,
    const List<IndexTrackingInfo>& defBlockIndices)
{
    IRInst* storeAddr = localVar;
    IRType* currType = as<IRPtrTypeBase>(localVar->getDataType())->getValueType();

    for (auto& index : defBlockIndices)
    {
        currType = as<IRArrayType>(currType)->getElementType();

        storeAddr = builder->emitElementAddress(
            builder->getPtrType(currType),
            storeAddr, 
            index.primalCountParam);
    }

    return storeAddr;
}


IRInst* emitIndexedLoadAddressForVar(
    IRBuilder* builder,
    IRVar* localVar,
    const List<IndexTrackingInfo>& defBlockIndices,
    const List<IndexTrackingInfo>& useBlockIndices)
{
    IRInst* loadAddr = localVar;
    IRType* currType = as<IRPtrTypeBase>(localVar->getDataType())->getValueType();

    for (auto index : defBlockIndices)
    {
        currType = as<IRArrayType>(currType)->getElementType();
        if (useBlockIndices.contains(index))
        {
            // If the use-block is under the same region, use the 
            // differential counter variable
            //
            auto diffCounterCurrValue = index.diffCountParam;

            loadAddr = builder->emitElementAddress(
                builder->getPtrType(currType),
                loadAddr, 
                diffCounterCurrValue);
        }
        else
        {
            // If the use-block is outside this region, use the
            // last available value (by indexing with primal counter minus 1)
            // 
            auto primalCounterCurrValue = index.primalCountParam;
            auto primalCounterLastValue = builder->emitSub(
                primalCounterCurrValue->getDataType(),
                primalCounterCurrValue,
                builder->getIntValue(builder->getIntType(), 1));

            loadAddr = builder->emitElementAddress(
                builder->getPtrType(currType),
                loadAddr, 
                primalCounterLastValue);
        }
    }

    return loadAddr;
}

IRVar* storeIndexedValue(
    IRBuilder* builder,
    IRBlock* defaultVarBlock,
    IRInst* instToStore,
    const List<IndexTrackingInfo>& defBlockIndices)
{
    IRVar* localVar = emitIndexedLocalVar(defaultVarBlock, instToStore->getDataType(), defBlockIndices);

    IRInst* addr = emitIndexedStoreAddressForVar(builder, localVar, defBlockIndices);

    builder->emitStore(addr, instToStore);

    return localVar;
}

IRInst* loadIndexedValue(
    IRBuilder* builder,
    IRVar* localVar,
    const List<IndexTrackingInfo>& defBlockIndices,
    const List<IndexTrackingInfo>& useBlockIndices)
{
    IRInst* addr = emitIndexedLoadAddressForVar(builder, localVar, defBlockIndices, useBlockIndices);

    return builder->emitLoad(addr);
}

bool areIndicesEqual(
    const List<IndexTrackingInfo>& indicesA, 
    const List<IndexTrackingInfo>& indicesB)
{
    if (indicesA.getCount() != indicesB.getCount())
        return false;
    
    for (Index ii = 0; ii < indicesA.getCount(); ii++)
    {
        if (indicesA[ii].primalCountParam != indicesB[ii].primalCountParam)
            return false;
    }

    return true;
}

bool areIndicesSubsetOf(
    List<IndexTrackingInfo>& indicesA, 
    List<IndexTrackingInfo>& indicesB)
{
    if (indicesA.getCount() > indicesB.getCount())
        return false;
    
    for (Index ii = 0; ii < indicesA.getCount(); ii++)
    {
        if (indicesA[ii].primalCountParam != indicesB[ii].primalCountParam)
            return false;
    }

    return true;
}

static int getInstRegionNestLevel(
    Dictionary<IRBlock*, List<IndexTrackingInfo>>& indexedBlockInfo,
    IRBlock* defBlock,
    IRInst* inst)
{
    auto result = indexedBlockInfo[defBlock].getValue().getCount();
    // Loop counters are considered to not belong to the region started by the its loop.
    if (result > 0 && inst->findDecoration<IRLoopCounterDecoration>())
        result--;
    return (int)result;
}


/// Legalizes all accesses to primal insts from recompute and diff blocks.
///
RefPtr<HoistedPrimalsInfo> ensurePrimalAvailability(
    HoistedPrimalsInfo* hoistInfo,
    IRGlobalValueWithCode* func,
    Dictionary<IRBlock*, List<IndexTrackingInfo>>& indexedBlockInfo)
{
    // In general, after checkpointing, we can have a function like the following:
    // ```
    // void func()
    // {
    // primal:
    //      for (int i = 0; i < 5; i++)
    //      {
    //            float x = g(i);
    //            use(x);
    //      }
    // recompute:
    //      ...
    // diff:
    //      for (int i = 5; i >= 0; i--)
    //      {
    //      recompute:
    //          ...
    //      diff:
    //          use_diff(x); // def of x is not dominating this location!
    //      }
    // }
    // ```
    // This function will legalize the access to x in the dff block by creating
    // a proper local variable and insert store/loads, so that the above function
    // will be transformed to:
    // ```
    // void func()
    // {
    // primal:
    //      float x_storage[5];
    //      
    //      for (int i = 0; i < 5; i++)
    //      {
    //            float x = g(i);
    //            x_storage[i] = x;
    //            use(x);
    //      }
    // recompute:
    //      ...
    // diff:
    //      for (int i = 5; i >= 0; i--)
    //      {
    //      recompute:
    //          ...
    //      diff:
    //          use_diff(x_storage[i]);
    //      }
    // }
    //

    RefPtr<IRDominatorTree> domTree = computeDominatorTree(func);
    
    IRBlock* defaultVarBlock = func->getFirstBlock()->getNextBlock();

    IRBuilder builder(func->getModule());

    IRBlock* defaultRecomptueVarBlock = nullptr;
    for (auto block : func->getBlocks())
        if (isDifferentialOrRecomputeBlock(block))
        {
            defaultRecomptueVarBlock = block;
            break;
        }
    SLANG_RELEASE_ASSERT(defaultRecomptueVarBlock);

    OrderedHashSet<IRInst*> processedStoreSet;

    auto ensureInstAvailable = [&](OrderedHashSet<IRInst*>& instSet, bool isRecomputeInst)
    {
        SLANG_ASSERT(!isDifferentialBlock(defaultVarBlock));

        for (auto instToStore : instSet)
        {
            IRBlock* defBlock = nullptr;
            if (const auto ptrInst = as<IRPtrTypeBase>(instToStore->getDataType()))
            {
                auto varInst = as<IRVar>(instToStore);
                auto storeUse = findEarliestUniqueWriteUse(varInst);

                defBlock = getBlock(storeUse->getUser());
            }
            else
                defBlock = getBlock(instToStore);

            SLANG_RELEASE_ASSERT(defBlock);

            List<IRUse*> outOfScopeUses;
            for (auto use = instToStore->firstUse; use;)
            {
                auto nextUse = use->nextUse;

                // Only consider uses in differential blocks. 
                // This method is not responsible for other blocks.
                //
                IRBlock* userBlock = getBlock(use->getUser());
                if (isDifferentialOrRecomputeBlock(userBlock))
                {
                    if (!domTree->dominates(defBlock, userBlock))
                    {
                        outOfScopeUses.add(use);
                    }
                    else if (!areIndicesSubsetOf(indexedBlockInfo[defBlock], indexedBlockInfo[userBlock]))
                    {
                        outOfScopeUses.add(use);
                    }
                    else if (getInstRegionNestLevel(indexedBlockInfo, defBlock, instToStore) > 0 &&
                        !isDifferentialOrRecomputeBlock(defBlock))
                    {
                        outOfScopeUses.add(use);
                    }
                    else if (as<IRPtrTypeBase>(instToStore->getDataType()) &&
                        !isDifferentialOrRecomputeBlock(defBlock))
                    {
                        outOfScopeUses.add(use);
                    }
                }

                use = nextUse;
            }

            if (outOfScopeUses.getCount() == 0)
            {

                if (!isRecomputeInst)
                    processedStoreSet.add(instToStore);
                continue;
            }

            auto defBlockIndices = indexedBlockInfo[defBlock].getValue();
            IRBlock* varBlock = defaultVarBlock;
            if (isRecomputeInst)
            {
                varBlock = defaultRecomptueVarBlock;
                if (defBlockIndices.getCount())
                {
                    varBlock = as<IRBlock>(defBlockIndices[0].diffCountParam->getParent());
                    defBlockIndices.clear();
                }
            }
            if (const auto ptrInst = as<IRPtrTypeBase>(instToStore->getDataType()))
            {
                IRVar* varToStore = as<IRVar>(instToStore);
                SLANG_RELEASE_ASSERT(varToStore);

                auto storeUse = findLatestUniqueWriteUse(varToStore);

                bool isIndexedStore = (storeUse && defBlockIndices.getCount() > 0);

                // TODO: There's a slight hackiness here. (Ideally we might just want to emit
                // additional vars when splitting a call)
                //
                if (!isIndexedStore && isDerivativeContextVar(varToStore))
                {
                    varToStore->insertBefore(defaultVarBlock->getFirstOrdinaryInst());

                    if (!isRecomputeInst)
                        processedStoreSet.add(varToStore);
                    continue;
                }

                setInsertAfterOrdinaryInst(&builder, getInstInBlock(storeUse->getUser()));

                IRVar* localVar = storeIndexedValue(
                    &builder,
                    varBlock,
                    builder.emitLoad(varToStore),
                    defBlockIndices);

                for (auto use : outOfScopeUses)
                {
                    setInsertBeforeOrdinaryInst(&builder, getInstInBlock(use->getUser()));

                    List<IndexTrackingInfo>& useBlockIndices = indexedBlockInfo[getBlock(use->getUser())];

                    IRInst* loadAddr = emitIndexedLoadAddressForVar(&builder, localVar, defBlockIndices, useBlockIndices);
                    builder.replaceOperand(use, loadAddr);
                }

                if (!isRecomputeInst)
                    processedStoreSet.add(localVar);
            }
            else
            {
                // Handle the special case of loop counters.
                // The only case where there will be a reference of primal loop counter from rev blocks
                // is the start of a loop in the reverse code. Since loop counters are not considered a
                // part of their loop region, so we remove the first index info.
                bool isLoopCounter = (instToStore->findDecoration<IRLoopCounterDecoration>() != nullptr);
                if (isLoopCounter)
                {
                    defBlockIndices.removeAt(0);
                }

                setInsertAfterOrdinaryInst(&builder, instToStore);
                auto localVar = storeIndexedValue(&builder, varBlock, instToStore, defBlockIndices);

                for (auto use : outOfScopeUses)
                {
                    List<IndexTrackingInfo> useBlockIndices = indexedBlockInfo[getBlock(use->getUser())];
                    setInsertBeforeOrdinaryInst(&builder, getInstInBlock(use->getUser()));
                    builder.replaceOperand(use, loadIndexedValue(&builder, localVar, defBlockIndices, useBlockIndices));
                }

                if (!isRecomputeInst)
                    processedStoreSet.add(localVar);
            }
        }
    };

    ensureInstAvailable(hoistInfo->storeSet, false);
    ensureInstAvailable(hoistInfo->recomputeSet, true);

    // Replace the old store set with the processed one.
    hoistInfo->storeSet = processedStoreSet;

    return hoistInfo;
}

void tryInferMaxIndex(IndexedRegion* region, IndexTrackingInfo* info)
{
    if (info->status != IndexTrackingInfo::CountStatus::Unresolved)
        return;

    auto loop = as<IRLoop>(region->getInitializerBlock()->getTerminator());

    if (auto maxItersDecoration = loop->findDecoration<IRLoopMaxItersDecoration>())
    {
        info->maxIters = (Count)maxItersDecoration->getMaxIters();
        info->status = IndexTrackingInfo::CountStatus::Static;
    }
}

IRInst* addPhiInputParam(IRBuilder* builder, IRBlock* block, IRType* type)
{
    builder->setInsertInto(block);
    return builder->emitParam(type);
}

IRInst* addPhiInputParam(IRBuilder* builder, IRBlock* block, IRType* type, UIndex index)
{
    List<IRParam*> params;
    for (auto param : block->getParams())
        params.add(param);

    SLANG_RELEASE_ASSERT(index == (UCount)params.getCount());

    return addPhiInputParam(builder, block, type);
}

static IRBlock* getUpdateBlock(IRLoop* loop)
{
    auto initBlock = cast<IRBlock>(loop->getParent());

    auto condBlock = loop->getTargetBlock();

    IRBlock* lastLoopBlock = nullptr;

    for (auto predecessor : condBlock->getPredecessors())
    {
        if (predecessor != initBlock)
            lastLoopBlock = predecessor;
    }

    // Should find atleast one predecessor that is _not_ the 
    // init block (that contains the loop info). This 
    // predecessor would be the last block in the loop
    // before looping back to the condition.
    // 
    SLANG_RELEASE_ASSERT(lastLoopBlock);

    return lastLoopBlock;
}

void lowerIndexedRegion(IRLoop*& primalLoop, IRLoop*& diffLoop, IRInst*& primalCountParam, IRInst*& diffCountParam)
{
    IRBuilder builder(primalLoop);
    primalCountParam = nullptr;

    // Grab first primal block.
    IRBlock* primalInitBlock = as<IRBlock>(primalLoop->getParent());
    builder.setInsertBefore(primalInitBlock->getTerminator());
    {
        auto primalCondBlock = as<IRUnconditionalBranch>(
            primalInitBlock->getTerminator())->getTargetBlock();
        builder.setInsertBefore(primalInitBlock->getTerminator());

        auto phiCounterArgLoopEntryIndex = addPhiOutputArg(
            &builder,
            primalInitBlock,
            *(IRInst**)&primalLoop,
            builder.getIntValue(builder.getIntType(), 0));

        builder.setInsertBefore(primalCondBlock->getTerminator());
        primalCountParam = addPhiInputParam(
            &builder,
            primalCondBlock,
            builder.getIntType(),
            phiCounterArgLoopEntryIndex);
        builder.addLoopCounterDecoration(primalCountParam);
        builder.addNameHintDecoration(primalCountParam, UnownedStringSlice("_pc"));
        builder.markInstAsPrimal(primalCountParam);

        IRBlock* primalUpdateBlock = getUpdateBlock(primalLoop);
        IRInst* terminator = primalUpdateBlock->getTerminator();
        builder.setInsertBefore(primalUpdateBlock->getTerminator());

        auto incCounterVal = builder.emitAdd(
            builder.getIntType(),
            primalCountParam,
            builder.getIntValue(builder.getIntType(), 1));
        builder.markInstAsPrimal(incCounterVal);

        auto phiCounterArgLoopCycleIndex = addPhiOutputArg(&builder, primalUpdateBlock, terminator, incCounterVal);

        SLANG_RELEASE_ASSERT(phiCounterArgLoopEntryIndex == phiCounterArgLoopCycleIndex);
    }

    {
        IRBlock* diffInitBlock = as<IRBlock>(diffLoop->getParent());

        auto diffCondBlock = as<IRUnconditionalBranch>(
            diffInitBlock->getTerminator())->getTargetBlock();
        builder.setInsertBefore(diffInitBlock->getTerminator());
        auto revCounterInitVal = builder.emitSub(
            builder.getIntType(),
            primalCountParam,
            builder.getIntValue(builder.getIntType(), 1));
        auto phiCounterArgLoopEntryIndex = addPhiOutputArg(
            &builder,
            diffInitBlock,
            *(IRInst**)&diffLoop,
            revCounterInitVal);

        builder.setInsertBefore(diffCondBlock->getTerminator());

        diffCountParam = addPhiInputParam(
            &builder,
            diffCondBlock,
            builder.getIntType(),
            phiCounterArgLoopEntryIndex);
        builder.addNameHintDecoration(diffCountParam, UnownedStringSlice("_dc"));
        builder.markInstAsPrimal(diffCountParam);

        IRBlock* diffUpdateBlock = getUpdateBlock(diffLoop);
        builder.setInsertBefore(diffUpdateBlock->getTerminator());
        IRInst* terminator = diffUpdateBlock->getTerminator();

        auto decCounterVal = builder.emitSub(
            builder.getIntType(),
            diffCountParam,
            builder.getIntValue(builder.getIntType(), 1));
        builder.markInstAsPrimal(decCounterVal);

        auto phiCounterArgLoopCycleIndex = addPhiOutputArg(&builder, diffUpdateBlock, terminator, decCounterVal);

        auto ifElse = as<IRIfElse>(diffCondBlock->getTerminator());
        builder.setInsertBefore(ifElse);
        auto exitCondition = builder.emitGeq(diffCountParam, builder.getIntValue(builder.getIntType(), 0));
        ifElse->condition.set(exitCondition);

        SLANG_RELEASE_ASSERT(phiCounterArgLoopEntryIndex == phiCounterArgLoopCycleIndex);
    }
}

// Insert iteration counters for all loops to form indexed regions. For loops in
// primal blocks, the counter is incremented from 0. For loops in reverse
// blocks, the counter is decremented from the final value in primal block
// downto 0. Returns a mapping from each block to a list of their enclosing loop
// regions. A loop region records the iteration counter for the corresponding
// loop in the primal block and the reverse block.
//
void buildIndexedBlocks(
    Dictionary<IRBlock*, List<IndexTrackingInfo>>& info,
    IRGlobalValueWithCode* func)
{
    Dictionary<IRLoop*, IndexTrackingInfo> mapLoopToTrackingInfo;

    for (auto block : func->getBlocks())
    {
        auto loop = as<IRLoop>(block->getTerminator());
        if (!loop) continue;
        auto diffDecor = loop->findDecoration<IRDifferentialInstDecoration>();
        if (!diffDecor) continue;
        auto primalLoop = as<IRLoop>(diffDecor->getPrimalInst());
        if (!primalLoop) continue;

        IndexTrackingInfo indexInfo = {};
        lowerIndexedRegion(primalLoop, loop, indexInfo.primalCountParam, indexInfo.diffCountParam);

        SLANG_RELEASE_ASSERT(indexInfo.primalCountParam);
        SLANG_RELEASE_ASSERT(indexInfo.diffCountParam);

        mapLoopToTrackingInfo[loop] = indexInfo;
        mapLoopToTrackingInfo[primalLoop] = indexInfo;
    }

    auto regionMap = buildIndexedRegionMap(func);

    for (auto block : func->getBlocks())
    {
        List<IndexTrackingInfo> trackingInfos;
        for (auto region : regionMap->getAllAncestorRegions(block))
        {
            IndexTrackingInfo trackingInfo;
            if (mapLoopToTrackingInfo.tryGetValue(region->loop, trackingInfo))
            {
                tryInferMaxIndex(region, &trackingInfo);
                trackingInfos.add(trackingInfo);
            }
        }
        info[block] = trackingInfos;
    }
}

// For each primal inst that is used in reverse blocks, decide if we should recompute or store
// its value, then make them accessible in reverse blocks based the decision.
//
RefPtr<HoistedPrimalsInfo> applyCheckpointPolicy(IRGlobalValueWithCode* func)
{
    sortBlocksInFunc(func);

    // Insert loop counters and establish loop regions.
    // Also makes the reverse loops counting downwards from the final iteration count.
    //
    Dictionary<IRBlock*, List<IndexTrackingInfo>> indexedBlockInfo;
    buildIndexedBlocks(indexedBlockInfo, func);

    // Create recompute blocks for each region following the same control flow structure
    // as in primal code.
    //
    RefPtr<IROutOfOrderCloneContext> cloneCtx = new IROutOfOrderCloneContext();
    auto recomputeBlockMap = createPrimalRecomputeBlocks(func, indexedBlockInfo, cloneCtx);

    sortBlocksInFunc(func);

    // Determine the strategy we should use to make a primal inst available.
    // If we decide to recompute the inst, emit the recompute inst in the corresponding recompute block.
    //
    RefPtr<AutodiffCheckpointPolicyBase> chkPolicy = new DefaultCheckpointPolicy(func->getModule());
    chkPolicy->preparePolicy(func);
    auto primalsInfo = chkPolicy->processFunc(func, recomputeBlockMap, cloneCtx, indexedBlockInfo);

    // Legalize the primal inst accesses by introducing local variables / arrays and emitting
    // necessary load/store logic.
    //
    primalsInfo = ensurePrimalAvailability(primalsInfo, func, indexedBlockInfo);
    return primalsInfo;
}

void DefaultCheckpointPolicy::preparePolicy(IRGlobalValueWithCode* func)
{
    SLANG_UNUSED(func)
    return;
}

enum CheckpointPreference
{
    None,
    PreferCheckpoint,
    PreferRecompute
};

static CheckpointPreference getCheckpointPreference(IRInst* callee)
{
    callee = getResolvedInstForDecorations(callee, true);
    for (auto decor : callee->getDecorations())
    {
        switch (decor->getOp())
        {
        case kIROp_PreferCheckpointDecoration:
            return CheckpointPreference::PreferCheckpoint;
        case kIROp_PreferRecomputeDecoration:
        case kIROp_TargetIntrinsicDecoration:
            return CheckpointPreference::PreferRecompute;
        }
    }
    return CheckpointPreference::None;
}

static bool isGlobalMutableAddress(IRInst* inst)
{
    auto root = getRootAddr(inst);
    if (root)
    {
        if (as<IRParameterGroupType>(root->getDataType()))
        {
            return false;
        }
        return as<IRModuleInst>(root->getParent()) != nullptr;
    }
    return false;
}

static bool isInstInPrimalOrTransposedParameterBlocks(IRInst* inst)
{
    auto func = getParentFunc(inst);
    if (!func)
        return false;
    auto firstBlock = func->getFirstBlock();
    if (inst->getParent() == firstBlock)
        return true;
    auto branch = as<IRUnconditionalBranch>(firstBlock->getTerminator());
    if (!branch)
        return false;
    auto secondBlock = branch->getTargetBlock();
    if (inst->getParent() == secondBlock)
        return true;
    return false;
}

static bool shouldStoreInst(IRInst* inst)
{
    if (!inst->getDataType())
    {
        return false;
    }

    if (!canTypeBeStored(inst->getDataType()))
        return false;

    switch (inst->getOp())
    {
    // Never store these opcodes because they are not real computations.
    case kIROp_CastFloatToInt:
    case kIROp_CastIntToFloat:
    case kIROp_IntCast:
    case kIROp_FloatCast:
    case kIROp_MakeVectorFromScalar:
    case kIROp_MakeMatrixFromScalar:
    case kIROp_Reinterpret:
    case kIROp_BitCast:
    case kIROp_DefaultConstruct:
    case kIROp_MakeStruct:
    case kIROp_MakeTuple:
    case kIROp_MakeArray:
    case kIROp_MakeVector:
    case kIROp_MakeMatrix:
    case kIROp_MakeArrayFromElement:
    case kIROp_MakeDifferentialPair:
    case kIROp_MakeDifferentialPairUserCode:
    case kIROp_MakeOptionalNone:
    case kIROp_MakeOptionalValue:
    case kIROp_MakeExistential:
    case kIROp_DifferentialPairGetDifferential:
    case kIROp_DifferentialPairGetPrimal:
    case kIROp_DifferentialPairGetDifferentialUserCode:
    case kIROp_DifferentialPairGetPrimalUserCode:
    case kIROp_ExtractExistentialValue:
    case kIROp_ExtractExistentialType:
    case kIROp_ExtractExistentialWitnessTable:
    case kIROp_undefined:
    case kIROp_GetSequentialID:
    case kIROp_GetStringHash:
    case kIROp_Specialize:
    case kIROp_LookupWitness:
    case kIROp_Param:
    case kIROp_DetachDerivative:
        return false;
    
    // Never store these op codes because they are trivial to compute.
    case kIROp_Add:
    case kIROp_Sub:
    case kIROp_Mul:
    case kIROp_Div:
    case kIROp_Neg:
    case kIROp_Geq:
    case kIROp_FRem:
    case kIROp_IRem:
    case kIROp_Leq:
    case kIROp_Neq:
    case kIROp_Eql:
    case kIROp_Greater:
    case kIROp_Less:
    case kIROp_And:
    case kIROp_Or:
    case kIROp_Not:
    case kIROp_BitNot:
    case kIROp_BitAnd:
    case kIROp_BitOr:
    case kIROp_BitXor:
    case kIROp_Lsh:
    case kIROp_Rsh:
        return false;

    case kIROp_GetElement:
    case kIROp_FieldExtract:
    case kIROp_swizzle:
    case kIROp_UpdateElement:
    case kIROp_OptionalHasValue:
    case kIROp_GetOptionalValue:
    case kIROp_MatrixReshape:
    case kIROp_VectorReshape:
    case kIROp_GetTupleElement:
        return false;

    case kIROp_Load:
        // In general, don't store loads, because:
        //  - Loads to constant data can just be reloaded.
        //  - Loads to local variables can only exist for the temp variables used for calls,
        //    those variables are written only once so we can always load them anytime.
        //  - Loads to global mutable variables are now allowed, but we will capture that
        //    case in canRecompute().
        //  - The only exception is the load of an inout param, in which case we do need
        //    to store it because the param may be modified by the func at exit. Similarly,
        //    this will be handled in canRecompute().
        return false;

    case kIROp_Call:
        // If the callee prefers recompute policy, don't store.
        if (getCheckpointPreference(inst->getOperand(0)) == CheckpointPreference::PreferRecompute)
        {
            return false;
        }
        break;
    default:
        break;
    }

    if (as<IRType>(inst))
        return false;

    return true;
}

static bool shouldStoreVar(IRVar* var)
{
    if (const auto typeDecor = var->findDecoration<IRBackwardDerivativePrimalContextDecoration>())
    {
        // If we are specializing a callee's intermediate context with types that can't be stored,
        // we can't store the entire context.
        if (auto spec = as<IRSpecialize>(as<IRPtrTypeBase>(var->getDataType())->getValueType()))
        {
            for (UInt i = 0; i < spec->getArgCount(); i++)
            {
                if (!canTypeBeStored(spec->getArg(i)))
                    return false;
            }
        }
    }

    auto storeUse = findLatestUniqueWriteUse(var);
    if (storeUse)
    {
        if (!canTypeBeStored(as<IRPtrTypeBase>(var->getDataType())->getValueType()))
            return false;
        if (auto callUser = as<IRCall>(storeUse->getUser()))
        {
            // If the var is being written to by a call, the decision
            // of the var will be the same as the decision for the call.
            return shouldStoreInst(callUser);
        }
        // Default behavior is to store if we can.
        return true;
    }
    // If the var has never been written to, don't store it.
    return false;
}

bool DefaultCheckpointPolicy::canRecompute(UseOrPseudoUse use)
{
    if (auto load = as<IRLoad>(use.usedVal))
    {
        auto ptr = load->getPtr();

        // We can't recompute a `load` is if it is a load from a global mutable
        // variable.
        if (isGlobalMutableAddress(ptr))
            return false;

        // We can't recompute a 'load' from a mutable function parameter.
        if (as<IRParam>(ptr) || as<IRVar>(ptr))
        {
            if (isInstInPrimalOrTransposedParameterBlocks(ptr))
                return true;
        }
    }
    auto param = as<IRParam>(use.usedVal);
    if (!param)
        return true;

    if (inductionValueInsts.containsKey(param))
        return true;

    // We can recompute a phi param if it is not in a loop start block.
    auto parentBlock = as<IRBlock>(param->getParent());
    for (auto pred : parentBlock->getPredecessors())
    {
        if (auto loop = as<IRLoop>(pred->getTerminator()))
        {
            if (loop->getTargetBlock() == parentBlock)
                return false;
        }
    }
    return true;
}

HoistResult DefaultCheckpointPolicy::classify(UseOrPseudoUse use)
{
    // Store all that we can.. by default, classify will only be called on relevant differential
    // uses (or on uses in a 'recompute' inst)
    // 
    if (auto var = as<IRVar>(use.usedVal))
    {
        if (shouldStoreVar(var))
            return HoistResult::store(var);
        else
            return HoistResult::recompute(var);
    }
    else
    {
        if (shouldStoreInst(use.usedVal))
        {
            return HoistResult::store(use.usedVal);
        }
        else
        {
            // We may not be able to recompute due to limitations of
            // the unzip pass. If so we will store the result.
            if (canRecompute(use))
                return HoistResult::recompute(use.usedVal);

            // The fallback is to store.
            return HoistResult::store(use.usedVal);
        }
    }
}

};
