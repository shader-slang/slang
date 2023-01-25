// slang-ir-autodiff-unzip.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"

#include "slang-ir-autodiff.h"
#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-autodiff-propagate.h"
#include "slang-ir-autodiff-transcriber-base.h"
#include "slang-ir-validate.h"

namespace Slang
{

struct GenericChildrenMigrationContext
{
    IRCloneEnv cloneEnv;
    IRGeneric* srcGeneric;
    void init(IRGeneric* genericSrc, IRGeneric* genericDst)
    {
        srcGeneric = genericSrc;
        if (!genericSrc)
            return;
        auto srcParam = genericSrc->getFirstBlock()->getFirstParam();
        auto dstParam = genericDst->getFirstBlock()->getFirstParam();
        while (srcParam && dstParam)
        {
            cloneEnv.mapOldValToNew[srcParam] = dstParam;
            srcParam = srcParam->getNextParam();
            dstParam = dstParam->getNextParam();
        }
        cloneEnv.mapOldValToNew[genericSrc] = genericDst;
        cloneEnv.mapOldValToNew[genericSrc->getFirstBlock()] = genericDst->getFirstBlock();
    }

    IRInst* cloneInst(IRBuilder* builder, IRInst* src)
    {
        if (!srcGeneric)
            return src;
        if (findOuterGeneric(src) == srcGeneric)
        {
            return Slang::cloneInst(&cloneEnv, builder, src);
        }
        return src;
    }
};

struct DiffUnzipPass
{
    AutoDiffSharedContext*                  autodiffContext;

    IRCloneEnv                              cloneEnv;

    DifferentiableTypeConformanceContext    diffTypeContext;

    // Maps used to keep track of primal and
    // differential versions of split insts.
    // 
    Dictionary<IRInst*, IRInst*>            primalMap;
    Dictionary<IRInst*, IRInst*>            diffMap;

    // First diff block.
    // TODO: Can the same pass object can be used for multiple functions?
    // might run into an issue here?
    IRBlock*                                firstDiffBlock;

    struct IndexedRegion
    {
        // Parent indexed region (for nested loops)
        IndexedRegion* parent           = nullptr;

        // Intializer block for the index.
        IRBlock*       initBlock        = nullptr;

        // Index 'starts' at the first loop block (included)
        IRBlock*       firstBlock       = nullptr;

        // Index stops at the break block (not included)
        IRBlock*       breakBlock       = nullptr;

        // Block where index updates happen.
        IRBlock*       continueBlock    = nullptr;

        // After lowering, store references to the count
        // variables associated with this region
        //
        IRVar*         primalCountVar   = nullptr;
        IRVar*         diffCountVar     = nullptr;

        enum CountStatus
        {
            Unresolved,
            Dynamic,
            Static
        };

        CountStatus    status           = CountStatus::Unresolved;

        // Inferred maximum number of iterations.
        Count          maxIters         = -1;

        IndexedRegion() : 
            parent(nullptr),
            initBlock(nullptr),
            firstBlock(nullptr),
            breakBlock(nullptr),
            continueBlock(nullptr),
            primalCountVar(nullptr),
            diffCountVar(nullptr),
            status(CountStatus::Unresolved),
            maxIters(-1)
        { }

        IndexedRegion(
            IndexedRegion* parent,
            IRBlock* initBlock,
            IRBlock* firstBlock,
            IRBlock* breakBlock,
            IRBlock* continueBlock) : 
            parent(parent),
            initBlock(initBlock),
            firstBlock(firstBlock),
            breakBlock(breakBlock),
            continueBlock(continueBlock),
            primalCountVar(nullptr),
            diffCountVar(nullptr),
            status(CountStatus::Unresolved),
            maxIters(-1)
        { }
    };

    // Keep track of indexed blocks and their corresponding index heirarchy.
    Dictionary<IRBlock*, IndexedRegion*>          indexRegionMap;

    List<IndexedRegion*>                          indexRegions;


    DiffUnzipPass(
        AutoDiffSharedContext* autodiffContext)
        : autodiffContext(autodiffContext)
        , diffTypeContext(autodiffContext)
    { }

    IRInst* lookupPrimalInst(IRInst* inst)
    {
        return primalMap[inst];
    }

    IRInst* lookupDiffInst(IRInst* inst)
    {
        return diffMap[inst];
    }

