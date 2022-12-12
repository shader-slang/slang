// slang-ir-autodiff-unzip.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"

#include "slang-ir-autodiff.h"
#include "slang-ir-autodiff-propagate.h"

namespace Slang
{

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

    DiffUnzipPass(AutoDiffSharedContext* autodiffContext) : 
        autodiffContext(autodiffContext), diffTypeContext(autodiffContext)
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
        IRFunc* unzippedFunc = as<IRFunc>(cloneInst(&cloneEnv, builder, func));

        builder->setInsertInto(unzippedFunc);

        // Work *only* with two-block functions for now.
        SLANG_ASSERT(unzippedFunc->getFirstBlock() != nullptr);
        SLANG_ASSERT(unzippedFunc->getFirstBlock()->getNextBlock() != nullptr);
        SLANG_ASSERT(unzippedFunc->getFirstBlock()->getNextBlock()->getNextBlock() == nullptr);

        // Ignore the first block (this is reserved for parameters), start
        // at the second block. (For now, we work with only a single block of insts)
        // TODO: expand to handle multi-block functions later.

        IRBlock* mainBlock = unzippedFunc->getFirstBlock()->getNextBlock();
        
        // Emit new blocks for split vesions of mainblock.
        IRBlock* primalBlock = builder->emitBlock();
        IRBlock* diffBlock = builder->emitBlock(); 

        // Mark the differential block as a differential inst.
        builder->markInstAsDifferential(diffBlock);

        // Split the main block into two. This method should also emit
        // a branch statement from primalBlock to diffBlock.
        // TODO: extend this code to split multiple blocks
        // 
        splitBlock(mainBlock, primalBlock, diffBlock);

        // Replace occurences of mainBlock with primalBlock
        mainBlock->replaceUsesWith(primalBlock);
        mainBlock->removeAndDeallocate();
        
        return unzippedFunc;
    }

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

        auto mixedDecoration = mixedCall->findDecoration<IRMixedDifferentialInstDecoration>();
        SLANG_ASSERT(mixedDecoration);

        auto fwdPairResultType = as<IRDifferentialPairType>(mixedDecoration->getPairType());
        SLANG_ASSERT(fwdPairResultType);

        auto primalType = fwdPairResultType->getValueType();
        auto diffType = (IRType*) diffTypeContext.getDifferentialForType(&globalBuilder, primalType);

        auto primalVal = primalBuilder->emitCallInst(primalType, baseFn, primalArgs);
        
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

    InstPair splitReturn(IRBuilder*, IRBuilder* diffBuilder, IRReturn* mixedReturn)
    {
        auto pairType = as<IRDifferentialPairType>(mixedReturn->getVal()->getDataType());
        auto primalType = pairType->getValueType();
        
        auto pairVal = diffBuilder->emitMakeDifferentialPair(
            pairType,
            lookupPrimalInst(mixedReturn->getVal()),
            lookupDiffInst(mixedReturn->getVal()));
        diffBuilder->markInstAsDifferential(pairVal, primalType);

        auto returnInst = diffBuilder->emitReturn(pairVal);
        diffBuilder->markInstAsDifferential(returnInst, primalType);

        return InstPair(nullptr, returnInst);
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

    void splitBlock(IRBlock* mainBlock, IRBlock* primalBlock, IRBlock* diffBlock)
    {
        // Make two builders for primal and differential blocks.
        IRBuilder primalBuilder;
        primalBuilder.init(autodiffContext->sharedBuilder);
        primalBuilder.setInsertInto(primalBlock);

        IRBuilder diffBuilder;
        diffBuilder.init(autodiffContext->sharedBuilder);
        diffBuilder.setInsertInto(diffBlock);

        List<IRInst*> splitInsts;
        for (auto child = mainBlock->getFirstChild(); child;)
        {
            IRInst* nextChild = child->getNextInst();

            if (auto getDiffInst = as<IRDifferentialPairGetDifferential>(child))
            {
                if (diffMap.ContainsKey(getDiffInst->getBase()))
                {
                    getDiffInst->replaceUsesWith(lookupDiffInst(getDiffInst->getBase()));
                    getDiffInst->removeAndDeallocate();
                    child = nextChild;
                    continue;
                }
            }

            if (auto getPrimalInst = as<IRDifferentialPairGetPrimal>(child))
            {
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
        SLANG_ASSERT(mainBlock->getFirstChild() == nullptr);

        // Branch from primal to differential block.
        // Functionally, the new blocks should produce the same output as the
        // old block.
        primalBuilder.emitBranch(diffBlock);
    }
};

}
