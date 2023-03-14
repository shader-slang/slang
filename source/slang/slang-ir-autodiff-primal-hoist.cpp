#include "slang-ir-autodiff-primal-hoist.h"
#include "slang-ir-autodiff-region.h"

namespace Slang 
{

bool containsOperand(IRInst* inst, IRInst* operand)
{
    for (UIndex ii = 0; ii < inst->getOperandCount(); ii++)
        if (inst->getOperand(ii) == operand)
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


RefPtr<HoistedPrimalsInfo> AutodiffCheckpointPolicyBase::processFunc(IRGlobalValueWithCode* func, BlockSplitInfo* splitInfo)
{
    RefPtr<CheckpointSetInfo> checkpointInfo = new CheckpointSetInfo();

    RefPtr<IRDominatorTree> domTree = computeDominatorTree(func);

    List<IRUse*> workList;
    HashSet<IRUse*> processedUses;

    HashSet<IRUse*> usesToReplace;
    
    auto addPrimalOperandsToWorkList = [&](IRInst* inst)
    {
        UIndex opIndex = 0;
        for (auto operand = inst->getOperands(); opIndex < inst->getOperandCount(); operand++, opIndex++)
        {   
            if (!operand->get()->findDecoration<IRDifferentialInstDecoration>() &&
                !as<IRFunc>(operand->get()) &&
                !as<IRBlock>(operand->get()) &&
                !as<IRConstant>(operand->get()) &&
                !as<IRType>(operand->get()))
                workList.add(operand);
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

        if (!block->findDecoration<IRDifferentialInstDecoration>())
            continue;

        for (auto child : block->getChildren())
        {
            if (!child->findDecoration<IRDifferentialInstDecoration>())
                continue;
            
            // Ignore the primals used to construct a pair
            if (as<IRMakeDifferentialPair>(child))
            {
                // quick consistency check..
                SLANG_RELEASE_ASSERT(as<IRReturn>(child->firstUse->getUser()));
                continue;
            }

            addPrimalOperandsToWorkList(child);
        }
    }
    
    while (workList.getCount() > 0)
    {
        auto use = workList.getLast();
        workList.removeLast();

        if (processedUses.Contains(use))
            continue;

        processedUses.Add(use);

        HoistResult result = this->classify(use);

        if (result.mode == HoistResult::Mode::Store)
        {
            SLANG_ASSERT(!checkpointInfo->recomputeSet.Contains(result.instToStore));
            checkpointInfo->storeSet.Add(result.instToStore);
        }
        else if (result.mode == HoistResult::Mode::Recompute)
        {
            SLANG_ASSERT(!checkpointInfo->storeSet.Contains(result.instToRecompute));
            checkpointInfo->recomputeSet.Add(result.instToRecompute);

            usesToReplace.Add(use);

            if (auto param = as<IRParam>(result.instToRecompute))
            {
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

                    workList.add(&branchInst->getOperands()[paramIndex]);
                }
            }
            else
            {
                if (auto var = as<IRVar>(result.instToRecompute))
                {
                    IRUse* storeUse = findUniqueStoredVal(var);
                    if (!storeUse)
                        workList.add(storeUse);
                }
                else
                {
                    addPrimalOperandsToWorkList(result.instToRecompute);
                }
            }
        }
        else if (result.mode == HoistResult::Mode::Invert)
        {
            auto instToInvert = result.inversionInfo.instToInvert;

            SLANG_RELEASE_ASSERT(containsOperand(instToInvert, use->getUser()));
            SLANG_RELEASE_ASSERT(result.inversionInfo.targetInsts.contains(use->getUser()));

            usesToReplace.Add(use);

            checkpointInfo->invertSet.Add(instToInvert);

            if (checkpointInfo->invInfoMap.ContainsKey(instToInvert))
            {
                List<IRInst*> currOperands = checkpointInfo->invInfoMap[instToInvert].GetValue().requiredOperands;
                for (Index ii = 0; ii < result.inversionInfo.requiredOperands.getCount(); ii++)
                {
                    SLANG_RELEASE_ASSERT(result.inversionInfo.requiredOperands[ii] == currOperands[ii]);
                }
            }
            else
                checkpointInfo->invInfoMap[instToInvert] = result.inversionInfo;
        }
    }

    return applyCheckpointSet(checkpointInfo, func, splitInfo, usesToReplace);
}

void applyToInst(
    IRBuilder* builder,
    CheckpointSetInfo* checkpointInfo,
    HoistedPrimalsInfo* hoistInfo,
    IROutOfOrderCloneContext* cloneCtx,
    IRInst* inst)
{
    // Early-out..
    if (checkpointInfo->storeSet.Contains(inst))
    {
        hoistInfo->storeSet.Add(inst);
        return;
    }

    bool isParamRecomputed = checkpointInfo->recomputeSet.Contains(inst);
    if (isParamRecomputed)
    {
        // TODO: We would need to clone in the control-flow for each region (without nested loops)
        // prior to this, and then hoist this parameter into the within-region block, otherwise
        // this parameter will not be visible to transposed insts.
        // This will also include adding an extra case to 'ensurePrimalAvailability': if both insts
        // are withing the _same_ indexed region, skip the indexed store/load and use a simple var.
        // 
        SLANG_UNIMPLEMENTED_X("Parameter recompute is not currently supported");
        //hoistInfo->recomputeSet.Add(cloneCtx->cloneInstOutOfOrder(builder, inst));
    }

    bool isParamInverted = checkpointInfo->invertSet.Contains(inst);
    if (isParamInverted)
    {
        InversionInfo info = checkpointInfo->invInfoMap[inst];
        auto clonedInstToInvert = cloneCtx->cloneInstOutOfOrder(builder, info.instToInvert);

        // Process operand set for the inverse inst.
        List<IRInst*> newOperands;
        for (auto operand : info.requiredOperands)
        {
            if (cloneCtx->cloneEnv.mapOldValToNew.ContainsKey(operand))
                newOperands.add(cloneCtx->cloneEnv.mapOldValToNew[operand]);
            else
                newOperands.add(operand);
        }

        info.requiredOperands = newOperands;

        hoistInfo->invertInfoMap[clonedInstToInvert] = info;
        hoistInfo->instsToInvert.Add(clonedInstToInvert);
        hoistInfo->invertSet.Add(cloneCtx->cloneInstOutOfOrder(builder, inst));
    }
}

RefPtr<HoistedPrimalsInfo> applyCheckpointSet(
    CheckpointSetInfo* checkpointInfo,
    IRGlobalValueWithCode* func,
    BlockSplitInfo* splitInfo,
    HashSet<IRUse*> pendingUses)
{
    RefPtr<HoistedPrimalsInfo> hoistInfo = new HoistedPrimalsInfo();

    RefPtr<IROutOfOrderCloneContext> cloneCtx = new IROutOfOrderCloneContext();

    for (auto use : pendingUses)
        cloneCtx->pendingUses.Add(use);
    
    // Populate the clone context with all the primal uses that we may need to replace with
    // cloned versions. That way any insts we clone into the diff block will automatically replace
    // their uses.
    //
    auto addPrimalUsesToCloneContext = [&](IRInst* inst)
    {
        UIndex opIndex = 0;
        for (auto operand = inst->getOperands(); opIndex < inst->getOperandCount(); operand++, opIndex++)
        {   
            if (!operand->get()->findDecoration<IRDifferentialInstDecoration>())
                cloneCtx->pendingUses.Add(operand);
        }
    };

    // Go back over the insts and move/clone them accoridngly.
    for (auto block : func->getBlocks())
    {
        // Skip parameter block.
        if (block == func->getFirstBlock())
            continue;

        if (block->findDecoration<IRDifferentialInstDecoration>())
            continue;

        auto diffBlock = as<IRBlock>(splitInfo->diffBlockMap[block]);

        auto firstDiffInst = as<IRBlock>(splitInfo->diffBlockMap[block])->getFirstOrdinaryInst();

        IRBuilder builder(func->getModule());

        UIndex ii = 0;
        for (auto param : block->getParams())
        {
            builder.setInsertBefore(diffBlock->getFirstOrdinaryInst());

            // Apply checkpoint rule to the parameter itself.
            applyToInst(&builder, checkpointInfo, hoistInfo, cloneCtx, param);

            // Copy primal branch-arg for predecessor blocks.
            HashSet<IRBlock*> predecessorSet;
            for (auto predecessor : block->getPredecessors())
            {
                if (predecessorSet.Contains(predecessor))
                    continue;

                predecessorSet.Add(predecessor);

                auto diffPredecessor = as<IRBlock>(splitInfo->diffBlockMap[block]);
 
                if (checkpointInfo->recomputeSet.Contains(param))
                    addPhiOutputArg(&builder,
                        diffPredecessor,
                        as<IRUnconditionalBranch>(predecessor->getTerminator())->getArg(ii));
                
                if (checkpointInfo->invertSet.Contains(param))
                    addPhiOutputArg(&builder,
                        diffPredecessor,
                        as<IRUnconditionalBranch>(predecessor->getTerminator())->getArg(ii));
            }

            ii++;
        }

        for (auto child : block->getChildren())
        {
            builder.setInsertBefore(firstDiffInst);
            
            applyToInst(&builder, checkpointInfo, hoistInfo, cloneCtx, child);
        }
    }

    return hoistInfo;
}

IRType* getTypeForLocalStorage(
    IRBuilder* builder,
    IRInst* inst,
    List<IndexTrackingInfo*> defBlockIndices)
{
    IRType* storageType = inst->getDataType();

    for (auto index : defBlockIndices)
    {
        SLANG_ASSERT(index->status == IndexTrackingInfo::CountStatus::Static);
        SLANG_ASSERT(index->maxIters >= 0);

        storageType = builder->getArrayType(
            storageType,
            builder->getIntValue(
                builder->getUIntType(),
                index->maxIters + 1));
    }

    return storageType;
}

IRVar* emitLocalVarForValue(
    IRBlock* varBlock,
    IRInst* instToStore,
    List<IndexTrackingInfo*> defBlockIndices)
{
    SLANG_RELEASE_ASSERT(!as<IRPtrTypeBase>(instToStore->getDataType()));

    IRBuilder varBuilder(varBlock->getModule());
    varBuilder.setInsertBefore(varBlock->getFirstOrdinaryInst());

    IRType* varType = getTypeForLocalStorage(&varBuilder, instToStore, defBlockIndices);

    auto var = varBuilder.emitVar(varType);
    varBuilder.emitStore(var, varBuilder.emitDefaultConstruct(varType));

    return var;
}

IRInst* emitIndexedStoreAddressForVar(
    IRBuilder* builder,
    IRVar* localVar,
    List<IndexTrackingInfo*> defBlockIndices)
{
    IRInst* storeAddr = localVar;
    IRType* currType = as<IRPtrTypeBase>(localVar->getDataType())->getValueType();

    for (auto index : defBlockIndices)
    {
        currType = as<IRArrayType>(currType)->getElementType();

        storeAddr = builder->emitElementAddress(
            builder->getPtrType(currType),
            storeAddr, 
            index->primalCountParam);
    }

    return storeAddr;
}


IRInst* emitIndexedLoadAddressForVar(
    IRBuilder* builder,
    IRVar* localVar,
    List<IndexTrackingInfo*> defBlockIndices,
    List<IndexTrackingInfo*> useBlockIndices)
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
            auto diffCounterCurrValue = index->diffCountParam;

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
            auto primalCounterCurrValue = builder->emitLoad(index->primalCountLastVar);
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
    List<IndexTrackingInfo*> defBlockIndices)
{
    IRVar* localVar = emitLocalVarForValue(defaultVarBlock, instToStore, defBlockIndices);

    IRInst* addr = emitIndexedStoreAddressForVar(builder, localVar, defBlockIndices);

    builder->emitStore(addr, instToStore);

    return localVar;
}

IRInst* loadIndexedValue(
    IRBuilder* builder,
    IRVar* localVar,
    List<IndexTrackingInfo*> defBlockIndices,
    List<IndexTrackingInfo*> useBlockIndices)
{
    IRInst* addr = emitIndexedLoadAddressForVar(builder, localVar, defBlockIndices, useBlockIndices);

    return builder->emitLoad(addr);
}

bool areIndicesEqual(
    List<IndexTrackingInfo*> indicesA, 
    List<IndexTrackingInfo*> indicesB)
{
    if (indicesA.getCount() != indicesB.getCount())
        return false;
    
    for (Index ii = 0; ii < indicesA.getCount(); ii++)
    {
        if (indicesA[ii] != indicesB[ii])
            return false;
    }

    return true;
}

bool isDifferentialBlock(IRBlock* block)
{
    return block->findDecoration<IRDifferentialInstDecoration>();
}

RefPtr<HoistedPrimalsInfo> ensurePrimalAvailability(
    HoistedPrimalsInfo* hoistInfo,
    IRGlobalValueWithCode* func,
    Dictionary<IRBlock*, List<IndexTrackingInfo*>> indexedBlockInfo)
{
    RefPtr<IRDominatorTree> domTree = computeDominatorTree(func);

    IRBuilder builder(func->getModule());
    IRBlock* defaultVarBlock = func->getFirstBlock();

    HashSet<IRInst*> processedStoreSet;

    // TODO: Also ensure availability of everything in the recompute set.
    for (auto instToStore : hoistInfo->storeSet)
    {
        IRBlock* defBlock = nullptr;
        if (auto ptrInst = as<IRPtrTypeBase>(instToStore->getDataType()))
        {
            auto varInst = as<IRVar>(instToStore);
            auto storeUse = findUniqueStoredVal(varInst);

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
            if (userBlock->findDecoration<IRDifferentialInstDecoration>())
            {
                if (!domTree->dominates(defBlock, userBlock))
                {
                    outOfScopeUses.add(use);
                }
                else if (!areIndicesEqual(indexedBlockInfo[defBlock], indexedBlockInfo[userBlock]))
                {
                    outOfScopeUses.add(use);
                }
                else if (indexedBlockInfo[defBlock].GetValue().getCount() > 0 && 
                         !isDifferentialBlock(defBlock))
                {
                    outOfScopeUses.add(use);
                }
            }

            use = nextUse;
        }

        if (outOfScopeUses.getCount() == 0)
        {
            processedStoreSet.Add(instToStore);
            continue;
        }

        if (auto ptrInst = as<IRPtrTypeBase>(instToStore->getDataType()))
        {

            IRVar* varToStore = as<IRVar>(instToStore);
            SLANG_RELEASE_ASSERT(varToStore);
            
            auto storeUse = findUniqueStoredVal(varToStore);
            
            List<IndexTrackingInfo*> defBlockIndices = indexedBlockInfo[defBlock];

            bool isIndexedStore = (storeUse && defBlockIndices.getCount() > 0);

            if (!isIndexedStore)
            {
                varToStore->insertBefore(defaultVarBlock->getFirstOrdinaryInst());
                processedStoreSet.Add(varToStore);
                continue;
            }

            IRStore* storeInst = as<IRStore>(storeUse->getUser());

            setInsertAfterOrdinaryInst(&builder, storeInst);

            IRVar* localVar = emitLocalVarForValue(defaultVarBlock, storeInst, defBlockIndices);
            IRInst* storeAddr = emitIndexedStoreAddressForVar(&builder, localVar, defBlockIndices);

            builder.replaceOperand(&storeInst->ptr, storeAddr);

            for (auto use : outOfScopeUses)
            {
                setInsertBeforeOrdinaryInst(&builder, getInstInBlock(use->getUser()));
                
                List<IndexTrackingInfo*> useBlockIndices = indexedBlockInfo[getBlock(use->getUser())];

                IRInst* loadAddr = emitIndexedLoadAddressForVar(&builder, localVar, defBlockIndices, useBlockIndices);
                builder.replaceOperand(use, loadAddr);
            }

            processedStoreSet.Add(localVar);
        }
        else
        {  
            setInsertAfterOrdinaryInst(&builder, instToStore);

            List<IndexTrackingInfo*> defBlockIndices = indexedBlockInfo[defBlock];
            auto localVar = storeIndexedValue(&builder, defaultVarBlock, instToStore, defBlockIndices);
            
            for (auto use : outOfScopeUses)
            {
                setInsertBeforeOrdinaryInst(&builder, getInstInBlock(use->getUser()));

                List<IndexTrackingInfo*> useBlockIndices = indexedBlockInfo[getBlock(use->getUser())];
                builder.replaceOperand(use, loadIndexedValue(&builder, localVar, defBlockIndices, useBlockIndices));
            }

            processedStoreSet.Add(localVar);
        }
    }
    
    // Replace the old store set with the processed onne one.
    hoistInfo->storeSet = processedStoreSet;

    return hoistInfo;
}

void DefaultCheckpointPolicy::preparePolicy(IRGlobalValueWithCode*)
{
    // Do nothing.. This is an always-store policy.
    return;
}

HoistResult DefaultCheckpointPolicy::classify(IRUse* use)
{
    // Store all. By default, classify will only be called on relevant differential
    // uses (or on uses in a 'recompute' inst)
    // 
    return HoistResult::store(use->get());
}

/*
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
*/
};