    IRFunc* unzipDiffInsts(IRFunc* func)
    {
        diffTypeContext.setFunc(func);

        IRBuilder builderStorage;
        builderStorage.init(autodiffContext->sharedBuilder);
        
        IRBuilder* builder = &builderStorage;

        // Clone the entire function.
        // TODO: Maybe don't clone? The reverse-mode process seems to clone several times.
        // TODO: Looks like we get a copy of the decorations?
        IRCloneEnv subEnv;
        subEnv.parent = &cloneEnv;
        builder->setInsertBefore(func);
        IRFunc* unzippedFunc = as<IRFunc>(cloneInst(&subEnv, builder, func));

        builder->setInsertInto(unzippedFunc);

        // Functions need to have at least two blocks at this point (one for parameters,
        // and atleast one for code)
        //
        SLANG_ASSERT(unzippedFunc->getFirstBlock() != nullptr);
        SLANG_ASSERT(unzippedFunc->getFirstBlock()->getNextBlock() != nullptr);
        
        IRBlock* firstBlock = as<IRUnconditionalBranch>(unzippedFunc->getFirstBlock()->getTerminator())->getTargetBlock();

        List<IRBlock*> mixedBlocks;
        for (IRBlock* block = firstBlock; block; block = block->getNextBlock())
        {
            // Only need to unzip blocks with both differential and primal instructions.
            if (block->findDecoration<IRMixedDifferentialInstDecoration>())
            {
                mixedBlocks.add(block);
            }
        }

        IRBlock* firstPrimalBlock = nullptr;
        
        // Emit an empty primal block for every mixed block.
        for (auto block : mixedBlocks)
        {
            IRBlock* primalBlock = builder->emitBlock();
            primalMap[block] = primalBlock;

            if (block == firstBlock)
                firstPrimalBlock = primalBlock;
        }

        // Emit an empty differential block for every mixed block.
        for (auto block : mixedBlocks)
        {
            IRBlock* diffBlock = builder->emitBlock(); 
            diffMap[block] = diffBlock;

            // Mark the differential block as a differential inst 
            // (and add a reference to the primal block)
            builder->markInstAsDifferential(diffBlock, nullptr, primalMap[block]);

            // Record the first differential (code) block,
            // since we want all 'return' insts in primal blocks
            // to be replaced with a brahcn into this block.
            // 
            if (block == firstBlock)
                this->firstDiffBlock = diffBlock;
        }

        // We're going to split the terminator insts (control-flow) of each block
        // _before_ everything else. This is because some blocks can end up
        // being inside a loop, and affect the instruction splitting logic for
        // ordinary insts.
        // 
        /*{
            IRBuilder primalBuilder;
            primalBuilder.init(autodiffContext->sharedBuilder);
            
            IRBuilder diffBuilder;
            diffBuilder.init(autodiffContext->sharedBuilder);
            
            for (auto block : mixedBlocks)
            {
                auto terminator = block->getTerminator();

                primalBuilder.setInsertInto(as<IRBlock>(primalMap[block]));
                diffBuilder.setInsertInto(as<IRBlock>(diffMap[block]));

                splitMixedInst(&primalBuilder, &diffBuilder, terminator);
            }
        }*/

        // Split each block into two. 
        for (auto block : mixedBlocks)
        {
            splitBlock(block, as<IRBlock>(primalMap[block]), as<IRBlock>(diffMap[block]));
        }

        // Propagate indexed region information.
        propagateAllIndexRegions();

        // Try to infer maximum counts for all regions.
        // (only regions whose intermediates are used outside their region
        // require a maximum count, so we may see some unresolved regions
        // without any issues)
        // 
        for (auto region : indexRegions)
        {
            tryInferMaxIndex(region);
        }

        // Emit counter variables and other supporting
        // instructions for all regions.
        // 
        lowerIndexedRegions();

        // Process intermediate insts in indexed blocks
        // into array loads/stores.
        // 
        for (auto block : mixedBlocks)
        {
            auto primalBlock = primalMap[block];
            
            if (isBlockIndexed(block))
            {
                processIndexedFwdBlock(block);
            }
        }

        // Swap the first block's occurences out for the first primal block.
        firstBlock->replaceUsesWith(firstPrimalBlock);

        cleanupIndexRegionInfo();

        // Remove old blocks.
        for (auto block : mixedBlocks)
            block->removeAndDeallocate();

        return unzippedFunc;
    }

    IRBlock* getInitializerBlock(IndexedRegion* region)
    {
        return region->initBlock;
    }

    IRBlock* getUpdateBlock(IndexedRegion* region)
    {
        return region->continueBlock;
    }
    
    void tryInferMaxIndex(IndexedRegion* region)
    {
        if (region->status != IndexedRegion::CountStatus::Unresolved)
            return;
        
        // We're going to fix this at a some random number
        // for now, and then add some basic inference + user-defined decoration
        // 
        region->maxIters = 5;
        region->status = IndexedRegion::CountStatus::Static;
    }

