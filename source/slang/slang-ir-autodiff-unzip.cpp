#include "slang-ir-autodiff-unzip.h"

#include "slang-ir-autodiff-rev.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-util.h"

namespace Slang
{

struct UnzippingContext
{
    AutoDiffSharedContext* autodiffContext;

    DifferentiableTypeConformanceContext diffTypeContext;

    // Maps used to keep track of primal and
    // differential versions of split insts.
    //
    Dictionary<IRInst*, IRInst*> primalMap;
    Dictionary<IRInst*, IRInst*> diffMap;
    Dictionary<IRBlock*, IRBlock*> recomputeBlockMap;

    IRBlock* firstDiffBlock;

    UnzippingContext(AutoDiffSharedContext* autodiffContext)
        : autodiffContext(autodiffContext), diffTypeContext(autodiffContext)
    {
    }

    IRInst* lookupPrimalInst(IRInst* inst) { return primalMap[inst]; }

    IRInst* lookupDiffInst(IRInst* inst) { return diffMap[inst]; }

    void unzipFunc(IRFunc* func)
    {
        // Build a map of blocks to loop regions.
        // This will be used later to insert tracking indices
        //
        RefPtr<IndexedRegionMap> indexRegionMap = buildIndexedRegionMap(func);

        IRBuilder builderStorage(autodiffContext->moduleInst->getModule());

        IRBuilder* builder = &builderStorage;

        IRFunc* unzippedFunc = func;

        // Initialize the primal/diff map for parameters.
        //
        // Generate distinct references for parameters that should be split.
        // We don't actually modify the parameter list here, instead we emit
        // PrimalParamRef(param) and DiffParamRef(param) and use those to represent
        // a use from the primal or diff part of the program.
        //
        builder->setInsertBefore(unzippedFunc->getFirstBlock()->getTerminator());

        // Functions need to have at least two blocks at this point (one for parameters,
        // and atleast one for code)
        //
        SLANG_ASSERT(unzippedFunc->getFirstBlock() != nullptr);
        SLANG_ASSERT(unzippedFunc->getFirstBlock()->getNextBlock() != nullptr);

        IRBlock* firstBlock = unzippedFunc->getFirstBlock();

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
            builder->markInstAsDifferential(
                diffBlock,
                builder->getBasicBlockType(),
                primalMap[block]);

            // Record the first differential (code) block,
            // since we want all 'return' insts in primal blocks
            // to be replaced with a brahcn into this block.
            //
            if (block == firstBlock)
                firstDiffBlock = diffBlock;
        }

        // Split each block into two.
        for (auto block : mixedBlocks)
        {
            // Special case the parameter block.
            if (block == firstBlock)
            {
                splitParameterBlock(
                    block,
                    as<IRBlock>(primalMap[block]),
                    as<IRBlock>(diffMap[block]));
                continue;
            }

            splitBlock(block, as<IRBlock>(primalMap[block]), as<IRBlock>(diffMap[block]));
        }

        // Copy regions from fwd-block to their split blocks
        // to make it easier to do lookups.
        //
        {
            List<IRBlock*> workList;
            for (auto [block, _] : indexRegionMap->map)
                workList.add(block);

            for (auto block : workList)
            {
                if (primalMap.containsKey(block))
                    indexRegionMap->map[as<IRBlock>(primalMap[block])] =
                        (IndexedRegion*)indexRegionMap->map[block];

                if (diffMap.containsKey(block))
                    indexRegionMap->map.set(
                        as<IRBlock>(diffMap[block]),
                        (IndexedRegion*)indexRegionMap->map[block]);
            }
        }

        // Swap the first block's occurences out for the first primal block.
        // firstBlock->replaceUsesWith(firstPrimalBlock);

        RefPtr<BlockSplitInfo> splitInfo = new BlockSplitInfo();

        for (auto block : mixedBlocks)
            if (primalMap.containsKey(block))
                splitInfo->diffBlockMap[as<IRBlock>(primalMap[block])] =
                    as<IRBlock>(diffMap[block]);

        for (auto block : mixedBlocks)
            block->removeAndDeallocate();
    }

    static IRInst* _getOriginalFunc(IRInst* call)
    {
        if (auto decor = call->findDecoration<IRAutoDiffOriginalValueDecoration>())
            return decor->getOriginalValue();
        return nullptr;
    }

    IRInst* getIntermediateType(IRBuilder* builder, IRInst* baseFn)
    {
        if (as<IRLookupWitnessMethod>(baseFn))
        {
            return builder->getVoidType();
        }
        else if (auto specialize = as<IRSpecialize>(baseFn))
        {
            if (as<IRLookupWitnessMethod>(specialize->getBase()))
                return builder->getVoidType();

            auto func = findSpecializeReturnVal(specialize);
            if (as<IRLookupWitnessMethod>(func))
            {
                // An interface method won't have intermediate type.
                return builder->getVoidType();
            }
            else
            {
                auto outerGen = findOuterGeneric(func);
                auto innerIntermediateType =
                    builder->getBackwardDiffIntermediateContextType(outerGen);

                List<IRInst*> args;
                for (UInt i = 0; i < specialize->getArgCount(); i++)
                    args.add(specialize->getArg(i));

                return builder->emitSpecializeInst(
                    builder->getTypeKind(),
                    innerIntermediateType,
                    args.getCount(),
                    args.getBuffer());
            }
        }
        else
        {
            return builder->getBackwardDiffIntermediateContextType(baseFn);
        }
    }

