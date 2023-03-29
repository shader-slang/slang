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
                !(as<IRModuleInst>(operand->get()->getParent())) &&
                !getBlock(operand->get())->findDecoration<IRDifferentialInstDecoration>())
                workList.add(operand);
        }

        // Is the type itself computed within our function? 
        // If so, we'll need to consider that too (this is for existential types, specialize insts, etc)
        // TODO: We might not really need to query the checkpointing algorithm for these
        // since they _have_ to be classified as 'recompute' 
        //
        if (inst->getDataType() && (getParentFunc(inst->getDataType()) == func))
        {
            if (!getBlock(inst->getDataType())->findDecoration<IRDifferentialInstDecoration>())
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

        if (!block->findDecoration<IRDifferentialInstDecoration>())
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

            if (use->getUser()->findDecoration<IRDifferentialInstDecoration>())
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
                    if (storeUse)
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

            if (use->getUser()->findDecoration<IRDifferentialInstDecoration>())
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

    bool isInstRecomputed = checkpointInfo->recomputeSet.Contains(inst);
    if (isInstRecomputed)
    {
        if (as<IRParam>(inst))
        {
            // Can completely ignore first block parameters
            if (getBlock(inst) != getBlock(inst)->getParent()->getFirstBlock())
            {    
                // TODO: We would need to clone in the control-flow for each region (without nested loops)
                // prior to this, and then hoist this parameter into the within-region block, otherwise
                // this parameter will not be visible to transposed insts.
                // This will also include adding an extra case to 'ensurePrimalAvailability': if both insts
                // are withing the _same_ indexed region, skip the indexed store/load and use a simple var.
                // 
                SLANG_UNIMPLEMENTED_X("Parameter recompute is not currently supported");
            }
        }
        else
        {
            hoistInfo->recomputeSet.Add(cloneCtx->cloneInstOutOfOrder(builder, inst));
        }
    }

    bool isInstInverted = checkpointInfo->invertSet.Contains(inst);
    if (isInstInverted)
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
    IRType* storageType,
    List<IndexTrackingInfo*> defBlockIndices)
{
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

IRVar* emitIndexedLocalVar(
    IRBlock* varBlock,
    IRType* baseType,
    List<IndexTrackingInfo*> defBlockIndices)
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
    IRVar* localVar = emitIndexedLocalVar(defaultVarBlock, instToStore->getDataType(), defBlockIndices);

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

bool areIndicesSubsetOf(
    List<IndexTrackingInfo*> indicesA, 
    List<IndexTrackingInfo*> indicesB)
{
    if (indicesA.getCount() > indicesB.getCount())
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
    IRBlock* defaultVarBlock = func->getFirstBlock()->getNextBlock();

    SLANG_ASSERT(!isDifferentialBlock(defaultVarBlock));

    HashSet<IRInst*> processedStoreSet;

    // TODO: Also ensure availability of everything in the recompute set (for proper recompute support)
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
                else if (!areIndicesSubsetOf(indexedBlockInfo[defBlock], indexedBlockInfo[userBlock]))
                {
                    outOfScopeUses.add(use);
                }
                else if (indexedBlockInfo[defBlock].GetValue().getCount() > 0 && 
                         !isDifferentialBlock(defBlock))
                {
                    outOfScopeUses.add(use);
                }
                else if (as<IRPtrTypeBase>(instToStore->getDataType()) &&
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

            // TODO: There's a slight hackiness here. (Ideally we might just want to emit
            // additional vars when splitting a call)
            //
            if (!isIndexedStore && isDerivativeContextVar(varToStore))
            {
                varToStore->insertBefore(defaultVarBlock->getFirstOrdinaryInst());
                processedStoreSet.Add(varToStore);
                continue;
            }

            setInsertAfterOrdinaryInst(&builder, getInstInBlock(storeUse->getUser()));

            IRVar* localVar = storeIndexedValue(
                &builder, 
                defaultVarBlock, 
                builder.emitLoad(varToStore),
                defBlockIndices);

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

void DefaultCheckpointPolicy::preparePolicy(IRGlobalValueWithCode* func)
{
    domTree = computeDominatorTree(func);
    return;
}

static bool doesInstHaveDiffUse(IRInst* inst)
{
    bool hasDiffUser = false;

    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        auto user = use->getUser();
        if (isDiffInst(user))
        {
            // Ignore uses that is a return or MakeDiffPair
            switch (user->getOp())
            {
            case kIROp_Return:
                continue;
            case kIROp_MakeDifferentialPair:
                if (!user->hasMoreThanOneUse() && user->firstUse &&
                    user->firstUse->getUser()->getOp() == kIROp_Return)
                    continue;
                break;
            default:
                break;
            }
            hasDiffUser = true;
            break;
        }
    }

    return hasDiffUser;
}

static bool doesInstHaveStore(IRInst* inst)
{
    SLANG_RELEASE_ASSERT(as<IRPtrTypeBase>(inst->getDataType()));

    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        if (as<IRStore>(use->getUser()))
            return true;

        if (as<IRPtrTypeBase>(use->getUser()->getDataType()))
        {
            if (doesInstHaveStore(use->getUser()))
                return true;
        }
    }

    return false;
}

static bool isIntermediateContextType(IRType* type)
{
    switch (type->getOp())
    {
    case kIROp_BackwardDiffIntermediateContextType:
        return true;
    case kIROp_PtrType:
        return isIntermediateContextType(as<IRPtrTypeBase>(type)->getValueType());
    case kIROp_ArrayType:
        return isIntermediateContextType(as<IRArrayType>(type)->getElementType());
    }

    return false;
}

static bool shouldStoreVar(IRVar* var)
{
    // Always store intermediate context var.
    if (auto typeDecor = var->findDecoration<IRBackwardDerivativePrimalContextDecoration>())
    {
        // If we are specializing a callee's intermediate context with types that can't be stored,
        // we can't store the entire context.
        if (auto spec = as<IRSpecialize>(as<IRPtrTypeBase>(var->getDataType())->getValueType()))
        {
            for (UInt i = 0; i < spec->getArgCount(); i++)
            {
                if (!canTypeBeStored(spec->getArg(i)->getDataType()))
                    return false;
            }
        }
        return true;
    }

    if (isIntermediateContextType(var->getDataType()))
    {
        return true;
    }

    // For now the store policy is simple, we use two conditions:
    // 1. Is the var used in a differential block and,
    // 2. Does the var have a store
    // 

    return (doesInstHaveDiffUse(var) && doesInstHaveStore(var) && canTypeBeStored(as<IRPtrTypeBase>(var->getDataType())->getValueType()));
}

static bool shouldStoreInst(IRInst* inst)
{
    if (!inst->getDataType())
    {
        return false;
    }

    if (!canTypeBeStored(inst->getDataType()))
        return false;

    // Never store certain opcodes.
    switch (inst->getOp())
    {
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
    case kIROp_MakeArrayFromElement:
    case kIROp_MakeDifferentialPair:
    case kIROp_MakeOptionalNone:
    case kIROp_MakeOptionalValue:
    case kIROp_DifferentialPairGetDifferential:
    case kIROp_DifferentialPairGetPrimal:
    case kIROp_ExtractExistentialValue:
    case kIROp_ExtractExistentialType:
    case kIROp_ExtractExistentialWitnessTable:
    case kIROp_undefined:
        return false;
    case kIROp_GetElement:
    case kIROp_FieldExtract:
    case kIROp_swizzle:
    case kIROp_UpdateElement:
    case kIROp_OptionalHasValue:
    case kIROp_GetOptionalValue:
    case kIROp_MatrixReshape:
    case kIROp_VectorReshape:
        // If the operand is already stored, don't store the result of these insts.
        if (inst->getOperand(0)->findDecoration<IRPrimalValueStructKeyDecoration>())
        {
            return false;
        }
        break;
    default:
        break;
    }

    // Only store if the inst has differential inst user.
    bool hasDiffUser = doesInstHaveDiffUse(inst);
    if (!hasDiffUser)
        return false;

    return true;
}

bool canRecompute(IRDominatorTree* domTree, IRUse* use)
{
    auto param = as<IRParam>(use->get());
    if (!param)
        return true;
    auto paramBlock = as<IRBlock>(param->getParent());
    for (auto predecessor : paramBlock->getPredecessors())
    {
        // If we hit this, the checkpoint policy is trying to recompute 
        // values across a loop region boundary (we don't currently support this,
        // and in general this is quite inefficient in both compute & memory)
        // 
        if (domTree->dominates(paramBlock, predecessor))
        {
            return false;
        }
    }
    return true;
}

HoistResult DefaultCheckpointPolicy::classify(IRUse* use)
{
    // Store all that we can.. by default, classify will only be called on relevant differential
    // uses (or on uses in a 'recompute' inst)
    // 
    if (auto var = as<IRVar>(use->get()))
    {
        if (shouldStoreVar(var))
            return HoistResult::store(var);
        else
            return HoistResult::recompute(var);
    }
    else
    {
        if (shouldStoreInst(use->get()))
            return HoistResult::store(use->get());
        else
        {
            // We may not be able to recompute due to limitations of
            // the unzip pass. If so we will store the result.
            if (canRecompute(domTree, use))
                return HoistResult::recompute(use->get());

            // The fallback is to store.
            return HoistResult::store(use->get());
        }
    }
}

};