    // Make a primal value *available* to the differential block.
    // This can get quite involved, and we're going to rely on
    // constructSSA to do most of the heavy-lifting & optimization
    // For now, we'll simply create a variable in the top-most
    // primal block, then load it in the last primal block
    // 
    //void hoistValue(IRInst* primalInst)
    //{
    //    IRBlock* terminalPrimalBlock = getTerminalPrimalBlock();
    //    IRBlock* firstPrimalBlock = getFirstPrimalBlock();
    //}

    void lowerIndexedRegions()
    {
        IRBuilder builder(autodiffContext->sharedBuilder);


        for (auto region : indexRegions)
        {

            IRBlock* initializerBlock = getInitializerBlock(region);

            // Grab first primal block.
            auto firstPrimalBlock = primalMap[region->breakBlock->getParent()->getFirstBlock()->getNextBlock()];
            
            // Make variable in the top-most block (so it's visible to diff blocks)
            builder.setInsertInto(firstPrimalBlock);
            region->primalCountVar = builder.emitVar(builder.getUIntType());

            // Make another variable in the diff block initialized to the 
            // final value of the primal counter.
            // 
            builder.setInsertInto(diffMap[initializerBlock]);
            auto primalCounterValue = builder.emitLoad(region->primalCountVar);
            region->diffCountVar = builder.emitVar(builder.getUIntType());
            builder.emitStore(region->diffCountVar, primalCounterValue);
            
            IRBlock* updateBlock = getUpdateBlock(region);
            
            {
                // TODO: Figure out if the counter update needs to go before or after
                // the rest of the update block.
                // 
                builder.setInsertBefore(as<IRBlock>(primalMap[updateBlock])->getTerminator());

                auto counterVal = builder.emitLoad(region->primalCountVar);
                auto incCounterVal = builder.emitAdd(
                    builder.getUIntType(), 
                    counterVal,
                    builder.getIntValue(builder.getUIntType(), 1));

                auto incStore = builder.emitStore(region->primalCountVar, incCounterVal);

                builder.addLoopCounterDecoration(counterVal);
                builder.addLoopCounterDecoration(incCounterVal);
                builder.addLoopCounterDecoration(incStore);
            }

            {
                // NOTE: This is a hacky shortcut we're taking here.
                // Technically the unzip pass should not affect the
                // correctness (it must still compute the proper fwd-mode derivative)
                // However, we're currently making the loop counter go backwards to
                // make it easier on the transposition pass, so the output from
                // the unzip pass is neither fwd-mode or rev-mode until the transposition
                // step is complete.
                // 
                // TODO: Ideally this needs to be replaced with a small inversion step
                // within the transposition pass.
                //

                builder.setInsertBefore(as<IRBlock>(diffMap[updateBlock])->getTerminator());

                auto counterVal = builder.emitLoad(region->diffCountVar);
                auto decCounterVal = builder.emitSub(
                    builder.getUIntType(), 
                    counterVal,
                    builder.getIntValue(builder.getUIntType(), 0));

                auto decStore = builder.emitStore(region->diffCountVar, decCounterVal);

                // Mark insts as loop counter insts to avoid removing them.
                //
                builder.addLoopCounterDecoration(counterVal);
                builder.addLoopCounterDecoration(decCounterVal);
                builder.addLoopCounterDecoration(decStore);
            }

        }
    }