    InstPair splitCall(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRCall* mixedCall)
    {
        IRBuilder globalBuilder(autodiffContext->moduleInst->getModule());

        auto baseFn = _getOriginalFunc(mixedCall);
        SLANG_RELEASE_ASSERT(baseFn);

        auto applyBwdFunc = diffTypeContext.tryGetAssociationOfKind(
            baseFn,
            ValAssociationKind::BackwardDerivativeApply);
        auto applyBwdFuncType = cast<IRFuncType>(
            diffTypeContext.resolveType(&globalBuilder, applyBwdFunc->getDataType()));

        List<IRInst*> applyFuncArgs;
        for (UIndex ii = 0; ii < mixedCall->getArgCount(); ii++)
        {
            auto arg = mixedCall->getArg(ii);
            if (isMixedDifferentialInst(arg))
                applyFuncArgs.add(lookupPrimalInst(arg));
            else
                applyFuncArgs.add(arg);
        }

        SLANG_ASSERT(applyFuncArgs.getCount() == (Index)applyBwdFuncType->getParamCount());
        auto contextVal = primalBuilder->emitCallInst(
            applyBwdFuncType->getResultType(),
            applyBwdFunc,
            applyFuncArgs);
        primalBuilder->markInstAsPrimal(contextVal);

        auto getValFunc = diffTypeContext.tryGetAssociationOfKind(
            baseFn,
            ValAssociationKind::BackwardDerivativeContextGetVal);
        SLANG_ASSERT(getValFunc);

        // Extract the primal return value from the context.
        // This will serve as the primal part of the mixed call.
        //
        auto primalReturnVal = primalBuilder->emitCallInst(
            as<IRFuncType>(diffTypeContext.resolveType(&globalBuilder, getValFunc->getDataType()))
                ->getResultType(),
            getValFunc,
            List<IRInst*>(contextVal));
        primalBuilder->markInstAsPrimal(primalReturnVal);

        //
        // Now place a call to the forward-propagation function for
        // the differential part of the mixed call.
        //
        // Note that the forward-propagation function is not actually
        // a real function, just a logically consistent placeholder
        // that the transpose function can flip (to the backward propagate
        // func)
        //

        List<IRInst*> propFuncArgs;
        propFuncArgs.add(contextVal);
        for (UIndex ii = 0; ii < mixedCall->getArgCount(); ii++)
        {
            auto arg = mixedCall->getArg(ii);
            if (isMixedDifferentialInst(arg))
                propFuncArgs.add(lookupDiffInst(arg));
            else
                propFuncArgs.add(diffBuilder->getVoidValue());
        }

        // Build forward-prop func type. This is just the .Differential
        // of each param type & result type.
        //
        List<IRType*> fwdPropFuncParamTypes;
        auto baseFuncType = cast<IRFuncType>(baseFn->getFullType());
        fwdPropFuncParamTypes.add(contextVal->getDataType());
        for (UIndex ii = 0; ii < baseFuncType->getParamCount(); ii++)
        {
            const auto& [paramDirection, paramType] =
                splitParameterDirectionAndType(baseFuncType->getParamType(ii));
            if (auto diffType = diffTypeContext.tryGetDifferentiableValueType(paramType))
            {
                fwdPropFuncParamTypes.add(
                    fromDirectionAndType(diffBuilder, paramDirection, (IRType*)diffType));
            }
            else
            {
                fwdPropFuncParamTypes.add(diffBuilder->getVoidType());
            }
        }

        IRType* fwdPropFuncResultType = nullptr;
        if (auto diffType =
                diffTypeContext.tryGetDifferentiableValueType(baseFuncType->getResultType()))
        {
            fwdPropFuncResultType = (IRType*)diffType;
        }
        else
        {
            fwdPropFuncResultType = diffBuilder->getVoidType();
        }

        auto fwdPropFuncType =
            diffBuilder->getFuncType(fwdPropFuncParamTypes, fwdPropFuncResultType);

        // ....
        auto fwdPropFuncInst =
            diffBuilder->emitForwardDifferentiatePropagateInst(fwdPropFuncType, baseFn);

        auto fwdPropCall = diffBuilder->emitCallInst(
            fwdPropFuncType->getResultType(),
            fwdPropFuncInst,
            propFuncArgs);
        diffBuilder->markInstAsDifferential(
            fwdPropCall,
            primalReturnVal->getDataType(),
            primalReturnVal);

        return InstPair(primalReturnVal, fwdPropCall);
    }

    InstPair splitMakePair(IRBuilder*, IRBuilder*, IRMakeDifferentialPair* mixedPair)
    {
        return InstPair(mixedPair->getPrimalValue(), mixedPair->getDifferentialValue());
    }

