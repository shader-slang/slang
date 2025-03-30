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
        for (IRInst* inst : deferBlock->getChildren())
        {
            // Copy everything except the terminator; the terminator should only
            // be a jump to mergeBlock, which isn't needed after inlining.
            if (!as<IRTerminatorInst>(inst))
                cloneInst(&env, builder, inst);
        }
    }

    // Returns the new last block.
    IRBlock* inlineDefer(
        IRInst* beforeInst,
        IRBlock* targetBlock,
        const List<IRBlock*>& deferBlocks,
        IRBlock* mergeBlock,
        IRBuilder* builder)
    {
        // The single-block inlining case is simple, we can just dump the
        // instructions at the target position, in the existing block.
        if (deferBlocks.getCount() == 1)
        {
            inlineSingleBlockDefer(beforeInst, deferBlocks.getFirst(), builder);
            return targetBlock;
        }

        // Otherwise, we'll have to splice the blocks in.
        IRCloneEnv env;
        builder->setInsertAfter(targetBlock);
        auto lastBlock = targetBlock;

        // Clone blocks first
        for (auto block : deferBlocks)
        {
            auto clonedBlock = builder->createBlock();
            builder->addInst(clonedBlock);
            env.mapOldValToNew[block] = clonedBlock;
        }

        // Then, clone instructions, but mapping old blocks to new blocks.
        for (auto block : deferBlocks)
        {
            auto clonedBlock = as<IRBlock>(env.mapOldValToNew.getValue(block));
            builder->setInsertInto(clonedBlock);
            for (auto inst : block->getChildren())
            {
                auto endBranch = as<IRUnconditionalBranch>(inst);
                if (endBranch && endBranch->getTargetBlock() == mergeBlock)
                {
                    lastBlock = clonedBlock;
                }
                else
                    cloneInst(&env, builder, inst);
            }
        }

        // Move old instructions to the last block's end. The last defer block
        // shouldn't have a terminator at this point yet.
        while (beforeInst)
        {
            auto nextInst = beforeInst->getNextInst();
            beforeInst->insertAtEnd(lastBlock);
            beforeInst = nextInst;
        }

        // Make target block jump to the cloned blocks.
        builder->setInsertInto(targetBlock);
        auto mainBlock = as<IRBlock>(env.mapOldValToNew.getValue(deferBlocks[0]));
        builder->emitBranch(mainBlock);

        return lastBlock;
    }

    bool isBlockInScope(IRBlock* block, IRDominatorTree* dom, IRBlock* scopeEndBlock)
    {
        if (block == scopeEndBlock)
            return false;

        // Technically, we'd like to know if 'scopeEndBlock' is a predecessor of
        // 'block'. This is usually the same as 'scopeEndBlock' dominating the
        // given block, except for when a statement can jump over the scope end
        // without completely exiting the dominance of the 'defer' statement
        // itself. The only case like this is the continue statement of a for
        // loop.
        if (dom->properlyDominates(scopeEndBlock, block))
            return false;

        // The continuation block is not always dominated by the end of the
        // scope, since a continue statement is another way to get there.
        // Luckily, we can just check if the scope end is directly succeeded by
        // the target block, which is the case in for loops.
        for (auto successor : scopeEndBlock->getSuccessors())
        {
            if (successor == block)
                return false;
        }

        return true;
    }

    void processFunc(IRGlobalValueWithCode* func)
    {
        // Iterating over `defer` instructions in reverse order allows us to
        // expand them in the correct order, including nested `defer`s.
        List<IRBlock*> reverseBlocks = getReversePostorderOnReverseCFG(func);
        List<IRDefer*> unhandledDefers;

        for (IRBlock* block : reverseBlocks)
        {
            for (auto child = block->getLastChild(); child; child = child->getPrevInst())
            {
                if (auto defer = as<IRDefer>(child))
                    unhandledDefers.add(defer);
            }
        }

        IRBuilder builder(module);
        Dictionary<IRBlock*, IRBlock*> mapOldScopeToNew;
        for (IRDefer* defer : unhandledDefers)
        {
            IRBlock* firstDeferBlock = defer->getDeferBlock();
            IRBlock* mergeBlock = defer->getMergeBlock();
            IRBlock* scopeEndBlock = defer->getScopeBlock();
            mapOldScopeToNew.tryGetValue(scopeEndBlock, scopeEndBlock);
            IRBlock* parentBlock = as<IRBlock>(defer->getParent());

            // The dominator tree gets invalidated on every iteration, so it's
            // necessary to construct it inside the loop.
            auto dom = module->findOrCreateDominatorTree(func);

            // Enumerate defer block range. That is, all blocks dominated by
            // parentBlock and not dominated by mergeBlock.
            auto deferDominatedBlocks = dom->getProperlyDominatedBlocks(firstDeferBlock);
            List<IRBlock*> deferBlocks;
            deferBlocks.add(firstDeferBlock);
            for (IRBlock* block : deferDominatedBlocks)
            {
                if (!dom->properlyDominates(mergeBlock, block) && block != mergeBlock)
                    deferBlocks.add(block);
            }

            auto dominatedBlocks = dom->getProperlyDominatedBlocks(mergeBlock);
            HashSet<IRBlock*> scopeBlocksSet;
            scopeBlocksSet.add(mergeBlock);
            for (IRBlock* block : dominatedBlocks)
            {
                if (isBlockInScope(block, dom, scopeEndBlock))
                    scopeBlocksSet.add(block);
            }

            // All jumps from blocks in scope to blocks out of scope are to be
            // preceded by a copy of the deferBlocks.
            for (IRBlock* block : scopeBlocksSet)
            {
                auto terminator = block->getTerminator();
                SLANG_ASSERT(terminator);
                bool exits = false;
                switch (terminator->getOp())
                {
                case kIROp_Return:
                case kIROp_discard:
                case kIROp_Throw:
                    exits = true;
                    break;
                case kIROp_unconditionalBranch:
                    {
                        auto targetBlock = as<IRBlock>(terminator->getOperand(0));
                        if (!scopeBlocksSet.contains(targetBlock))
                        {
                            exits = true;
                        }
                    }
                    break;
                case kIROp_conditionalBranch:
                    {
                        auto trueBlock = as<IRBlock>(terminator->getOperand(1));
                        auto falseBlock = as<IRBlock>(terminator->getOperand(2));
                        if (!scopeBlocksSet.contains(trueBlock) ||
                            !scopeBlocksSet.contains(falseBlock))
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
                    auto newEnd = inlineDefer(terminator, block, deferBlocks, mergeBlock, &builder);
                    if (newEnd != block)
                    {
                        mapOldScopeToNew[block] = newEnd;
                    }
                }
            }

            // Replace defer with unconditional branch to mergeBlock. Defer
            // blocks should now be orphaned, and we can remove them too.
            defer->removeAndDeallocate();
            builder.setInsertInto(parentBlock);
            builder.emitBranch(mergeBlock);

            for (IRBlock* deferBlock : deferBlocks)
            {
                deferBlock->removeAndDeallocate();
            }

            // Some blocks got removed and added, so mark analysis of the
            // function with defer as outdated.
            module->invalidateAnalysisForInst(func);
        }
    }

    void processModule()
    {
        processInstsOfType<IRFunc>(
            kIROp_Func,
            [&](IRFunc* func) { processFunc(func); });
    }
};

void lowerDefer(IRModule* module, DiagnosticSink* sink)
{
    DeferLoweringContext context(module);
    context.diagnosticSink = sink;
    return context.processModule();
}

} // namespace Slang