    void processIndexedFwdBlock(IRBlock* fwdBlock)
    {
        if (!isBlockIndexed(fwdBlock))
            return;
        
        // Grab first primal block.
        IRBlock* firstPrimalBlock = as<IRBlock>(primalMap[fwdBlock->getParent()->getFirstBlock()->getNextBlock()]);
        
        // Scan through instructions and identify those that are used
        // outside the local block.
        //
        IRBlock* primalBlock = as<IRBlock>(primalMap[fwdBlock]);

        List<IRInst*> primalInsts;
        for (auto child = primalBlock->getFirstChild(); child; child = child->getNextInst())
            primalInsts.add(child);

        IRBuilder builder(autodiffContext->sharedBuilder);

        // Build list of indices that this block is affected by.
        List<IndexedRegion*> regions;
        {
            IndexedRegion* region = indexRegionMap[fwdBlock];
            for (; region; region = region->parent)
                regions.add(region);
        }
        
        for (auto inst : primalInsts)
        {
            // 1. Check if we need to store inst (is it used in a differential block?)

            bool shouldStore = false;
            for (auto use = inst->firstUse; use; use = use->nextUse)
            {
                IRBlock* useBlock = as<IRBlock>(use->getUser()->getParent());

                if (isDifferentialInst(useBlock))
                {
                    shouldStore = true;
                }
            }

            if (!shouldStore) continue;

            // 2. Emit an array to top-level to allocate space.
            
            builder.setInsertBefore(firstPrimalBlock->getTerminator());

            IRType* arrayType = inst->getDataType();
            SLANG_ASSERT(!as<IRPtrTypeBase>(arrayType)); // can't store pointers.

            for (auto region : regions)
            {
                SLANG_ASSERT(region->status == IndexedRegion::CountStatus::Static);
                SLANG_ASSERT(region->maxIters >= 0);

                arrayType = builder.getArrayType(
                    arrayType,
                    builder.getIntValue(
                        builder.getUIntType(),
                        region->maxIters));
            }

            // Reverse the list since the indices needs to be 
            // emitted in reverse order.
            // 
            regions.reverse();
            
            auto storageVar = builder.emitVar(arrayType);

            // 3. Store current value into the array and replace uses with a load.
            {
                builder.setInsertAfter(inst);
                
                IRInst* storeAddr = storageVar;
                IRType* currType = storageVar->getDataType();

                for (auto region : regions)
                {
                    currType = as<IRArrayType>(currType)->getElementType();

                    storeAddr = builder.emitElementAddress(
                        currType,
                        storeAddr, 
                        region->primalCountVar);
                }

                builder.emitStore(storeAddr, inst);
            }

            // 4. Replace uses in differential blocks with loads from the array.
            {
                for (auto use = inst->firstUse; use; use = use->nextUse)
                {
                    IRBlock* useBlock = as<IRBlock>(use->getUser()->getParent());

                    if (isDifferentialInst(useBlock))
                    {
                        builder.setInsertBefore(use->getUser());

                        IRInst* loadAddr = storageVar;
                        IRType* currType = storageVar->getDataType();

                        for (auto region : regions)
                        {
                            currType = as<IRArrayType>(currType)->getElementType();

                            loadAddr = builder.emitElementAddress(
                                currType,
                                loadAddr, 
                                region->diffCountVar);
                        }

                        use->set(builder.emitLoad(loadAddr));
                    }
                }
            }
        }
    }

    IRFunc* extractPrimalFunc(IRFunc* func, IRFunc* originalFunc, IRInst*& intermediateType);

    bool isRelevantDifferentialPair(IRType* type)
    {
        if (as<IRDifferentialPairType>(type))
        {
            return true;
        }
        else if (auto argPtrType = as<IRPtrTypeBase>(type))
        {
            if (as<IRDifferentialPairType>(argPtrType->getValueType()))
            {
                return true;
            }
        }

        return false;
    }

    static IRInst* _getOriginalFunc(IRInst* call)
    {
        if (auto decor = call->findDecoration<IRAutoDiffOriginalValueDecoration>())
            return decor->getOriginalValue();
        return nullptr;
    }