    InstPair splitLoad(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRLoad* mixedLoad)
    {
        auto primalPtr = lookupPrimalInst(mixedLoad->getPtr());
        auto diffPtr = lookupDiffInst(mixedLoad->getPtr());
        auto primalVal = primalBuilder->emitLoad(primalPtr);
        auto diffVal = diffBuilder->emitLoad(diffPtr);
        diffBuilder->markInstAsDifferential(diffVal, primalVal->getFullType());
        return InstPair(primalVal, diffVal);
    }

    InstPair splitStore(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRStore* mixedStore)
    {
        auto primalAddr = lookupPrimalInst(mixedStore->getPtr());
        auto diffAddr = lookupDiffInst(mixedStore->getPtr());

        auto primalVal = lookupPrimalInst(mixedStore->getVal());
        auto diffVal = lookupDiffInst(mixedStore->getVal());

        auto primalStore = primalBuilder->emitStore(primalAddr, primalVal);
        auto diffStore = diffBuilder->emitStore(diffAddr, diffVal);

        diffBuilder->markInstAsDifferential(diffStore, primalVal->getFullType());
        return InstPair(primalStore, diffStore);
    }

    InstPair splitVar(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRVar* mixedVar)
    {
        auto pairType =
            as<IRDifferentialPairType>(as<IRPtrTypeBase>(mixedVar->getDataType())->getValueType());
        auto primalType = pairType->getValueType();
        auto diffType = (IRType*)diffTypeContext.getDifferentialForType(primalType);
        auto primalVar = primalBuilder->emitVar(primalType);
        auto diffVar = diffBuilder->emitVar(diffType);
        diffBuilder->markInstAsDifferential(diffVar, diffBuilder->getPtrType(primalType));
        return InstPair(primalVar, diffVar);
    }

    InstPair splitReturn(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRReturn* mixedReturn)
    {
        auto pairType = as<IRDifferentialPairType>(mixedReturn->getVal()->getDataType());
        // Are we returning a differentiable value?
        if (pairType)
        {
            auto primalType = pairType->getValueType();

            // Check that we have an unambiguous 'first' differential block.
            SLANG_ASSERT(firstDiffBlock);

            auto primalBranch = primalBuilder->emitBranch(firstDiffBlock);
            primalBuilder->addBackwardDerivativePrimalReturnDecoration(
                primalBranch,
                lookupPrimalInst(mixedReturn->getVal()));

            /*auto pairVal = diffBuilder->emitMakeDifferentialPair(
                pairType,
                lookupPrimalInst(mixedReturn->getVal()),
                lookupDiffInst(mixedReturn->getVal()));
            diffBuilder->markInstAsDifferential(pairVal, primalType);*/

            auto returnInst = diffBuilder->emitReturn(lookupDiffInst(mixedReturn->getVal()));
            diffBuilder->markInstAsDifferential(returnInst, primalType);

            return InstPair(primalBranch, returnInst);
        }
        else
        {
            // If return value is not differentiable, just turn it into a trivial branch.
            auto primalBranch = primalBuilder->emitBranch(firstDiffBlock);
            primalBuilder->addBackwardDerivativePrimalReturnDecoration(
                primalBranch,
                mixedReturn->getVal());

            auto returnInst = diffBuilder->emitReturn();
            diffBuilder->markInstAsDifferential(returnInst, nullptr);
            return InstPair(primalBranch, returnInst);
        }
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

        // Split args.
        List<IRInst*> primalArgs;
        List<IRInst*> diffArgs;
        for (UIndex ii = 0; ii < mixedLoop->getArgCount(); ii++)
        {
            if (isDifferentialInst(mixedLoop->getArg(ii)))
                diffArgs.add(mixedLoop->getArg(ii));
            else
                primalArgs.add(mixedLoop->getArg(ii));
        }

        auto primalLoop = primalBuilder->emitLoop(
            as<IRBlock>(primalMap[nextBlock]),
            as<IRBlock>(primalMap[breakBlock]),
            as<IRBlock>(primalMap[continueBlock]),
            primalArgs.getCount(),
            primalArgs.getBuffer());

        auto diffLoop = diffBuilder->emitLoop(
            as<IRBlock>(diffMap[nextBlock]),
            as<IRBlock>(diffMap[breakBlock]),
            as<IRBlock>(diffMap[continueBlock]),
            diffArgs.getCount(),
            diffArgs.getBuffer());

        if (auto maxItersDecoration = mixedLoop->findDecoration<IRLoopMaxItersDecoration>())
        {
            primalBuilder->addLoopMaxItersDecoration(primalLoop, maxItersDecoration->getMaxIters());
            diffBuilder->addLoopMaxItersDecoration(diffLoop, maxItersDecoration->getMaxIters());
        }

        return InstPair(primalLoop, diffLoop);
    }

    InstPair splitControlFlow(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRInst* branchInst)
    {
        switch (branchInst->getOp())
        {
        case kIROp_UnconditionalBranch:
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

        case kIROp_ConditionalBranch:
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

        case kIROp_IfElse:
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

                for (UIndex ii = 0; ii < switchInst->getCaseCount(); ii++)
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

        case kIROp_Loop:
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

        case kIROp_Store:
            return splitStore(primalBuilder, diffBuilder, as<IRStore>(inst));

        case kIROp_Return:
            return splitReturn(primalBuilder, diffBuilder, as<IRReturn>(inst));

        case kIROp_UnconditionalBranch:
        case kIROp_ConditionalBranch:
        case kIROp_IfElse:
        case kIROp_Switch:
        case kIROp_Loop:
            return splitControlFlow(primalBuilder, diffBuilder, inst);

        case kIROp_Unreachable:
            return InstPair(primalBuilder->emitUnreachable(), diffBuilder->emitUnreachable());

        default:
            SLANG_ASSERT_FAILURE("Unhandled mixed diff inst");
        }
    }

