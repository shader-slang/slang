#include "slang-ir-autodiff-primal-hoist.h"
#include "slang-ir-autodiff-region.h"

namespace Slang 
{

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

IRUse* findUniqueStoredVal(IRVar* var)
{
    if (as<IRBackwardDerivativeIntermediateTypeDecoration>(var->getDataType()))
    {
        IRUse* primalCallUse = nullptr;
        for (auto use = var->firstUse; use; use = use->nextUse)
        {
            if (auto callInst = as<IRCall>(use->getUser()))
            {
                // Should not see more than one IRCall. If we do
                // we'll need to pick the primal call.
                // 
                SLANG_RELEASE_ASSERT(!primalCallUse);
                primalCallUse = use;
            }
        }
        return primalCallUse;
    }
    else
    {
        IRUse* storeUse = nullptr;
        for (auto use = var->firstUse; use; use = use->nextUse)
        {
            if (auto storeInst = as<IRStore>(use->getUser()))
            {
                // Should not see more than one IRStore
                SLANG_RELEASE_ASSERT(!storeUse);
                storeUse = use;
            }
        }
        return storeUse;
    }
}



RefPtr<CheckpointSetInfo> AutodiffCheckpointPolicyBase::processFunc(IRGlobalValueWithCode* func, BlockSplitInfo* splitInfo)
{
    RefPtr<CheckpointSetInfo> checkpointInfo = new CheckpointSetInfo();

    RefPtr<IRDominatorTree> domTree = computeDominatorTree(func);

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
        checkpointInfo->hoistModeMap[use] = result;

        if (result.mode == HoistResult::Mode::Store)
        {
            SLANG_ASSERT(!checkpointInfo->recomputeSet.Contains(result.instToStore));
            checkpointInfo->storeSet.Add(result.instToStore);
        }
        else if (result.mode == HoistResult::Mode::Recompute)
        {
            SLANG_ASSERT(!checkpointInfo->storeSet.Contains(result.instToRecompute));
            checkpointInfo->recomputeSet.Add(result.instToRecompute);

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
            SLANG_ASSERT(containsOperand(result.inversionInfo.instToInvert, use->getUser()));
            checkpointInfo->invertSet.Add(result.inversionInfo.instToInvert);
        }
    }

}

RefPtr<HoistedPrimalsInfo> applyCheckpointSet(
    CheckpointSetInfo* checkpointInfo,
    IRGlobalValueWithCode* func,
    BlockSplitInfo* splitInfo)
{
    RefPtr<HoistedPrimalsInfo> hoistInfo = new HoistedPrimalsInfo();

    RefPtr<IROutOfOrderCloneContext> cloneCtx = new IROutOfOrderCloneContext();

    // Go back over the insts and move/clone them accoridngly.
    for (auto block : func->getBlocks())
    {
        // Skip parameter block.
        if (block == func->getFirstBlock())
            continue;

        if (!block->findDecoration<IRDifferentialInstDecoration>())
            continue;
        
        auto firstDiffInst = as<IRBlock>(splitInfo->diffBlockMap[block])->getFirstOrdinaryInst();

        auto firstParam = block->getFirstParam();

        IRBuilder builder(func->getModule());

        for (auto child : block->getChildren())
        {
            if (checkpointInfo->recomputeSet.Contains(child))
            {
                if (!as<IRParam>(child))
                {
                    builder.setInsertBefore(firstDiffInst);
                    hoistInfo->recomputeSet.Add(cloneCtx->cloneInstOutOfOrder(&builder, child));
                }
                else
                {
                    builder.setInsertBefore(firstParam);
                    hoistInfo->recomputeSet.Add(cloneCtx->cloneInstOutOfOrder(&builder, child));
                }
            }
            else if (checkpointInfo->storeSet.Contains(child))
            {
                hoistInfo->storeSet.Add(cloneCtx->cloneInstOutOfOrder(&builder, child));
            }
            else if (checkpointInfo->invertSet.Contains(child))
            {
                SLANG_UNIMPLEMENTED_X("Inverted insts not currently handled");
            }
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
    varBuilder.emitStore(var, varBuilder.emitDefaultConstruct(instToStore->getDataType()));

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

RefPtr<HoistedPrimalsInfo> ensurePrimalAvailability(
    HoistedPrimalsInfo* hoistInfo,
    IRGlobalValueWithCode* func,
    Dictionary<IRBlock*, List<IndexTrackingInfo*>> indexedBlockInfo)
{
    RefPtr<IRDominatorTree> domTree = computeDominatorTree(func);

    IRBuilder builder(func->getModule());
    IRBlock* defaultVarBlock = func->getFirstBlock();

    HashSet<IRInst*> processedStoreSet;

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
            
            IRBlock* userBlock = getBlock(use->getUser());
            if (!domTree->dominates(defBlock, userBlock) ||
                !areIndicesEqual(indexedBlockInfo[defBlock], indexedBlockInfo[userBlock]))
            {
                outOfScopeUses.add(use);
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

};