    InstPair splitCall(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRCall* mixedCall)
    {
        IRBuilder globalBuilder;
        globalBuilder.init(autodiffContext->sharedBuilder);

        auto fwdCalleeType = mixedCall->getCallee()->getDataType();
        auto baseFn = _getOriginalFunc(mixedCall);
        SLANG_RELEASE_ASSERT(baseFn);

        auto primalFuncType = autodiffContext->transcriberSet.primalTranscriber->differentiateFunctionType(
            primalBuilder, baseFn, as<IRFuncType>(baseFn->getDataType()));

        IRInst* intermediateType = nullptr;

        if (auto specialize = as<IRSpecialize>(baseFn))
        {
            auto func = findSpecializeReturnVal(specialize);
            auto outerGen = findOuterGeneric(func);
            intermediateType = primalBuilder->getBackwardDiffIntermediateContextType(outerGen);
            intermediateType = specializeWithGeneric(
                *primalBuilder,
                intermediateType,
                as<IRGeneric>(findOuterGeneric(primalBuilder->getInsertLoc().getParent())));
        }
        else
        {
            intermediateType = primalBuilder->getBackwardDiffIntermediateContextType(baseFn);
        }

        auto intermediateVar = primalBuilder->emitVar((IRType*)intermediateType);
        primalBuilder->addBackwardDerivativePrimalContextDecoration(intermediateVar, intermediateVar);

        auto primalFn = primalBuilder->emitBackwardDifferentiatePrimalInst(primalFuncType, baseFn);

        List<IRInst*> primalArgs;
        for (UIndex ii = 0; ii < mixedCall->getArgCount(); ii++)
        {
            auto arg = mixedCall->getArg(ii);

            if (isRelevantDifferentialPair(arg->getDataType()))
            {
                primalArgs.add(lookupPrimalInst(arg));
            }
            else
            {
                primalArgs.add(arg);
            }
        }
        primalArgs.add(intermediateVar);

        auto mixedDecoration = mixedCall->findDecoration<IRMixedDifferentialInstDecoration>();
        SLANG_ASSERT(mixedDecoration);

        auto fwdPairResultType = as<IRDifferentialPairType>(mixedDecoration->getPairType());
        SLANG_ASSERT(fwdPairResultType);

        auto primalType = fwdPairResultType->getValueType();
        auto diffType = (IRType*) diffTypeContext.getDifferentialForType(&globalBuilder, primalType);

        auto primalVal = primalBuilder->emitCallInst(primalType, primalFn, primalArgs);
        primalBuilder->addBackwardDerivativePrimalContextDecoration(primalVal, intermediateVar);

        List<IRInst*> diffArgs;
        for (UIndex ii = 0; ii < mixedCall->getArgCount(); ii++)
        {
            auto arg = mixedCall->getArg(ii);

            if (isRelevantDifferentialPair(arg->getDataType()))
            {
                auto primalArg = lookupPrimalInst(arg);
                auto diffArg = lookupDiffInst(arg);

                // If arg is a mixed differential (pair), it should have already been split.
                SLANG_ASSERT(primalArg);
                SLANG_ASSERT(diffArg);

                auto pairArg = diffBuilder->emitMakeDifferentialPair(
                        arg->getDataType(),
                        primalArg,
                        diffArg);

                diffBuilder->markInstAsDifferential(pairArg, primalArg->getDataType());
                diffArgs.add(pairArg);
            }
            else
            {
                diffArgs.add(arg);
            }
        }
        
        auto newFwdCallee = diffBuilder->emitForwardDifferentiateInst(fwdCalleeType, baseFn);

        diffBuilder->markInstAsDifferential(newFwdCallee);

        auto diffPairVal = diffBuilder->emitCallInst(
            fwdPairResultType, 
            newFwdCallee,
            diffArgs);
        diffBuilder->markInstAsDifferential(diffPairVal, primalType);

        disableIRValidationAtInsert();
        diffBuilder->addBackwardDerivativePrimalContextDecoration(diffPairVal, intermediateVar);
        enableIRValidationAtInsert();

        auto diffVal = diffBuilder->emitDifferentialPairGetDifferential(diffType, diffPairVal);
        diffBuilder->markInstAsDifferential(diffVal, primalType);

        return InstPair(primalVal, diffVal);
    }

    InstPair splitMakePair(IRBuilder*, IRBuilder*, IRMakeDifferentialPair* mixedPair)
    {
        return InstPair(mixedPair->getPrimalValue(), mixedPair->getDifferentialValue());
    }

    InstPair splitLoad(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRLoad* mixedLoad)
    {
        // By the nature of how diff pairs are used, and the fact that FieldAddress/GetElementPtr,
        // etc, cannot appear before a GetDifferential/GetPrimal, a mixed load can only be from a
        // parameter or a variable.
        // 
        if (as<IRParam>(mixedLoad->getPtr()))
        {
            // Should not occur with current impl of fwd-mode.
            // If impl. changes, impl this case too.
            // 
            SLANG_UNIMPLEMENTED_X("Splitting a load from a param is not currently implemented.");
        }

        // Everything else should have already been split.
        auto primalPtr = lookupPrimalInst(mixedLoad->getPtr());
        auto diffPtr = lookupDiffInst(mixedLoad->getPtr());

        return InstPair(primalBuilder->emitLoad(primalPtr), diffBuilder->emitLoad(diffPtr));
    }

    InstPair splitVar(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRVar* mixedVar)
    {
        auto pairType = as<IRDifferentialPairType>(mixedVar->getDataType());
        auto primalType = pairType->getValueType();
        auto diffType = (IRType*) diffTypeContext.getDifferentialForType(primalBuilder, primalType);
        
        return InstPair(primalBuilder->emitVar(primalType), diffBuilder->emitVar(diffType));
    }

    InstPair splitReturn(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRReturn* mixedReturn)
    {
        auto pairType = as<IRDifferentialPairType>(mixedReturn->getVal()->getDataType());
        auto primalType = pairType->getValueType();

        // Check that we have an unambiguous 'first' differential block.
        SLANG_ASSERT(firstDiffBlock);
        auto primalBranch = primalBuilder->emitBranch(firstDiffBlock);
        auto pairVal = diffBuilder->emitMakeDifferentialPair(
            pairType,
            lookupPrimalInst(mixedReturn->getVal()),
            lookupDiffInst(mixedReturn->getVal()));
        diffBuilder->markInstAsDifferential(pairVal, primalType);

        auto returnInst = diffBuilder->emitReturn(pairVal);
        diffBuilder->markInstAsDifferential(returnInst, primalType);

        return InstPair(primalBranch, returnInst);
    }