    void splitMixedInst(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRInst* inst)
    {
        IRBuilderSourceLocRAII primalLocationScope(primalBuilder, inst->sourceLoc);
        IRBuilderSourceLocRAII diffLocationScope(diffBuilder, inst->sourceLoc);

        auto instPair = _splitMixedInst(primalBuilder, diffBuilder, inst);

        primalMap[inst] = instPair.primal;
        diffMap[inst] = instPair.differential;
    }

    void splitParameterBlock(IRBlock* paramBlock, IRBlock* primalBlock, IRBlock* diffBlock)
    {
        List<IRParam*> fwdParams;
        for (auto param : paramBlock->getParams())
            fwdParams.add(param);

        // Split the parameters into two and insert them into the
        // appropriate blocks.
        //
        IRBuilder primalBuilder(autodiffContext->moduleInst->getModule());
        primalBuilder.setInsertInto(primalBlock);

        IRBuilder diffBuilder(autodiffContext->moduleInst->getModule());
        diffBuilder.setInsertInto(diffBlock);

        for (auto fwdParam : fwdParams)
        {
            // Is this a mixed differential param?
            if (isMixedDifferentialInst(fwdParam))
            {
                const auto& [pairDirection, pairType] =
                    splitParameterDirectionAndType(fwdParam->getDataType());
                SLANG_ASSERT(isRelevantDifferentialPair(pairType));

                auto primalType = as<IRDifferentialPairTypeBase>(pairType)->getValueType();
                auto diffType = (IRType*)diffTypeContext.getDiffTypeFromPairType(
                    &diffBuilder,
                    as<IRDifferentialPairTypeBase>(pairType));

                auto primalParam = primalBuilder.emitParam(
                    fromDirectionAndType(&primalBuilder, pairDirection, primalType));
                auto diffParam = diffBuilder.emitParam(
                    fromDirectionAndType(&diffBuilder, pairDirection, diffType));
                diffBuilder.markInstAsDifferential(diffParam, primalType);

                primalParam->sourceLoc = fwdParam->sourceLoc;
                diffParam->sourceLoc = fwdParam->sourceLoc;

                primalMap[fwdParam] = primalParam;
                diffMap[fwdParam] = diffParam;
            }
            else
            {
                // Move to primal block.
                fwdParam->insertAtEnd(primalBlock);
                auto diffParam = diffBuilder.emitParam(diffBuilder.getVoidType());
                diffParam->sourceLoc = fwdParam->sourceLoc;
                primalMap[fwdParam] = fwdParam;
            }
        }

        splitControlFlow(&primalBuilder, &diffBuilder, paramBlock->getTerminator());
    }

