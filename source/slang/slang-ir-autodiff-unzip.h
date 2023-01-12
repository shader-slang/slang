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

        IRBlock* firstBlock = unzippedFunc->getFirstBlock()->getNextBlock();

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

        // Split each block into two. 
        for (auto block : mixedBlocks)
        {
            splitBlock(block, as<IRBlock>(primalMap[block]), as<IRBlock>(diffMap[block]));
        }

        // Swap the first block's occurences out for the first primal block.
        firstBlock->replaceUsesWith(firstPrimalBlock);

        // Remove old blocks.
        for (auto block : mixedBlocks)
            block->removeAndDeallocate();

        return unzippedFunc;
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

    InstPair splitCall(IRBuilder* primalBuilder, IRBuilder* diffBuilder, IRCall* mixedCall)
    {
        IRBuilder globalBuilder;
        globalBuilder.init(autodiffContext->sharedBuilder);

        auto fwdCallee = as<IRForwardDifferentiate>(mixedCall->getCallee());
        auto fwdCalleeType = as<IRFuncType>(fwdCallee->getDataType());
        auto baseFn = fwdCallee->getBaseFn();

        auto primalFuncType = autodiffContext->transcriberSet.primalTranscriber->differentiateFunctionType(
            primalBuilder, baseFn, as<IRFuncType>(baseFn->getDataType()));

        auto intermediateVar = primalBuilder->emitVar(primalBuilder->getBackwardDiffIntermediateContextType(baseFn));
        primalBuilder->addBackwardDerivativePrimalContextDecoration(intermediateVar, intermediateVar);

        auto primalFn = primalBuilder->emitBackwardDifferentiatePrimalInst(primalFuncType, baseFn);

        List<IRInst*> primalArgs;
        for (UIndex ii = 0; ii < mixedCall->getArgCount(); ii++)
        {
            auto arg = mixedCall->getArg(0);

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
            auto arg = mixedCall->getArg(0);

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

            inst->removeAndDeallocate();
        }

        // Nothing should be left in the original block.
        SLANG_ASSERT(block->getFirstChild() == nullptr);

        // Branch from primal to differential block.
        // Functionally, the new blocks should produce the same output as the
        // old block.
        // primalBuilder.emitBranch(diffBlock);
    }
};

}