    bool isBlockIndexed(IRBlock* block)
    {
        return indexRegionMap.ContainsKey(block) && indexRegionMap[block] != nullptr;
    }

    void addNewIndex(IRLoop* targetLoop)
    {
        // Create indexed region without a parent for now. 
        // The parent will be filled in during propagation.
        // 
        IndexedRegion* region = new IndexedRegion(
            nullptr,
            as<IRBlock>(targetLoop->getParent()),
            targetLoop->getTargetBlock(),
            targetLoop->getBreakBlock(),
            targetLoop->getContinueBlock());
        
        indexRegionMap[targetLoop->getTargetBlock()] = region;
        indexRegions.add(region);
    }

    // Deallocate regions
    void cleanupIndexRegionInfo()
    {
        for (auto region : indexRegions)
        {
            delete region;
        }

        indexRegions.clear();
        indexRegionMap.Clear();
    }

    void propagateAllIndexRegions()
    {


        // Load up the starting block of every region into
        // initial worklist.
        // 
        List<IRBlock*> workList;
        HashSet<IRBlock*> workSet;
        for (auto region : indexRegions)
        {
            workList.add(region->firstBlock);
            workSet.Add(region->firstBlock);
        }

        // Keep propagating from initial work list to predecessors
        // Add blocks to work list if their region assignment has changed
        // Add the beginning blocks for complete regions if region parent has changed.
        // 
        while (workList.getCount() > 0)
        {
            auto block = workList.getLast();
            workList.removeLast();
            workSet.Remove(block);
            
            HashSet<IRBlock*> successors;

            for (auto successor : block->getSuccessors())
            {
                if (successors.Contains(successor))
                    continue;
                
                if (propagateIndexRegion(block, successor))
                {
                    if (!workSet.Contains(successor))
                    {
                        workList.add(successor);
                        workSet.Add(successor);
                    }

                    // Do we have an index region for the successor, which is
                    // also the starting block of that region?
                    // Then the change might have been the addition of 
                    // a parent node. Add the break block so the
                    // change can be propagated further.
                    // 
                    if (isBlockIndexed(successor))
                    {
                        IndexedRegion* succRegion = indexRegionMap[successor];
                        if (succRegion->firstBlock == successor)
                        {
                            if (!workSet.Contains(succRegion->breakBlock))
                            {
                                workList.add(succRegion->breakBlock);
                                workSet.Add(succRegion->breakBlock);
                            }
                        }
                    }
                }

                successors.Add(successor);
            }
        }
    }

    bool setIndexRegion(IRBlock* block, IndexedRegion* region)
    {
        if (!region) return false;

        if (indexRegionMap.ContainsKey(block)
            && indexRegionMap[block] == region)
            return false;

        indexRegionMap[block] = region;
        return true;
    }

    bool propagateIndexRegion(IRBlock* srcBlock, IRBlock* nextBlock)
    {
        // Is the current region indexed? 
        // If not, there's nothing to propagate
        //
        if (!isBlockIndexed(srcBlock))
            return false;
        
        IndexedRegion* region = indexRegionMap[srcBlock];
        
        // If the target's index is already resolved, 
        // check if it's a sub-region.
        // 
        if (isBlockIndexed(nextBlock))
        {
            IndexedRegion* nextRegion = indexRegionMap[nextBlock];

            // If we're at the first block of a region, 
            // set current region as continue-region's
            // parent.
            //
            if (nextBlock == nextRegion->firstBlock && nextRegion != region)
            {
                nextRegion->parent = region;
                return true;
            }

            return false;
        }

        // If we're at the break block, move up to the parent index.
        if (nextBlock == region->breakBlock)
            return setIndexRegion(nextBlock, region->parent);

        // If none of the special cases hit, copy the 
        // current region to the next block.
        // 
        return setIndexRegion(nextBlock, region);
    }