    void splitBlock(IRBlock* block, IRBlock* primalBlock, IRBlock* diffBlock)
    {
        // Make two builders for primal and differential blocks.
        IRBuilder primalBuilder(autodiffContext->moduleInst->getModule());
        primalBuilder.setInsertInto(primalBlock);

        IRBuilder diffBuilder(autodiffContext->moduleInst->getModule());
        diffBuilder.setInsertInto(diffBlock);

        List<IRInst*> splitInsts;
        for (auto child : block->getModifiableChildren())
        {
            if (auto getDiffInst = as<IRDifferentialPairGetDifferential>(child))
            {
                // Replace GetDiff(A) with A.d
                if (diffMap.containsKey(getDiffInst->getBase()))
                {
                    getDiffInst->replaceUsesWith(lookupDiffInst(getDiffInst->getBase()));
                    getDiffInst->removeAndDeallocate();
                    continue;
                }
            }
            else if (auto getPrimalInst = as<IRDifferentialPairGetPrimal>(child))
            {
                // Replace GetPrimal(A) with A.p
                if (primalMap.containsKey(getPrimalInst->getBase()))
                {
                    getPrimalInst->replaceUsesWith(lookupPrimalInst(getPrimalInst->getBase()));
                    getPrimalInst->removeAndDeallocate();
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
        }

        // Remove insts that were split.
        for (auto inst : splitInsts)
        {
            // TODO: This is odd.. why do we need this?
            if (!diffTypeContext.isDifferentiableType(inst->getDataType()))
            {
                inst->replaceUsesWith(lookupPrimalInst(inst));
            }

            // Consistency check.
            for (auto use = inst->firstUse; use; use = use->nextUse)
            {
                SLANG_RELEASE_ASSERT(
                    (use->getUser()->getParent() != primalBlock) &&
                    (use->getUser()->getParent() != diffBlock));
            }

            // Leave terminator in to keep CFG info.
            if (!as<IRTerminatorInst>(inst))
                inst->removeAndDeallocate();
        }

        // Nothing should be left in the original block.
        SLANG_ASSERT(block->getFirstChild() == block->getTerminator());
    }
};

struct ExtractPrimalFuncContext
{
    IRModule* module;
    AutoDiffSharedContext* autodiffContext;
    DifferentiableTypeConformanceContext diffTypeContext;

    ExtractPrimalFuncContext(IRModule* inModule, AutoDiffSharedContext* autodiffContext)
        : module(inModule), autodiffContext(autodiffContext), diffTypeContext(autodiffContext)
    {
    }

    IRInst* cloneGenericHeader(IRBuilder& builder, IRCloneEnv& cloneEnv, IRGeneric* gen)
    {
        auto newGeneric = builder.emitGeneric();
        newGeneric->setFullType(builder.getTypeKind());
        for (auto decor : gen->getDecorations())
            cloneDecoration(decor, newGeneric);
        builder.emitBlock();
        auto originalBlock = gen->getFirstBlock();
        for (auto child = originalBlock->getFirstChild(); child != originalBlock->getLastParam();
             child = child->getNextInst())
        {
            cloneInst(&cloneEnv, &builder, child);
        }
        return newGeneric;
    }

    IRInst* createGenericIntermediateType(IRGeneric* gen)
    {
        IRBuilder builder(module);
        builder.setInsertBefore(gen);
        IRCloneEnv intermediateTypeCloneEnv;
        auto clonedGen = cloneGenericHeader(builder, intermediateTypeCloneEnv, gen);
        auto structType = builder.createStructType();
        builder.emitReturn(structType);
        auto func = findGenericReturnVal(gen);
        if (auto nameHint = func->findDecoration<IRNameHintDecoration>())
        {
            StringBuilder newName;
            newName << nameHint->getName() << "_Intermediates";
            builder.addNameHintDecoration(structType, UnownedStringSlice(newName.getBuffer()));
        }
        return clonedGen;
    }

    IRInst* createIntermediateType(IRGlobalValueWithCode* func)
    {
        if (func->getOp() == kIROp_Generic)
            return createGenericIntermediateType(as<IRGeneric>(func));
        IRBuilder builder(module);
        builder.setInsertBefore(func);

        auto intermediateType = builder.createStructType();

        builder.addDecoration(intermediateType, kIROp_OptimizableTypeDecoration);
        if (auto nameHint = func->findDecoration<IRNameHintDecoration>())
        {
            StringBuilder newName;
            newName << nameHint->getName() << "_Intermediates";
            builder.addNameHintDecoration(
                intermediateType,
                UnownedStringSlice(newName.getBuffer()));
        }

        return intermediateType;
    }

    IRInst* generatePrimalFuncType(
        IRGlobalValueWithCode* destFunc,
        IRGlobalValueWithCode* originalFunc,
        IRInst*& outIntermediateType)
    {
        IRBuilder builder(module);
        builder.setInsertBefore(destFunc);
        IRFuncType* originalFuncType = nullptr;

        outIntermediateType = builder.createStructType();
        builder.addDecoration(outIntermediateType, kIROp_OptimizableTypeDecoration);
        builder.addCheckpointIntermediateDecoration(outIntermediateType, originalFunc);
        outIntermediateType->sourceLoc = originalFunc->sourceLoc;

        originalFuncType = as<IRFuncType>(originalFunc->getDataType());

        IRInst* args[] = {originalFuncType, outIntermediateType};
        auto applyFuncType = diffTypeContext.resolveType(
            &builder,
            builder.emitIntrinsicInst(builder.getTypeKind(), kIROp_ApplyForBwdFuncType, 2, args));
        SLANG_RELEASE_ASSERT(as<IRFuncType>(applyFuncType));

        return applyFuncType;
    }

    IRInst* insertIntoReturnBlock(IRBuilder& builder, IRInst* inst)
    {
        if (!isDiffInst(inst))
            return inst;

        switch (inst->getOp())
        {
        case kIROp_Return:
            {
                IRInst* val = builder.getVoidValue();
                if (inst->getOperandCount() != 0)
                {
                    val = insertIntoReturnBlock(builder, inst->getOperand(0));
                }
                return builder.emitReturn(val);
            }
        case kIROp_MakeDifferentialPair:
            {
                auto diff = builder.emitDefaultConstruct(inst->getOperand(1)->getDataType());
                auto primal = insertIntoReturnBlock(builder, inst->getOperand(0));
                return builder.emitMakeDifferentialPair(inst->getDataType(), primal, diff);
            }
        default:
            SLANG_UNREACHABLE("unknown case of mixed inst.");
        }
    }

    IRStructField* addIntermediateContextField(IRInst* type, IRInst* intermediateOutput)
    {
        IRBuilder genTypeBuilder(module);
        auto ptrStructType = as<IRPtrTypeBase>(intermediateOutput->getDataType());
        SLANG_RELEASE_ASSERT(ptrStructType);
        auto structType = as<IRStructType>(ptrStructType->getValueType());
        if (auto outerGen = findOuterGeneric(structType))
            genTypeBuilder.setInsertBefore(outerGen);
        else
            genTypeBuilder.setInsertBefore(structType);
        auto fieldType = type;
        SLANG_RELEASE_ASSERT(structType);
        auto structKey = genTypeBuilder.createStructKey();
        genTypeBuilder.setInsertInto(structType);

        if (fieldType->getParent() != structType->getParent() &&
            isChildInstOf(fieldType->getParent(), structType->getParent()))
        {
            IRCloneEnv cloneEnv;
            fieldType = cloneInst(&cloneEnv, &genTypeBuilder, fieldType);
        }
        auto structField =
            genTypeBuilder.createStructField(structType, structKey, (IRType*)fieldType);

        return structField;
    }

    void storeInst(IRBuilder& builder, IRInst* inst, IRInst* intermediateOutput)
    {
        auto field = addIntermediateContextField(inst->getDataType(), intermediateOutput);
        field->sourceLoc = inst->sourceLoc;
        auto key = field->getKey();
        if (auto nameHint = inst->findDecoration<IRNameHintDecoration>())
            cloneDecoration(nameHint, key);
        builder.addPrimalValueStructKeyDecoration(inst, key);
        builder.emitStore(
            builder
                .emitFieldAddress(builder.getPtrType(inst->getFullType()), intermediateOutput, key),
            inst);
    }

    void storeReturnValue(IRBuilder& builder, IRInst* inst, IRInst* intermediateOutput)
    {
        auto field = addIntermediateContextField(inst->getDataType(), intermediateOutput);
        builder.addReturnValueContextFieldDecoration(field);
        field->sourceLoc = inst->sourceLoc;
        auto key = field->getKey();
        if (auto nameHint = inst->findDecoration<IRNameHintDecoration>())
            cloneDecoration(nameHint, key);
        builder.addPrimalValueStructKeyDecoration(inst, key);
        builder.emitStore(
            builder
                .emitFieldAddress(builder.getPtrType(inst->getFullType()), intermediateOutput, key),
            inst);
    }

    // Creates a function that takes the intermediate context type as input
    // and returns just the return value that was stored into the context type.
    //
    IRFunc* createGetValFunc(IRFunc*, IRInst* intermediateType, IRInst* returnValueKey)
    {
        IRBuilder builder(module);
        builder.setInsertAfter(intermediateType);

        IRFunc* getValFunc = builder.createFunc();
        IRType* returnType = nullptr;
        // Create the function type
        {
            List<IRType*> paramTypes;
            paramTypes.add(
                (IRType*)intermediateType); // Take the intermediate type as value, not pointer

            if (returnValueKey)
            {
                // Get the return type by looking up the field type
                auto structType = as<IRStructType>(intermediateType);
                for (auto field : structType->getFields())
                {
                    if (field->getKey() == returnValueKey)
                    {
                        returnType = field->getFieldType();
                        break;
                    }
                }

                SLANG_RELEASE_ASSERT(returnType);
            }
            else
            {
                returnType = builder.getVoidType();
            }

            auto funcType = builder.getFuncType(paramTypes, returnType);
            getValFunc->setFullType(funcType);
        }

        // Create parameter block
        builder.setInsertInto(getValFunc);
        auto paramBlock = builder.emitBlock();
        builder.setInsertInto(paramBlock);
        auto intermediateParam = builder.emitParam((IRType*)intermediateType);

        // Create function body block
        auto bodyBlock = builder.emitBlock();
        builder.setInsertInto(paramBlock);

        builder.emitBranch(bodyBlock);
        builder.setInsertInto(bodyBlock);

        // Extract the return value from the intermediate structure
        if (returnValueKey)
        {
            auto returnVal =
                builder.emitFieldExtract(returnType, intermediateParam, returnValueKey);
            builder.emitReturn(returnVal);
        }
        else
        {
            builder.emitReturn();
        }

        return getValFunc;
    }

    void setInsertOrdinaryInstIntoBlock(IRBuilder& builder, IRBlock* block)
    {
        auto firstOrdinaryInst = block->getFirstOrdinaryInst();
        if (firstOrdinaryInst)
            builder.setInsertBefore(firstOrdinaryInst);
        else
            builder.setInsertInto(block);
    }


    IRFunc* turnUnzippedFuncIntoPrimalFunc(
        IRFunc* unzippedFunc,
        IRFunc* originalFunc,
        HoistedPrimalsInfo* primalsInfo,
        IRInst*& outIntermediateType,
        // TODO(sai): cleanup. temporary extra value
        IRInst*& returnValueKey)
    {
        IRBuilder builder(module);

        IRFunc* func = unzippedFunc;
        IRInst* intermediateType = nullptr;
        auto newFuncType = generatePrimalFuncType(unzippedFunc, originalFunc, intermediateType);
        outIntermediateType = intermediateType;
        func->setFullType((IRType*)newFuncType);

        auto paramBlock = func->getFirstBlock();
        builder.setInsertInto(paramBlock);

        builder.setInsertBefore(paramBlock->getFirstOrdinaryInst());
        auto outIntermediary = builder.emitVar((IRType*)intermediateType);

        List<IRBlock*> primalBlocksList;

        for (auto block : func->getBlocks())
        {
            if (!(isDiffInst(block) || block->findDecoration<IRRecomputeBlockDecoration>()))
                primalBlocksList.add(block);
        }

        // Go over primal blocks and store insts that have been classified as requiring storage.
        for (auto block : primalBlocksList)
        {
            for (auto inst : block->getChildren())
            {
                if (primalsInfo->storeSet.contains(inst))
                {
                    if (as<IRVar>(inst))
                    {
                        if (inst->hasUses())
                        {
                            auto field = addIntermediateContextField(
                                cast<IRPtrTypeBase>(inst->getDataType())->getValueType(),
                                outIntermediary);
                            field->sourceLoc = inst->sourceLoc;
                            if (inst->findDecoration<IRLoopCounterDecoration>())
                                builder.addLoopCounterDecoration(field);

                            builder.setInsertBefore(inst);
                            auto fieldAddr = builder.emitFieldAddress(
                                inst->getFullType(),
                                outIntermediary,
                                field->getKey());
                            inst->replaceUsesWith(fieldAddr);
                            builder.addPrimalValueStructKeyDecoration(inst, field->getKey());
                        }
                    }
                    else
                    {
                        if (as<IRParam, IRDynamicCastBehavior::NoUnwrap>(inst))
                            builder.setInsertAfter(outIntermediary);
                        else
                            builder.setInsertAfter(inst);
                        storeInst(builder, inst, outIntermediary);
                    }
                }
            }
        }

        for (auto block : primalBlocksList)
        {
            auto term = block->getTerminator();
            builder.setInsertBefore(term);
            if (auto decor = term->findDecoration<IRBackwardDerivativePrimalReturnDecoration>())
            {
                auto returnValue = decor->getBackwardDerivativePrimalReturnValue();
                if (!as<IRVoidType>(returnValue->getDataType()))
                {
                    // If we have a non-void return, store it in the output intermediary.
                    storeReturnValue(builder, returnValue, outIntermediary);
                    if (returnValue->findDecoration<IRPrimalValueStructKeyDecoration>())
                    {
                        returnValueKey =
                            returnValue->findDecoration<IRPrimalValueStructKeyDecoration>()
                                ->getStructKey();
                    }
                }

                builder.emitReturn(builder.emitLoad(outIntermediary));
                term->removeAndDeallocate();
            }
        }

        // Remove all Differential & Recompute blocks from the apply func.

        List<IRBlock*> unusedBlocks;
        for (auto block : func->getBlocks())
        {
            if (isDiffInst(block) || block->findDecoration<IRRecomputeBlockDecoration>())
                unusedBlocks.add(block);
        }
        for (auto block : unusedBlocks)
            block->removeAndDeallocate();

        setInsertAfterOrdinaryInst(&builder, outIntermediary);
        auto defVal = builder.emitDefaultConstructRaw((IRType*)intermediateType);
        builder.emitStore(outIntermediary, defVal);

        return unzippedFunc;
    }
};

bool isIntermediateContextType(IRInst* type)
{
    switch (type->getOp())
    {
    case kIROp_BackwardDiffIntermediateContextType:
        return true;
    case kIROp_AttributedType:
        return isIntermediateContextType(as<IRAttributedType>(type)->getBaseType());
    case kIROp_Specialize:
        return isIntermediateContextType(as<IRSpecialize>(type)->getBase());
    default:
        if (auto ptrType = asRelevantPtrType(type))
            return isIntermediateContextType(ptrType->getValueType());
        return false;
    }
}

void markNonContextParamsAsSideEffectFree(IRBuilder* builder, IRFunc* func)
{
    for (auto param : func->getParams())
    {
        if (!param->findDecorationImpl(kIROp_PrimalContextDecoration))
            builder->addDecoration(param, kIROp_IgnoreSideEffectsDecoration);
    }
}

static void copyPrimalValueStructKeyDecorations(IRInst* inst, IRCloneEnv& cloneEnv)
{
    IRInst* newInst = nullptr;
    if (cloneEnv.mapOldValToNew.tryGetValue(inst, newInst))
    {
        if (auto decor = newInst->findDecoration<IRPrimalValueStructKeyDecoration>())
        {
            cloneDecoration(decor, inst);
        }
    }

    for (auto child : inst->getChildren())
    {
        copyPrimalValueStructKeyDecorations(child, cloneEnv);
    }
}

static IRBlock* getFirstRecomputeBlock(IRFunc* func)
{
    // This logic is a bit fragile.
    // We shouldn't necessarily make the
    // assumption that the order in the list of blocks is related to the
    // control-flow order, but it works with the current system.
    //
    for (auto block : func->getBlocks())
    {
        if (block->findDecoration<IRRecomputeBlockDecoration>())
            return block;
    }
    return nullptr;
}

void unzipDiffInsts(AutoDiffSharedContext* context, IRFunc* func)
{
    UnzippingContext unzipContext(context);
    unzipContext.unzipFunc(func);
}

IRFunc* splitApplyAndPropFuncs(
    AutoDiffSharedContext* autodiffContext,
    IRFunc* func,
    IRFunc* originalFunc,
    HoistedPrimalsInfo* primalsInfo,
    IRInst*& intermediateType,
    IRFunc*& getValFunc)
{
    IRBuilder builder(autodiffContext->moduleInst);
    builder.setInsertBefore(func);

    IRCloneEnv subEnv;
    subEnv.squashChildrenMapping = true;
    // subEnv.parent = &cloneEnv;
    auto clonedFunc = as<IRFunc>(cloneInst(&subEnv, &builder, func));
    auto clonedPrimalsInfo = primalsInfo->applyMap(&subEnv);

    // Remove [KeepAlive] decorations in clonedFunc.
    for (auto block : clonedFunc->getBlocks())
        for (auto inst : block->getChildren())
            if (auto decor = inst->findDecoration<IRKeepAliveDecoration>())
                decor->removeAndDeallocate();

    ExtractPrimalFuncContext context(autodiffContext->moduleInst->getModule(), autodiffContext);

    intermediateType = nullptr;
    IRInst* returnValueKey = nullptr;
    auto primalFunc = context.turnUnzippedFuncIntoPrimalFunc(
        clonedFunc,
        originalFunc,
        clonedPrimalsInfo,
        intermediateType,
        returnValueKey);

    getValFunc = context.createGetValFunc(primalFunc, intermediateType, returnValueKey);

    if (auto nameHint = primalFunc->findDecoration<IRNameHintDecoration>())
    {
        nameHint->removeAndDeallocate();
    }

    builder.addDecoration(primalFunc, kIROp_IgnoreSideEffectsDecoration);
    markNonContextParamsAsSideEffectFree(&builder, primalFunc);

    // Copy PrimalValueStructKey decorations from primal func.
    copyPrimalValueStructKeyDecorations(func, subEnv);

    auto propParamBlock = getFirstRecomputeBlock(func);
    SLANG_ASSERT(propParamBlock);

    // Add intermediate context parameter to the front.
    auto intermediateParam = builder.createParam((IRType*)intermediateType);
    propParamBlock->insertParamAtHead(intermediateParam);

    // Replace all insts that has intermediate results with a load of the intermediate.
    List<IRInst*> instsToRemove;
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (auto structKeyDecor = inst->findDecoration<IRPrimalValueStructKeyDecoration>())
            {
                builder.setInsertBefore(inst);
                if (inst->getOp() == kIROp_Var)
                {
                    // This is a var for intermediate context.
                    // Replace all loads of the var with a field extract.
                    // Other type of uses will get a temp var that stores a copy of the field.
                    while (auto use = inst->firstUse)
                    {
                        if (as<IRDecoration>(use->getUser()))
                        {
                            use->set(builder.getVoidValue());
                            continue;
                        }

                        IRBuilderSourceLocRAII sourceLocationScope(
                            &builder,
                            use->getUser()->sourceLoc);

                        builder.setInsertBefore(use->getUser());
                        auto valType = cast<IRPtrTypeBase>(inst->getFullType())->getValueType();
                        auto val = builder.emitFieldExtract(
                            valType,
                            intermediateParam,
                            structKeyDecor->getStructKey());

                        if (use->getUser()->getOp() == kIROp_Load)
                        {
                            use->getUser()->replaceUsesWith(val);
                            use->getUser()->removeAndDeallocate();
                        }
                        else
                        {
                            auto tempVar = builder.emitVar(valType);
                            builder.emitStore(tempVar, val);
                            use->set(tempVar);
                        }
                    }
                }
                else
                {
                    // Ordinary value.
                    // We insert a fieldExtract at each use site instead of before `inst`,
                    // since at this stage of autodiff pass, `inst` does not necessarily
                    // dominate all the use sites if `inst` is defined in partial branch
                    // in a primal block.
                    while (auto iuse = inst->firstUse)
                    {
                        auto user = iuse->getUser();
                        if (as<IRDecoration>(user))
                            user = user->getParent();
                        if (!user)
                            continue;
                        builder.setInsertBefore(user);
                        auto val = builder.emitFieldExtract(
                            inst->getFullType(),
                            intermediateParam,
                            structKeyDecor->getStructKey());
                        val->sourceLoc = user->sourceLoc;
                        builder.replaceOperand(iuse, val);
                    }
                }
                instsToRemove.add(inst);
            }
        }
    }

