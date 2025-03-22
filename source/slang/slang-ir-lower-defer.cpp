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

    void inlineSingleBlockDefer(IRInst* terminator, IRBlock* deferBlock, IRBuilder* builder)
    {
        // Insert to the end of the block right before the terminator.
        builder->setInsertBefore(terminator);

        IRCloneEnv env;
        for(IRInst* inst: deferBlock->getChildren())
            cloneInst(&env, builder, inst);
    }

    void inlineDefer(IRInst* terminator, IRBlock* targetBlock, IRDefer* defer, IRBuilder* builder)
    {
        // The single-block inlining case is simple, we can just dump the
        // instructions at the target position, in the existing block.
        auto firstBlock = defer->getFirstBlock();
        if (!firstBlock->getNextBlock())
        {
            inlineSingleBlockDefer(terminator, firstBlock, builder);
            return;
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

        // Move old terminator to afterBlock
        terminator->removeFromParent();
        terminator->insertAtEnd(lastBlock);

        // Make target block jump to the cloned blocks.
        builder->setInsertInto(targetBlock);
        auto mainBlock = as<IRBlock>(env.mapOldValToNew.getValue(firstBlock));
        builder->emitBranch(mainBlock);
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
                dominatedBlocksSet.add(block);

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
                case kIROp_conditionalBranch:
                    // Check through all Block operands and check that all
                    // are dominated.
                    for (UInt index = 0; index < terminator->getOperandCount(); ++index)
                    {
                        auto target = as<IRBlock>(terminator->getOperand(index));
                        if (target && !dominatedBlocksSet.contains(target))
                        {
                            exits = true;
                        }
                    }
                    break;
                default:
                    break;
                }

                if (exits)
                { // Duplicate child instructions to the end of this block.
                    inlineDefer(terminator, block, defer, &builder);
                }
            }

            defer->removeAndDeallocate();
        }
    }
};

void lowerDefer(IRModule* module, DiagnosticSink* sink)
{
    DeferLoweringContext context(module);
    context.diagnosticSink = sink;
    return context.processModule();
}

} // namespace Slang