    // Splitting a loop is one of the trickiest parts of the unzip pass.
    // Thus far, we've been dealing with blocks that are only run once, so we 
    // could arbitrarily move intermediate instructions to other blocks since they are
    // generated and consumed at-most one time.
    // 
    // Intermediate instructions in a loop can take on a different value each iteration
    // and thus need to be stored explicitly to an array.
    // 
    // We also need to ascertain an upper limit on the iteration count. 
    // With very few exceptions, this is a fundamental requirement.
    // 
    InstPair splitLoop(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRLoop* mixedLoop)
    {
        
        auto breakBlock = mixedLoop->getBreakBlock();
        auto continueBlock = mixedLoop->getContinueBlock();
        auto nextBlock = mixedLoop->getTargetBlock();

        // Push a new index.
        addNewIndex(mixedLoop);

        return InstPair(
            primalBuilder->emitLoop(
                as<IRBlock>(primalMap[nextBlock]),
                as<IRBlock>(primalMap[breakBlock]),
                as<IRBlock>(primalMap[continueBlock])),
            diffBuilder->emitLoop(
                as<IRBlock>(diffMap[nextBlock]),
                as<IRBlock>(diffMap[breakBlock]),
                as<IRBlock>(diffMap[continueBlock])));
    }

    InstPair splitControlFlow(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRInst* branchInst)
    {
        switch (branchInst->getOp())
        {
        case kIROp_unconditionalBranch:
            {
                auto uncondBranchInst = as<IRUnconditionalBranch>(branchInst);
                auto targetBlock = uncondBranchInst->getTargetBlock();
                
                // Split args.
                List<IRInst*> primalArgs;
                List<IRInst*> diffArgs;
                for (UIndex ii = 0; ii < uncondBranchInst->getArgCount(); ii++)
                {
                    if (isDifferentialInst(uncondBranchInst->getArg(ii)))
                        diffArgs.add(uncondBranchInst->getArg(ii));
                    else
                        primalArgs.add(uncondBranchInst->getArg(ii));
                }
                
                return InstPair(
                    primalBuilder->emitBranch(
                        as<IRBlock>(primalMap[targetBlock]),
                        primalArgs.getCount(),
                        primalArgs.getBuffer()),
                    diffBuilder->emitBranch(
                        as<IRBlock>(diffMap[targetBlock]),
                        diffArgs.getCount(),
                        diffArgs.getBuffer()));

            }
        
        case kIROp_conditionalBranch:
            {
                auto trueBlock = as<IRConditionalBranch>(branchInst)->getTrueBlock();
                auto falseBlock = as<IRConditionalBranch>(branchInst)->getFalseBlock();
                auto condInst = as<IRConditionalBranch>(branchInst)->getCondition();

                return InstPair(
                    primalBuilder->emitBranch(
                        condInst,
                        as<IRBlock>(primalMap[trueBlock]),
                        as<IRBlock>(primalMap[falseBlock])),
                    diffBuilder->emitBranch(
                        condInst,
                        as<IRBlock>(diffMap[trueBlock]),
                        as<IRBlock>(diffMap[falseBlock])));
            }
        
        case kIROp_ifElse:
            {
                auto trueBlock = as<IRIfElse>(branchInst)->getTrueBlock();
                auto falseBlock = as<IRIfElse>(branchInst)->getFalseBlock();
                auto afterBlock = as<IRIfElse>(branchInst)->getAfterBlock();
                auto condInst = as<IRIfElse>(branchInst)->getCondition();

                return InstPair(
                    primalBuilder->emitIfElse(
                        condInst,
                        as<IRBlock>(primalMap[trueBlock]),
                        as<IRBlock>(primalMap[falseBlock]),
                        as<IRBlock>(primalMap[afterBlock])),
                    diffBuilder->emitIfElse(
                        condInst,
                        as<IRBlock>(diffMap[trueBlock]),
                        as<IRBlock>(diffMap[falseBlock]),
                        as<IRBlock>(diffMap[afterBlock])));
            }
        
        case kIROp_Switch:
            {
                auto switchInst = as<IRSwitch>(branchInst);
                auto breakBlock = switchInst->getBreakLabel();
                auto defaultBlock = switchInst->getDefaultLabel();
                auto condInst = switchInst->getCondition();

                List<IRInst*> primalCaseArgs;
                List<IRInst*> diffCaseArgs;

                for (UIndex ii = 0; ii < switchInst->getCaseCount(); ii ++)
                {
                    primalCaseArgs.add(switchInst->getCaseValue(ii));
                    diffCaseArgs.add(switchInst->getCaseValue(ii));

                    primalCaseArgs.add(primalMap[switchInst->getCaseLabel(ii)]);
                    diffCaseArgs.add(diffMap[switchInst->getCaseLabel(ii)]);
                }

                return InstPair(
                    primalBuilder->emitSwitch(
                        condInst,
                        as<IRBlock>(primalMap[breakBlock]),
                        as<IRBlock>(primalMap[defaultBlock]),
                        primalCaseArgs.getCount(),
                        primalCaseArgs.getBuffer()),
                    diffBuilder->emitSwitch(
                        condInst,
                        as<IRBlock>(diffMap[breakBlock]),
                        as<IRBlock>(diffMap[defaultBlock]),
                        diffCaseArgs.getCount(),
                        diffCaseArgs.getBuffer()));
            }
        
        case kIROp_loop:
            return splitLoop(primalBuilder, diffBuilder, as<IRLoop>(branchInst));
        
        default:
            SLANG_UNEXPECTED("Unhandled instruction");
        }
    }
    