    for (auto inst : instsToRemove)
    {
        if (as<IRParam, IRDynamicCastBehavior::NoUnwrap>(inst))
            removePhiArgs(inst);
        inst->removeAndDeallocate();
    }

    // Erase all primal blocks (except for the param & prelude blocks).
    // TODO: Lots of ways to clean this up.
    //
    List<IRBlock*> blocksToRemove;
    for (auto block : func->getBlocks())
    {
        if (!block->findDecoration<IRRecomputeBlockDecoration>() &&
            !block->findDecoration<IRDifferentialInstDecoration>())
            blocksToRemove.add(block);
    }

    // Before erasing the blocks, go through and 're-hoist' any hoistable instructions (such as
    // types) Any remaining valid instructions should be automatically moved to the recompute
    // blocks. The rest can be removed.
    //
    List<IRInst*> instsToReHoist;
    for (auto block : blocksToRemove)
        for (auto inst : block->getChildren())
            if (getIROpInfo(inst->getOp()).flags & kIROpFlag_Hoistable)
                instsToReHoist.add(inst);

    for (auto inst : instsToReHoist)
    {
        inst->removeFromParent();
        addHoistableInst(&builder, inst);
    }

    for (auto block : blocksToRemove)
        block->removeAndDeallocate();

    return primalFunc;
}
} // namespace Slang
