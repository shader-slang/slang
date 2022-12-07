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

    DiffUnzipPass(AutoDiffSharedContext* autodiffContext) : 
        autodiffContext(autodiffContext)
    { }

    IRFunc* unzipDiffInsts(IRFunc* func)
    {
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

    void splitBlock(IRBlock* mainBlock, IRBlock* primalBlock, IRBlock* diffBlock)
    {
        // Make two builders for primal and differential blocks.
        IRBuilder primalBuilder;
        primalBuilder.init(autodiffContext->sharedBuilder);
        primalBuilder.setInsertInto(primalBlock);

        IRBuilder diffBuilder;
        diffBuilder.init(autodiffContext->sharedBuilder);
        diffBuilder.setInsertInto(diffBlock);

        for (auto child = mainBlock->getFirstChild(); child;)
        {
            IRInst* nextChild = child->getNextInst();

            if (isDifferentialInst(child) || as<IRTerminatorInst>(child))
            {
                child->insertAtEnd(diffBlock);
            }
            else
            {
                child->insertAtEnd(primalBlock);
            }

            child = nextChild;
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