    InstPair _splitMixedInst(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_Call:
            return splitCall(primalBuilder, diffBuilder, as<IRCall>(inst));

        case kIROp_Var:
            return splitVar(primalBuilder, diffBuilder, as<IRVar>(inst));

        case kIROp_MakeDifferentialPair:
            return splitMakePair(primalBuilder, diffBuilder, as<IRMakeDifferentialPair>(inst));
 
        case kIROp_Load:
            return splitLoad(primalBuilder, diffBuilder, as<IRLoad>(inst));
        
        case kIROp_Return:
            return splitReturn(primalBuilder, diffBuilder, as<IRReturn>(inst));

        case kIROp_unconditionalBranch:
        case kIROp_conditionalBranch:
        case kIROp_ifElse:
        case kIROp_Switch:
        case kIROp_loop:
            return splitControlFlow(primalBuilder, diffBuilder, inst);
        
        case kIROp_Unreachable:
            return InstPair(primalBuilder->emitUnreachable(),
                diffBuilder->emitUnreachable());

        default:
            SLANG_ASSERT_FAILURE("Unhandled mixed diff inst");
        }
    }

    void splitMixedInst(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRInst* inst)
    {
        auto instPair = _splitMixedInst(primalBuilder, diffBuilder, inst);

        primalMap[inst] = instPair.primal;
        diffMap[inst] = instPair.differential;
    }

    void splitBlock(IRBlock* block, IRBlock* primalBlock, IRBlock* diffBlock)
    {
        // Make two builders for primal and differential blocks.
        IRBuilder primalBuilder;
        primalBuilder.init(autodiffContext->sharedBuilder);
        primalBuilder.setInsertInto(primalBlock);

        IRBuilder diffBuilder;
        diffBuilder.init(autodiffContext->sharedBuilder);
        diffBuilder.setInsertInto(diffBlock);

        List<IRInst*> splitInsts;
        for (auto child = block->getFirstChild(); child;)
        {
            IRInst* nextChild = child->getNextInst();

            if (auto getDiffInst = as<IRDifferentialPairGetDifferential>(child))
            {
                // Replace GetDiff(A) with A.d
                if (diffMap.ContainsKey(getDiffInst->getBase()))
                {
                    getDiffInst->replaceUsesWith(lookupDiffInst(getDiffInst->getBase()));
                    getDiffInst->removeAndDeallocate();
                    child = nextChild;
                    continue;
                }
            }
            else if (auto getPrimalInst = as<IRDifferentialPairGetPrimal>(child))
            {
                // Replace GetPrimal(A) with A.p
                if (primalMap.ContainsKey(getPrimalInst->getBase()))
                {
                    getPrimalInst->replaceUsesWith(lookupPrimalInst(getPrimalInst->getBase()));
                    getPrimalInst->removeAndDeallocate();
                    child = nextChild;
                    continue;
                }
            }

            if (isDifferentialInst(child))
            {
                child->insertAtEnd(diffBlock);
            }
            else if (isMixedDifferentialInst(child))
            {
                splitMixedInst(&primalBuilder, &diffBuilder, child);
                splitInsts.add(child);
            }
            else
            {
                child->insertAtEnd(primalBlock);
            }

            child = nextChild;
        }

        // Remove insts that were split.
        for (auto inst : splitInsts)
        {
            // Consistency check.
            for (auto use = inst->firstUse; use; use = use->nextUse)
            {
                SLANG_RELEASE_ASSERT((use->getUser()->getParent() != primalBlock) && 
                    (use->getUser()->getParent() != diffBlock));
            }

            // Leave terminator in to keep CFG info.
            if (!as<IRTerminatorInst>(inst))
                inst->removeAndDeallocate();
        }

        // Nothing should be left in the original block.
        SLANG_ASSERT(block->getFirstChild() == block->getTerminator());

        // Branch from primal to differential block.
        // Functionally, the new blocks should produce the same output as the
        // old block.
        // primalBuilder.emitBranch(diffBlock);
    }
};

}
