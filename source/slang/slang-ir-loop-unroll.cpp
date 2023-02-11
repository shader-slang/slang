#include "slang-ir-loop-unroll.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-peephole.h"
#include "slang-ir-dominators.h"
#include "slang-ir-clone.h"
#include "slang-ir-util.h"
#include "slang-ir-simplify-cfg.h"

namespace Slang
{

static bool _eliminateDeadBlocks(List<IRBlock*>& blocks, IRBlock* unreachableBlock)
{
    if (blocks.getCount() == 0)
        return false;
    bool changed = false;
    HashSet<IRBlock*> aliveBlocks;
    aliveBlocks.Add(blocks[0]);
    List<IRBlock*> workList;
    workList.add(blocks[0]);
    for (Index i = 0; i < workList.getCount(); i++)
    {
        auto block = workList[i];
        for (auto succ : block->getSuccessors())
        {
            if (aliveBlocks.Add(succ))
            {
                workList.add(succ);
            }
        }
    }
    for (auto& b : blocks)
    {
        if (!aliveBlocks.Contains(b))
        {
            if (b->hasUses())
            {
                b->replaceUsesWith(unreachableBlock);
            }
            b->removeAndDeallocate();
            b = nullptr;
            changed = true;
        }
    }
    return changed;
}

List<IRBlock*> _collectBlocksInLoop(Dictionary<IRBlock*, int>& blockOrdering, IRLoop* loopInst)
{
    List<IRBlock*> loopBlocks;
    HashSet<IRBlock*> loopBlocksSet;
    auto addBlock = [&](IRBlock* block)
    {
        if (loopBlocksSet.Add(block))
            loopBlocks.add(block);
    };
    auto firstBlock = as<IRBlock>(loopInst->block.get());
    auto breakBlock = as<IRBlock>(loopInst->breakBlock.get());
    auto breakBlockOrdering = blockOrdering[breakBlock].GetValue();

    addBlock(firstBlock);
    for (Index i = 0; i < loopBlocks.getCount(); i++)
    {
        auto block = loopBlocks[i];
        for (auto succ : block->getSuccessors())
        {
            if (succ == breakBlock)
                continue;
            auto successorOrdering = blockOrdering[block].GetValue();
            // The target must be post-dominated by the break block in order to be considered
            // the body of the loop.
            // Since we don't support arbitrary goto or multi-level continue, the simple
            // ordering comparison is sufficient to serve as a post-dominance check.
            if (successorOrdering < breakBlockOrdering)
                addBlock(succ);
        }
    }
    return loopBlocks;
}

static int _getLoopMaxIterationsToUnroll(IRLoop* loopInst)
{
    static constexpr int kMaxIterationsToAttempt = 100;

    auto forceUnrollDecor = loopInst->findDecoration<IRForceUnrollDecoration>();
    if (!forceUnrollDecor)
        return -1;

    int maxIterations = kMaxIterationsToAttempt;
    auto maxIterCount = as<IRIntLit>(forceUnrollDecor->getOperand(0));
    if (maxIterCount && maxIterCount->getValue() != 0)
    {
        maxIterations =
            Math::Clamp(maxIterations, (int)maxIterCount->getValue() + 1, kMaxIterationsToAttempt);
    }
    return maxIterations;
}

static void _foldAndSimplifyLoopIteration(
    IRBuilder& builder,
    List<IRBlock*>& clonedBlocks,
    IRBlock* firstIterationBreakBlock,
    IRBlock* unreachableBlock)
{
    for (;;)
    {
        // Try to simplify and evaluate each inst in `firstIterationBreakBlock` and in
        // cloned loop body.
        for (auto b : clonedBlocks)
        {
            for (auto inst : b->getChildren())
            {
                tryReplaceInstUsesWithSimplifiedValue(builder.getSharedBuilder(), inst);
            }
        }

        // It is important to also evaluate `firstIterationBreakBlock` because we need to have
        // the phi arguments for next iteration evaluated (args in the new loop inst).
        for (auto inst : firstIterationBreakBlock->getChildren())
        {
            tryReplaceInstUsesWithSimplifiedValue(builder.getSharedBuilder(), inst);
        }

        // Fold conditional branches into unconditional branches if the condition is known.
        for (auto b : clonedBlocks)
        {
            auto terminator = b->getTerminator();
            if (auto cbranch = as<IRConditionalBranch>(terminator))
            {
                if (auto constCondition = as<IRConstant>(cbranch->getCondition()))
                {
                    auto targetBlock = (constCondition->value.intVal != 0) ? cbranch->getTrueBlock() : cbranch->getFalseBlock();
                    builder.setInsertBefore(cbranch);
                    builder.emitBranch(targetBlock);
                    cbranch->removeAndDeallocate();
                }
            }
            else if (auto switchInst = as<IRSwitch>(terminator))
            {
                if (auto constCondition = as<IRConstant>(switchInst->condition.get()))
                {
                    for (UInt i = 0; i < switchInst->getCaseCount(); i++)
                    {
                        if (constCondition == switchInst->getCaseValue(i))
                        {
                            builder.setInsertBefore(switchInst);
                            builder.emitBranch(switchInst->getCaseLabel(i));
                            switchInst->removeAndDeallocate();
                            break;
                        }
                    }
                }
            }
        }

        // DCE on CFG.
        bool hasChanges = _eliminateDeadBlocks(clonedBlocks, unreachableBlock);
        if (!hasChanges)
            break;

        // Delete removed blocks from clonedBlocks.
        Index insertIndex = 0;
        for (Index i = 0; i < clonedBlocks.getCount(); i++)
        {
            auto b = clonedBlocks[i];
            if (b)
            {
                if (i != insertIndex)
                {
                    clonedBlocks[insertIndex] = b;
                    insertIndex++;
                }
            }
        }
        clonedBlocks.setCount(insertIndex);
    }
}

// Unroll loop up to a predefined maximum number of iterations.
// Returns true if we can statically determine that the loop terminated within the iteration limit.
// This operation assumes the loop does not have `continue` jumps, i.e. continueBlock == targetBlock.
static bool _unrollLoop(
    SharedIRBuilder* sharedBuilder,
    IRLoop* loopInst,
    List<IRBlock*>& blocks)
{
    if (blocks.getCount() == 0)
    {
        IRBuilder subBuilder(sharedBuilder);
        subBuilder.setInsertBefore(loopInst);
        subBuilder.emitBranch(loopInst->getBreakBlock());
        loopInst->removeAndDeallocate();
        return true;
    }

    auto maxIterations = _getLoopMaxIterationsToUnroll(loopInst);
    if (maxIterations < 0)
        return true;

    // We assume all `continue`s are eliminated and turned into multi-level breaks
    // before this operation.
    SLANG_RELEASE_ASSERT(loopInst->getContinueBlock() == loopInst->getTargetBlock());

    // Insert an outer breakable region so we have a break label to use as the target for
    // any `break` jumps in the unrolled loop.
    // Transform CFG from [..., loopInst] -> [loopTarget] ->... [originalLoopBreakBlock]
    // Into: [..., loop] -> [outerBreakableRegionHeader, loopInst(phi_arg)] -> [(phi_param) loopTarget] -> ... ->
    //       [newLoopBreakBlock] -> [originalLoopBreakBlock/outerBreakableRegionBreakBlock]
    // After this transform, the original break block of the loop will serve as the break block for the
    // outer breakable region.

    IRBuilder builder(sharedBuilder);
    
    auto unreachableBlock = builder.createBlock();
    builder.setInsertInto(unreachableBlock);
    builder.emitUnreachable();
    unreachableBlock->insertAtEnd(loopInst->parent->parent);

    auto outerBreakableRegionHeader = builder.createBlock();
    outerBreakableRegionHeader->insertBefore(loopInst->getTargetBlock());
    
    auto newLoopBreakableRegionBreakBlock = builder.createBlock();
    newLoopBreakableRegionBreakBlock->insertBefore(loopInst->getBreakBlock());

    IRBlock* outerBreakableRegionBreakBlock = nullptr;
    {
        auto originalBreakBlock = loopInst->getBreakBlock();

        // Since all `break`s in the original loop body will become jumps into
        // `newLoopBreakableRegionBreakBlock` after unrolling, we need to make sure
        // `newLoopBreakableRegionBreakBlock` contains exactly the same set of
        // phi parameters as the original break block.

        IRCloneEnv cloneEnv;
        builder.setInsertInto(newLoopBreakableRegionBreakBlock);
        List<IRInst*> newParams;
        for (auto param : originalBreakBlock->getParams())
        {
            auto clonedParam = cloneInst(&cloneEnv, &builder, param);
            newParams.add(clonedParam);
        }

        // Make the existing code in the loop body to jump into `newLoopBreakableRegionBreakBlock`
        // instead, because we are going to make `originalBreakBlock` the new break block for
        // the outer breakable region.

        originalBreakBlock->replaceUsesWith(newLoopBreakableRegionBreakBlock);
        builder.emitBranch(originalBreakBlock, newParams.getCount(), newParams.getBuffer());

        // Use the original break block as the break block for the new outer loop.
        outerBreakableRegionBreakBlock = originalBreakBlock;

        // Use a loop inst to enter the breakable region. (This isn't a real loop).
        builder.setInsertBefore(loopInst);
        builder.emitLoop(
            outerBreakableRegionHeader,
            outerBreakableRegionBreakBlock,
            outerBreakableRegionHeader);

        // The original loop inst should now be moved into `outerBreakableRegionHeader`.
        loopInst->insertAtEnd(outerBreakableRegionHeader);
    }

    bool loopTerminated = false;
    for (int attempedIterations = 0; attempedIterations < maxIterations; attempedIterations++)
    {
        // Our task is to peel off the first iteration and put it in front of the
        // loop.
        // We will create a breakable region (via single iteration loop), and clone the loop body
        // into this region. This region is defined by the header block `firstIterationLoopHeader`,
        // and the converge block `firstIterationBreakBlock`.

        IRCloneEnv cloneEnv;

        auto loopTargetBlock = loopInst->getTargetBlock();
        auto firstIterationLoopHeader = builder.createBlock();
        firstIterationLoopHeader->insertBefore(loopTargetBlock);
        auto firstIterationBreakBlock = builder.createBlock();
        firstIterationBreakBlock->insertBefore(loopTargetBlock);

        // Map loop params for first iteration to arguments, so that
        // when we clone the blocks, these parameters will get replaced
        // with the actual arguments.
        UInt argId = 0;
        for (auto param : loopTargetBlock->getParams())
        {
            cloneEnv.mapOldValToNew[param] = loopInst->getArg(argId);
            argId++;
        }

        // While cloning the loop body, if we see any `break`s, we replace it with a branch
        // into outerBreakableRegionBreakBlock.
        // We replace the back edge with a jump into firstIterationBreakBlock.
        // The original loop will start from firstIterationBreakBlock.
        cloneEnv.mapOldValToNew[loopInst->getBreakBlock()] = outerBreakableRegionBreakBlock;
        cloneEnv.mapOldValToNew[loopInst->getTargetBlock()] = firstIterationBreakBlock;

        // Wire up the breakable region blocks.
        // Note that the breakable region header will never have any phi params because there will never
        // be back jumps into the header (it is a single iteration loop just for the break label).

        builder.setInsertBefore(loopInst);
        builder.emitLoop(firstIterationLoopHeader, firstIterationBreakBlock, firstIterationLoopHeader);

        // The `firstIterationBreakBlock` is supposed to act as the `targetBlock` for the back-jump in the
        // loop body. Therefore, if the original loop target block has any phi params, we will need the
        // same set of phi params in `firstIterationBreakBlock` so keep those branches valid.

        builder.setInsertInto(firstIterationBreakBlock);
        {
            IRCloneEnv paramCloneEnv;
            List<IRInst*> newParams;
            for (auto param : loopTargetBlock->getParams())
            {
                newParams.add(cloneInst(&paramCloneEnv, &builder, param));
            }

            // In `firstIterationBreakBlock`, we emit a new loop inst
            // to start a loop for the remaining iterations.
            auto newLoopInst = as<IRLoop>(builder.emitLoop(
                loopTargetBlock,
                loopInst->getBreakBlock(),
                loopInst->getContinueBlock(),
                newParams.getCount(),
                newParams.getBuffer()));
            loopInst->removeAndDeallocate();

            // Update `loopInst` to represent the remaining loop iterations that are yet to be unrolled.
            loopInst = newLoopInst;
        }

        // With the break region set up and wired, we can now clone the loop body into the break region.
        // We create all the blocks first, and setup the clone mapping for the blocks so when we
        // clone the insts later, the branch targets will automatically set to their clones.

        List<IRBlock*> clonedBlocks;
        for (auto b : blocks)
        {
            builder.setInsertBefore(firstIterationBreakBlock);
            auto clonedBlock = builder.createBlock();
            clonedBlock->insertBefore(firstIterationBreakBlock);
            cloneEnv.mapOldValToNew.AddIfNotExists(b, clonedBlock);
            clonedBlocks.add(clonedBlock);
        }

        // Now clone the insts inside each block.

        for (Index i = 0; i < blocks.getCount(); i++)
        {
            auto originalBlock = blocks[i];
            auto clonedBlock = clonedBlocks[i];
            builder.setInsertInto(clonedBlock);
            for (auto inst : originalBlock->getChildren())
            {
                cloneInst(&cloneEnv, &builder, inst);
            }
        }

        // Wire the break region header to jump to the first loop body block.

        builder.setInsertInto(firstIterationLoopHeader);
        builder.emitBranch(clonedBlocks[0]);

        // Cloned first block of the iteration should not have any params,
        // they must have been replaced with actual arguments since we have set up
        // the mappings for them before the clone.

        SLANG_RELEASE_ASSERT(clonedBlocks[0]->getFirstParam() == nullptr);

        // With all the insts for the first iteration in place, we now iteratively run
        // SCCP and simplification for the cloned blocks, in hope that some
        // conditional jumps can be folded into unconditional jumps.

        _foldAndSimplifyLoopIteration(
            builder, clonedBlocks, firstIterationBreakBlock, unreachableBlock);

        // Now we have peeled off one iteration from the loop, we check if there are any
        // branches into next iteration, if not, the loop terminates and we are done.

        bool hasJumpsToRemainingLoop = false;
        for (auto b : clonedBlocks)
        {
            for (auto succ : b->getSuccessors())
            {
                if (succ == firstIterationBreakBlock)
                {
                    hasJumpsToRemainingLoop = true;
                    break;
                }
            }
        }
        if (!hasJumpsToRemainingLoop)
        {
            loopTerminated = true;

            // Now we know the loop terminates and we have just emitted the last iteration.
            // We need to replace all uses of the insts defined within the loop body with their
            // clones in the last iteration.

            HashSet<IRBlock*> blockSet;
            for (auto block : blocks)
            {
                blockSet.Add(block);
            }
            for (auto block : blocks)
            {
                for (auto inst : block->getChildren())
                {
                    IRInst* newInst = nullptr;
                    if (!cloneEnv.mapOldValToNew.TryGetValue(inst, newInst))
                        continue;
                    for (auto use = inst->firstUse; use;)
                    {
                        auto nextUse = use->nextUse;
                        if (!blockSet.Contains(as<IRBlock>(use->getUser()->getParent())))
                        {
                            use->set(newInst);
                        }
                        use = nextUse;
                    }
                }
            }

            // Now we can safely delete the original loop blocks.

            for (auto block : blocks)
            {
                block->replaceUsesWith(unreachableBlock);
                block->removeAndDeallocate();
            }

            // firstIterationBreakBlock is no longer reachable, so we can delete its children
            // and turn it into an unreachable block.

            firstIterationBreakBlock->removeAndDeallocateAllDecorationsAndChildren();
            builder.setInsertInto(firstIterationBreakBlock);
            builder.emitBranch(unreachableBlock);

            break;
        }
    }

    return loopTerminated;
}

bool unrollLoopsInFunc(
    SharedIRBuilder* sharedBuilder,
    IRGlobalValueWithCode* func,
    DiagnosticSink* sink)
{
    List<IRLoop*> loops;

    // Post order processing allows us to process inner loops first.
    auto postOrder = getPostorder(func);

    for (auto block : postOrder)
    {
        if (auto loop = as<IRLoop>(block->getTerminator()))
        {
            if (loop->findDecoration<IRForceUnrollDecoration>())
            {
                loops.add(loop);
            }
        }
    }

    if (loops.getCount() == 0)
        return true;

    for (auto loop : loops)
    {
        auto postOrderReverseCFG = getPostorderOnReverseCFG(func);
        Dictionary<IRBlock*, int> blockOrdering;
        
        for (Index i = 0; i < postOrderReverseCFG.getCount(); i++)
        {
            blockOrdering[postOrderReverseCFG[i]] = (int)i;
        }

        auto blocks = _collectBlocksInLoop(blockOrdering, loop);
        auto loopLoc = loop->sourceLoc;
        if (!_unrollLoop(sharedBuilder, loop, blocks))
        {
            if (sink)
                sink->diagnose(loopLoc, Diagnostics::cannotUnrollLoop);
            return false;
        }

        // Make sure we simplify things as much as possible before
        // attempting to potentially unroll outer loop.
        simplifyCFG(func);
    }
    return true;
}

bool unrollLoopsInModule(SharedIRBuilder* sharedBuilder, IRModule* module, DiagnosticSink* sink)
{
    for (auto inst : module->getGlobalInsts())
    {
        if (auto genFunc = as<IRGeneric>(inst))
        {
            if (auto func = as<IRGlobalValueWithCode>(findGenericReturnVal(genFunc)))
            {
                bool result = unrollLoopsInFunc(sharedBuilder, func, sink);
                if (!result)
                    return false;
            }
        }
        else if (auto func = as<IRGlobalValueWithCode>(inst))
        {
            bool result = unrollLoopsInFunc(sharedBuilder, func, sink);
            if (!result)
                return false;
        }
    }
    return true;
}

}
