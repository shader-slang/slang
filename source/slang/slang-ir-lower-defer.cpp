// slang-ir-lower-defer.cpp

#include "slang-ir-lower-defer.h"

#include "slang-ir-clone.h"
#include "slang-ir-dominators.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

struct DeferLoweringContext : InstPassBase
{
    DiagnosticSink* diagnosticSink;

    DeferLoweringContext(IRModule* inModule)
        : InstPassBase(inModule)
    {
    }

    void inlineSingleBlockDefer(IRInst* beforeInst, IRBlock* deferBlock, IRBuilder* builder)
    {
        builder->setInsertBefore(beforeInst);
        IRCloneEnv env;
        for(IRInst* inst: deferBlock->getChildren())
            cloneInst(&env, builder, inst);
    }

    // Returns the new last block.
    IRBlock* inlineDefer(IRInst* beforeInst, IRBlock* targetBlock, IRDefer* defer, IRBuilder* builder)
    {
        // The single-block inlining case is simple, we can just dump the
        // instructions at the target position, in the existing block.
        auto firstBlock = defer->getFirstBlock();
        if (!firstBlock->getNextBlock())
        {
            inlineSingleBlockDefer(beforeInst, firstBlock, builder);
            return targetBlock;
        }

        // Otherwise, we'll have to splice the blocks in.
        IRCloneEnv env;
        auto lastBlock = targetBlock;

        // Clone blocks first
        for (auto block : defer->getBlocks())
        {
            auto clonedBlock = builder->createBlock();
            clonedBlock->insertAfter(lastBlock);
            env.mapOldValToNew[block] = clonedBlock;
            lastBlock = clonedBlock;
        }

        // Then, clone instructions, but mapping old blocks to new blocks.
        for (auto block : defer->getBlocks())
        {
            auto clonedBlock = env.mapOldValToNew.getValue(block);
            builder->setInsertInto(clonedBlock);
            for (auto inst : block->getChildren())
            {
                cloneInst(&env, builder, inst);
            }
        }

        // Move old instructions to the last block's end. The last defer block
        // shouldn't have a terminator at this point yet.
        while (beforeInst)
        {
            auto nextInst = beforeInst->getNextInst();
            beforeInst->removeFromParent();
            beforeInst->insertAtEnd(lastBlock);
            beforeInst = nextInst;
        }

        // If the previous target had defer hooks, move them to the new one.
        IRDecoration* oldDecor = targetBlock->getFirstDecoration();
        while (oldDecor)
        {
            auto nextDecor = oldDecor->getNextDecoration();
            auto hook = as<IRDeferHookDecoration>(oldDecor);
            if (hook)
            {
                oldDecor->removeFromParent();
                oldDecor->insertAtStart(lastBlock);
            }
            oldDecor = nextDecor;
        }

        // Make target block jump to the cloned blocks.
        builder->setInsertInto(targetBlock);
        auto mainBlock = as<IRBlock>(env.mapOldValToNew.getValue(firstBlock));
        builder->emitBranch(mainBlock);
        return lastBlock;
    }

    bool blockSucceedsDeferScope(IRBlock* block, IRIntegerValue hookIndex)
    {
        // Scroll through previous blocks and check if they're decorated with
        // `DeferHook`s.
        IRBlock* prev = block->getPrevBlock();
        while (prev != nullptr)
        {
            for (IRDecoration* decor : prev->getDecorations())
            {
                auto hook = as<IRDeferHookDecoration>(decor);
                if (hook && hook->getHookIndex() == hookIndex)
                {
                    return true;
                }
            }
            prev = prev->getPrevBlock();
        }
        return false;
    }

    void processModule()
    {
        List<IRDefer*> unhandledDefers;
        processInstsOfType<IRDefer>(
            kIROp_Defer,
            [&](IRDefer* defer)
            {
                unhandledDefers.add(defer);
            });

        IRBuilder builder(module);

        // Iterating over `defer` instructions in reverse order allows us to
        // expand them in the correct order, including nested `defer`s.
        while (unhandledDefers.getCount() != 0)
        {
            IRDefer* defer = unhandledDefers.getLast();
            IRIntegerValue hook = defer->getHookIndex();

            unhandledDefers.removeLast();

            IRFunc* func = getParentFunc(defer);
            auto dom = module->findOrCreateDominatorTree(func);

            auto parentBlock = as<IRBlock>(defer->getParent());
            auto dominatedBlocks = dom->getProperlyDominatedBlocks(parentBlock);

            // Construct a set from the dominated blocks so that we can quickly
            // check if the terminator of a block jumps to a non-dominated
            // location.
            HashSet<IRBlock*> dominatedBlocksSet;
            dominatedBlocksSet.add(parentBlock);
            for (IRBlock* block : dominatedBlocks)
            {
                if (!blockSucceedsDeferScope(block, hook))
                    dominatedBlocksSet.add(block);
            }

            for (IRBlock* block : dominatedBlocksSet)
            {
                auto terminator = block->getTerminator();
                SLANG_ASSERT(terminator);
                bool exits = false;
                switch(terminator->getOp())
                {
                case kIROp_Return:
                case kIROp_discard:
                case kIROp_Throw:
                    exits = true;
                    break;
                case kIROp_unconditionalBranch:
                    {
                        auto targetBlock = as<IRBlock>(terminator->getOperand(0));
                        if (!dominatedBlocksSet.contains(targetBlock))
                        {
                            exits = true;
                        }
                    }
                    break;
                case kIROp_conditionalBranch:
                    {
                        auto trueBlock = as<IRBlock>(terminator->getOperand(1));
                        auto falseBlock = as<IRBlock>(terminator->getOperand(2));
                        if (
                            !dominatedBlocksSet.contains(trueBlock) ||
                            !dominatedBlocksSet.contains(falseBlock)
                        ){
                            exits = true;
                        }
                    }
                    break;
                default:
                    break;
                }

                if (exits)
                { // Duplicate child instructions to the end of this block.
                    if(inlineDefer(terminator, block, defer, &builder) != block)
                    {
                        // New last block is not the same as before, so mark
                        // analysis of the function with defer as outdated.
                        module->invalidateAnalysisForInst(func);
                    }
                }
            }

            defer->removeAndDeallocate();
        }

        // Remove all defer hooks, they're not needed anymore.
        processInstsOfType<IRDeferHookDecoration>(
            kIROp_DeferHookDecoration,
            [&](IRDeferHookDecoration* deferHook)
            {
                deferHook->removeAndDeallocate();
            });
    }
};

void lowerDefer(IRModule* module, DiagnosticSink* sink)
{
    DeferLoweringContext context(module);
    context.diagnosticSink = sink;
    return context.processModule();
}

} // namespace Slang
