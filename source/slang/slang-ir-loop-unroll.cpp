#include "slang-ir-loop-unroll.h"

#include "core/slang-performance-profiler.h"
#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-peephole.h"
#include "slang-ir-simplify-cfg.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

static bool _eliminateDeadBlocks(List<IRBlock*>& blocks, IRBlock* unreachableBlock)
{
    if (blocks.getCount() == 0)
        return false;
    bool changed = false;
    HashSet<IRBlock*> aliveBlocks;
    aliveBlocks.add(blocks[0]);
    List<IRBlock*> workList;
    workList.add(blocks[0]);
    for (Index i = 0; i < workList.getCount(); i++)
    {
        auto block = workList[i];
        for (auto succ : block->getSuccessors())
        {
            if (aliveBlocks.add(succ))
            {
                workList.add(succ);
            }
        }
    }
    for (auto& b : blocks)
    {
        if (!aliveBlocks.contains(b))
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

static constexpr int kMaxIterationsToAttempt = 4096;

// Number of peel attempts `_unrollLoop` is allowed for a loop, or -1 if the loop is not
// `[ForceUnroll]`. A user `[ForceUnroll(N)]` permits N iterations, i.e. N + 1 peel attempts (the
// extra one folds the exit test after the Nth body). `[ForceUnroll]` / `[ForceUnroll(0)]` means "as
// many as it takes", capped at `kMaxIterationsToAttempt`.
//
// When unrolling is split across `_unrollLoop` calls (the deferral pass peels part of a loop and
// leaves the rest for the diagnosing pass), the residual carries its *remaining* attempt budget in
// a second decoration operand so the promised iteration count is not silently reset to the full
// budget on the later call. That operand holds an attempt count directly (0 = no attempts left, so
// a still-iterating residual is reported), which is why it is read here in preference to the user
// count rather than reusing operand 0 — operand 0's 0 means "unlimited", the opposite of "none".
static int _getLoopMaxIterationsToUnroll(IRLoop* loopInst)
{
    auto forceUnrollDecor = loopInst->findDecoration<IRForceUnrollDecoration>();
    if (!forceUnrollDecor)
        return -1;

    if (forceUnrollDecor->getOperandCount() > 1)
    {
        if (auto remaining = as<IRIntLit>(forceUnrollDecor->getOperand(1)))
            return Math::Min((int)remaining->getValue(), kMaxIterationsToAttempt);
    }

    int maxIterations = kMaxIterationsToAttempt;
    auto maxIterCount = as<IRIntLit>(forceUnrollDecor->getOperand(0));
    if (maxIterCount && maxIterCount->getValue() != 0)
    {
        maxIterations = Math::Min((int)maxIterCount->getValue() + 1, kMaxIterationsToAttempt);
    }
    return maxIterations;
}

static void _foldAndSimplifyLoopIteration(
    TargetProgram* targetProgram,
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
                tryReplaceInstUsesWithSimplifiedValue(targetProgram, builder.getModule(), inst);
            }
        }

        // It is important to also evaluate `firstIterationBreakBlock` because we need to have
        // the phi arguments for next iteration evaluated (args in the new loop inst).
        for (auto inst : firstIterationBreakBlock->getChildren())
        {
            tryReplaceInstUsesWithSimplifiedValue(targetProgram, builder.getModule(), inst);
        }

        // Fold conditional branches into unconditional branches if the condition is known.
        for (auto b : clonedBlocks)
        {
            auto terminator = b->getTerminator();
            if (auto cbranch = as<IRConditionalBranch>(terminator))
            {
                if (auto constCondition = as<IRConstant>(cbranch->getCondition()))
                {
                    auto targetBlock = (constCondition->value.intVal != 0)
                                           ? cbranch->getTrueBlock()
                                           : cbranch->getFalseBlock();
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
                clonedBlocks[insertIndex] = b;
                insertIndex++;
            }
        }
        clonedBlocks.setCount(insertIndex);
    }
}

// If a loop induction param has uses outside of the loop, create
// a duplicate var before the loop inst, and insert an update to the
// variable at start of each iteration. Then replace all outside
// uses to load from the new var instead.
// This transformation ensures that any uses of induction variables
// outside of the loop will get the up-to-date value after
// unrolling the loop.
//
//
// For reference, the following code will create a situation where an induction param is
// used outside the loop:
// ```
//    int sum = 0; // sum is an induction variable.
//    for (int i = 0; i < N; i++) {
//        sum += i;
//    }
//    use(sum);
// ```
static void allocVarForLoopInductionPhiParam(
    IRModule* module,
    IRLoop* loopInst,
    List<IRBlock*>& blocks)
{
    // Collect all blocks in the loop into a set so we can
    // quickly check if a use is inside or outside the loop.
    HashSet<IRBlock*> loopBlocks;
    for (auto b : blocks)
        loopBlocks.add(b);

    auto targetBlock = loopInst->getTargetBlock();

    struct NewBreakParamInfo
    {
        IRVar* inductionVar;             // The new variable created before the loop inst.
        IRParam* originalInductionParam; // The original induction param in the targetBlock.
    };

    // Collect all induction params that have uses outside of the loop.
    ShortList<NewBreakParamInfo> newBreakParams;
    for (auto param : targetBlock->getParams())
    {
        ShortList<IRUse*> outsideUses;
        for (auto use = param->firstUse; use; use = use->nextUse)
        {
            auto userBlock = getBlock(use->getUser());
            if (!loopBlocks.contains(userBlock))
            {
                outsideUses.add(use);
            }
        }
        if (outsideUses.getCount() != 0)
        {
            IRBuilder builder(module);
            builder.setInsertBefore(loopInst);
            auto inductionVar = builder.emitVar(param->getDataType());
            if (auto nameHintDecor = param->findDecoration<IRNameHintDecoration>())
            {
                builder.addNameHintDecoration(inductionVar, nameHintDecor->getName());
            }
            newBreakParams.add({inductionVar, param});
            setInsertAfterOrdinaryInst(&builder, param);
            builder.emitStore(inductionVar, param);
            for (auto use : outsideUses)
            {
                builder.setInsertBefore(use->getUser());
                auto newParam = builder.emitLoad(param->getDataType(), inductionVar);
                builder.replaceOperand(use, newParam);
            }
        }
    }
}

// Collect the blocks of one peeled iteration whose branch decides whether this loop iterates
// again or exits — its "fate separators" — into `outSeparators`.
//
// A peeled iteration reaches the remaining loop through `continueSink` (a back-edge to the not-
// yet-peeled iterations) and leaves the loop through `exitSink` (the loop's break target). A
// separator is a conditional branch that can reach `continueSink` on one side and `exitSink`
// (without first continuing) on the other — precisely a test that governs the loop's own
// continue/exit decision. Reachability is computed over `blocks` alone with the two sinks as
// terminals, so a nested inner loop's own test is *not* a separator here: both of its arms
// eventually rejoin this iteration's continue path, so neither leads to `exitSink`.
//
// A loop can have several separators (e.g. a header test plus an independent `break`); all are
// collected so the caller can tell whether *any* of them resolved this peel.
static void _collectLoopFateSeparators(
    const List<IRBlock*>& blocks,
    IRBlock* continueSink,
    IRBlock* exitSink,
    HashSet<IRBlock*>& outSeparators)
{
    HashSet<IRBlock*> regionBlocks;
    for (auto b : blocks)
        regionBlocks.add(b);

    // canReach[sink]: region blocks from which `sink` is reachable without leaving the region.
    auto computeCanReach = [&](IRBlock* sink)
    {
        HashSet<IRBlock*> canReach;
        for (bool changed = true; changed;)
        {
            changed = false;
            for (auto b : blocks)
            {
                if (canReach.contains(b))
                    continue;
                for (auto succ : b->getSuccessors())
                {
                    if (succ == sink || (regionBlocks.contains(succ) && canReach.contains(succ)))
                    {
                        canReach.add(b);
                        changed = true;
                        break;
                    }
                }
            }
        }
        return canReach;
    };

    HashSet<IRBlock*> canContinue = computeCanReach(continueSink);
    HashSet<IRBlock*> canExit = computeCanReach(exitSink);

    for (auto b : blocks)
    {
        auto terminator = b->getTerminator();
        if (!as<IRConditionalBranch>(terminator) && !as<IRSwitch>(terminator))
            continue;

        // A fate separator has one successor that leads to continuing the loop and another that
        // leads out of it (and cannot itself continue).
        bool anyContinues = false;
        bool anyOnlyExits = false;
        for (auto succ : b->getSuccessors())
        {
            bool succContinues = succ == continueSink || canContinue.contains(succ);
            bool succExits = succ == exitSink || canExit.contains(succ);
            if (succContinues)
                anyContinues = true;
            else if (succExits)
                anyOnlyExits = true;
        }
        if (anyContinues && anyOnlyExits)
            outSeparators.add(b);
    }
}

// Unroll loop up to a predefined maximum number of iterations.
// Returns true if we can statically determine that the loop terminated within the iteration limit.
// This operation assumes the loop does not have `continue` jumps, i.e. continueBlock ==
// targetBlock.
//
// `allowCheapBail` enables an early exit for a loop that cannot converge yet: if a peeled iteration
// still branches back into the loop but none of the loop's own exit tests folded to a constant this
// peel, peeling continues no further and the residual loop is left in place. That inference is only
// an approximation — a loop whose exit test depends on loop-carried state that itself converges
// (e.g. `while (current) { current = next; next = false; }` stops after two iterations) would be
// abandoned prematurely. It is therefore enabled ONLY on the non-diagnosing deferral pass, where a
// premature bail merely defers the loop to the diagnosing pass and changes no observable outcome.
// The diagnosing pass passes `false`, so it performs the honest full peel up to the iteration limit
// before it reports a loop as non-terminating.
static bool _unrollLoop(
    TargetProgram* targetProgram,
    IRModule* module,
    IRLoop* loopInst,
    List<IRBlock*>& blocks,
    bool allowCheapBail)
{
    if (blocks.getCount() == 0)
    {
        IRBuilder subBuilder(module);
        subBuilder.setInsertBefore(loopInst);
        subBuilder.emitBranch(loopInst->getBreakBlock());
        loopInst->removeAndDeallocate();
        return true;
    }

    auto maxIterations = _getLoopMaxIterationsToUnroll(loopInst);
    if (maxIterations < 0)
        return true;

    // If the loop contains any induction variables (phi params in the header block)
    // that are used outside of the loop, we need to make sure these uses are referencing
    // the up-to-date value after unrolling the loop.
    // The simplest way to achieve this is to create a duplicate `Var` before the loop inst
    // and copy the value of the param to the new var at start of each iteration.
    // Then replace all outside uses to loads from the new var instead.
    //
    allocVarForLoopInductionPhiParam(module, loopInst, blocks);

    // We assume all `continue`s are eliminated and turned into multi-level breaks
    // before this operation.
    SLANG_RELEASE_ASSERT(loopInst->getContinueBlock() == loopInst->getTargetBlock());

    // Insert an outer breakable region so we have a break label to use as the target for
    // any `break` jumps in the unrolled loop.
    // Transform CFG from [..., loopInst] -> [loopTarget] ->... [originalLoopBreakBlock]
    // Into: [..., loop] -> [outerBreakableRegionHeader, loopInst(phi_arg)] -> [(phi_param)
    // loopTarget] -> ... ->
    //       [newLoopBreakBlock] -> [originalLoopBreakBlock/outerBreakableRegionBreakBlock]
    // After this transform, the original break block of the loop will serve as the break block for
    // the outer breakable region.

    IRBuilder builder(module);

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
        // Note that the breakable region header will never have any phi params because there will
        // never be back jumps into the header (it is a single iteration loop just for the break
        // label).

        builder.setInsertBefore(loopInst);
        builder.emitLoop(
            firstIterationLoopHeader,
            firstIterationBreakBlock,
            firstIterationLoopHeader);

        // The `firstIterationBreakBlock` is supposed to act as the `targetBlock` for the back-jump
        // in the loop body. Therefore, if the original loop target block has any phi params, we
        // will need the same set of phi params in `firstIterationBreakBlock` so keep those branches
        // valid.

        builder.setInsertInto(firstIterationBreakBlock);
        {
            IRCloneEnv paramCloneEnv;
            ShortList<IRInst*> newParams;
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
                (UInt)newParams.getCount(),
                newParams.getArrayView().getBuffer()));

            // Carry the original loop's decorations (notably `[ForceUnroll]`) and source
            // location onto the residual loop. If unrolling stops early with iterations still
            // remaining, the leftover loop must still be recognizable as one to unroll on a
            // later pass (the decoration) and must still point at the user's loop when that pass
            // diagnoses it (the source location) — `emitLoop` carries neither on its own.
            newLoopInst->sourceLoc = loopInst->sourceLoc;
            loopInst->transferDecorationsTo(newLoopInst);
            loopInst->removeAndDeallocate();

            // Preserve the `[ForceUnroll]` iteration budget across a split unroll. This call was
            // granted `maxIterations` peel attempts and has consumed `attempedIterations + 1` of
            // them by peeling this iteration; the remainder belongs to the residual. Record it in a
            // second decoration operand so a later `_unrollLoop` call resumes the same budget
            // rather than restarting from the user's full count (which would let a
            // `[ForceUnroll(N)]` loop run more than N iterations when unrolling is deferred). The
            // remainder is stored in its own operand — not by rewriting the user count in operand 0
            // — because operand 0's `0` means "unlimited", which cannot encode a remaining budget
            // of "none".
            if (auto residualForceUnroll = newLoopInst->findDecoration<IRForceUnrollDecoration>())
            {
                IRIntegerValue userCount = 0;
                if (auto c = as<IRIntLit>(residualForceUnroll->getOperand(0)))
                    userCount = c->getValue();
                residualForceUnroll->removeAndDeallocate();

                int remainingAttempts = maxIterations - (attempedIterations + 1);
                if (remainingAttempts < 0)
                    remainingAttempts = 0;

                builder.addDecoration(
                    newLoopInst,
                    kIROp_ForceUnrollDecoration,
                    builder.getIntValue(builder.getIntType(), userCount),
                    builder.getIntValue(builder.getIntType(), remainingAttempts));
            }

            // Update `loopInst` to represent the remaining loop iterations that are yet to be
            // unrolled.
            loopInst = newLoopInst;
        }

        // With the break region set up and wired, we can now clone the loop body into the break
        // region. We create all the blocks first, and setup the clone mapping for the blocks so
        // when we clone the insts later, the branch targets will automatically set to their clones.

        List<IRBlock*> clonedBlocks;
        for (auto b : blocks)
        {
            builder.setInsertBefore(firstIterationBreakBlock);
            auto clonedBlock = builder.createBlock();
            clonedBlock->insertBefore(firstIterationBreakBlock);
            cloneEnv.mapOldValToNew.addIfNotExists(b, clonedBlock);
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

        // Before folding, find this loop's exit tests in the freshly cloned iteration: the
        // blocks that decide between continuing the loop (reaching `firstIterationBreakBlock`)
        // and leaving it (reaching `outerBreakableRegionBreakBlock`). We look now, before
        // `_foldAndSimplifyLoopIteration` may collapse the CFG, so the tests are found
        // structurally rather than being confused with control flow that only appears once a
        // constant test folds. Only the deferral pass (`allowCheapBail`) uses these; the
        // diagnosing pass peels fully and never bails, so it skips the analysis.
        HashSet<IRBlock*> preFoldSeparators;
        if (allowCheapBail)
        {
            _collectLoopFateSeparators(
                clonedBlocks,
                firstIterationBreakBlock,
                outerBreakableRegionBreakBlock,
                preFoldSeparators);
        }

        // With all the insts for the first iteration in place, we now iteratively run
        // SCCP and simplification for the cloned blocks, in hope that some
        // conditional jumps can be folded into unconditional jumps.

        _foldAndSimplifyLoopIteration(
            targetProgram,
            builder,
            clonedBlocks,
            firstIterationBreakBlock,
            unreachableBlock);

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

        // On the deferral pass, check whether *any* of this loop's exit tests resolved to a
        // constant this peel: a separator resolved if its block was folded away (no longer among
        // `clonedBlocks`) or its branch became unconditional. If the loop is going to iterate
        // again but none of its exit tests resolved this peel, none is likely to soon — each
        // typically depends on a value still loop-carried at this point (e.g. `for (j = 0; j < i;
        // ++j)` with `i` not yet a literal). Rather than peel up to `kMaxIterationsToAttempt`, stop
        // and leave the loop for the diagnosing pass. This is only an approximation (a loop whose
        // carried state converges could still terminate), which is why it is confined to the
        // non-diagnosing pass: a premature bail here only defers the loop, and the diagnosing pass
        // (`allowCheapBail == false`) peels it fully before concluding it cannot be unrolled. The
        // residual loop keeps its `[ForceUnroll]` decoration and source location so that pass can
        // find and retry or diagnose it.
        if (allowCheapBail && hasJumpsToRemainingLoop)
        {
            bool anySeparatorResolved = preFoldSeparators.getCount() == 0;
            HashSet<IRBlock*> survivingBlocks;
            for (auto b : clonedBlocks)
                survivingBlocks.add(b);
            for (auto sep : preFoldSeparators)
            {
                if (!survivingBlocks.contains(sep))
                {
                    anySeparatorResolved = true; // Folded away entirely.
                    break;
                }
                auto t = sep->getTerminator();
                if (!as<IRConditionalBranch>(t) && !as<IRSwitch>(t))
                {
                    anySeparatorResolved = true; // Folded to an unconditional branch.
                    break;
                }
            }
            if (!anySeparatorResolved)
                break;
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
                blockSet.add(block);
            }
            for (auto block : blocks)
            {
                for (auto inst : block->getChildren())
                {
                    IRInst* newInst = nullptr;
                    if (!cloneEnv.mapOldValToNew.tryGetValue(inst, newInst))
                        continue;
                    for (auto use = inst->firstUse; use;)
                    {
                        auto nextUse = use->nextUse;
                        if (!blockSet.contains(as<IRBlock>(use->getUser()->getParent())))
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
            builder.emitUnreachable();

            break;
        }
    }

    return loopTerminated;
}

// Visits all loop insts in a func, inner loop first.
template<typename TFunc>
List<IRLoop*> collectLoopsInFunc(IRGlobalValueWithCode* func, const TFunc& filter)
{
    List<IRLoop*> loops;

    // Post order processing allows us to process inner loops first.
    auto postOrder = getPostorder(func);

    for (auto block : postOrder)
    {
        if (auto loop = as<IRLoop>(block->getTerminator()))
        {
            if (filter(loop))
            {
                loops.add(loop);
            }
        }
    }
    return loops;
}

// Collect every `[ForceUnroll]` loop in `func`, outermost first — the reverse of the
// inner-first order `collectLoopsInFunc` produces. Reverse-postorder visits an enclosing
// loop's entry block before the blocks it dominates, so an outer loop's `IRLoop` is seen
// before the inner loops nested in it.
static List<IRLoop*> collectForceUnrollLoopsOutermostFirst(IRGlobalValueWithCode* func)
{
    List<IRLoop*> loops;
    for (auto block : getReversePostorder(func))
    {
        if (auto loop = as<IRLoop>(block->getTerminator()))
        {
            if (loop->findDecoration<IRForceUnrollDecoration>())
                loops.add(loop);
        }
    }
    return loops;
}

// Unroll all `[ForceUnroll]` loops in a function, in two passes.
//
// A `[ForceUnroll]` loop whose trip count depends on an enclosing `[ForceUnroll]` loop's
// induction variable cannot be unrolled on its own: while the enclosing loop is still rolled,
// that variable is loop-carried, so the inner loop's bound never folds to a constant. Consider
// shader-slang/slang#12473:
//
//     [ForceUnroll] for (int i = 0; i < 8; ++i)
//         [ForceUnroll] for (int j = 0; j < i; ++j)
//             dst[i*8+j] = src[i*8+j];
//
// If the `j` loop is attempted first (inner-first, the natural order), `j < i` never folds and
// `_unrollLoop` bails without unrolling it. Once the `i` loop is unrolled, each cloned body has
// `i` substituted by a per-iteration literal, so the copies of the `j` loop then unroll.
//
//  - Pass 1 unrolls what it can, inner-first, and does NOT diagnose a loop it fails to unroll —
//    it simply leaves it. It runs `_unrollLoop` with `allowCheapBail`, so a loop that has not
//    resolved an exit test after one peel is left for pass 2 rather than peeled all 4096 times.
//    A premature bail here is harmless: pass 1 emits no diagnostic, so at worst the loop is
//    deferred to pass 2, which decides its fate honestly.
//  - Pass 2 unrolls outermost-first and DOES diagnose, with `allowCheapBail` off so it peels a
//    loop fully (up to the iteration limit) before concluding it cannot terminate. By the time it
//    reaches a loop, every enclosing `[ForceUnroll]` loop has already been unrolled (or itself
//    diagnosed and aborted), so a loop that still will not unroll here genuinely cannot be, and its
//    failure is reported. Because pass 2 aborts on the first failure and each loop is visited once,
//    "diagnose exactly once" holds without any per-loop bookkeeping.
bool unrollLoopsInFunc(
    TargetProgram* targetProgram,
    IRModule* module,
    IRGlobalValueWithCode* func,
    DiagnosticSink* sink,
    bool* outChanged)
{
    // Nothing to do if this function has no `[ForceUnroll]` loops. Unrolling only ever clones
    // existing `[ForceUnroll]` loops (it never introduces the first one), so both passes are
    // no-ops here. Returning early also avoids running `sortBlocksInFunc` on a function we did not
    // touch, which would needlessly reorder its blocks and perturb otherwise-unrelated emit.
    auto pass1Loops = collectLoopsInFunc(
        func,
        [](IRLoop* l) { return l->findDecoration<IRForceUnrollDecoration>() != nullptr; });
    if (pass1Loops.getCount() == 0)
        return true;

    // Pass 1: inner-first, best-effort, no diagnostics. A loop that cannot be unrolled yet is
    // left for pass 2 (which runs after its enclosing loops have been unrolled).
    for (auto loop : pass1Loops)
    {
        if (!loop->parent)
            continue;

        eliminateContinueBlocks(module, loop);

        auto blocks = collectBlocksInRegion(func, loop);
        if (_unrollLoop(targetProgram, module, loop, blocks, /*allowCheapBail:*/ true))
        {
            if (outChanged)
                *outChanged = true;

            // Simplify before attempting an enclosing loop, so its body is as folded as
            // possible when it is cloned.
            simplifyCFG(func, CFGSimplificationOptions::getDefault());
            eliminateDeadCode(func);
        }
    }

    // Pass 2: outermost-first, diagnosing. Re-collect from the current CFG after each unroll
    // rather than iterating a once-computed list, because unrolling an enclosing loop clones the
    // loops nested in it — those clones are new `[ForceUnroll]` loops that a stale list would
    // miss, and the originals are deallocated. Each round unrolls the outermost remaining loop
    // (so a loop's enclosing loops are always unrolled before it) and stops once none remain.
    for (;;)
    {
        auto loops = collectForceUnrollLoopsOutermostFirst(func);
        IRLoop* loopToUnroll = nullptr;
        for (auto loop : loops)
        {
            if (loop->parent)
            {
                loopToUnroll = loop;
                break;
            }
        }
        if (!loopToUnroll)
            break;

        eliminateContinueBlocks(module, loopToUnroll);

        auto blocks = collectBlocksInRegion(func, loopToUnroll);

        // Capture the location before unrolling: `_unrollLoop` deallocates the loop (replacing it
        // with a residual loop) on its first peel even when it later bails, so the pointer is
        // dangling by the time we would diagnose.
        auto loopLoc = loopToUnroll->sourceLoc;
        if (!_unrollLoop(targetProgram, module, loopToUnroll, blocks, /*allowCheapBail:*/ false))
        {
            if (sink)
                sink->diagnose(Diagnostics::CannotUnrollLoop{.location = loopLoc});
            return false;
        }

        if (outChanged)
            *outChanged = true;

        simplifyCFG(func, CFGSimplificationOptions::getDefault());
        eliminateDeadCode(func);
    }

    sortBlocksInFunc(func);
    return true;
}

bool unrollLoopsInModule(
    IRModule* module,
    TargetProgram* target,
    DiagnosticSink* sink,
    bool* outChanged)
{
    SLANG_PROFILE;

    for (auto inst : module->getGlobalInsts())
    {
        if (as<IRGeneric>(inst))
            continue;

        if (auto func = as<IRGlobalValueWithCode>(inst))
        {
            bool result = unrollLoopsInFunc(target, module, func, sink, outChanged);
            if (!result)
                return false;
        }
    }
    return true;
}

void eliminateContinueBlocks(IRModule* module, IRLoop* loopInst)
{
    // Eliminate the continue jumps by turning a loop in the form of:
    //   for (;;)
    //   {
    //       <loop body>
    //   continueBlock:
    //       <continuePart>
    //   }
    // into:
    //   for (;;) // original loop
    //   {
    //      for(;;) // breakableRegionHeader
    //      {
    //         <loop body>
    //      }
    //   breakableRegionBreakBlock:
    //      <continuePart>
    //   }
    //  where a continue is replaced with a "break" into breakableRegionBreakBlock.
    //

    auto continueBlock = loopInst->getContinueBlock();

    if (continueBlock == loopInst->getTargetBlock())
        return;

    // If the continue block is not reachable, remove it.
    if (continueBlock && !continueBlock->hasMoreThanOneUse())
    {
        loopInst->continueBlock.set(loopInst->getTargetBlock());
        continueBlock->removeAndDeallocate();
        return;
    }

    // We have determined that there is really a non-trivial continue block in the loop body,
    // we will now introduce a breakable region for each iteration.

    IRBuilder builder(module);
    IRBuilderSourceLocRAII sourceLocationScope(&builder, loopInst->sourceLoc);

    auto targetBlock = loopInst->getTargetBlock();

    auto innerBreakableRegionHeader = builder.createBlock();
    innerBreakableRegionHeader->insertBefore(targetBlock);

    auto innerBreakableRegionBreakBlock = builder.createBlock();
    innerBreakableRegionBreakBlock->insertBefore(continueBlock);

    loopInst->block.set(innerBreakableRegionHeader);
    loopInst->continueBlock.set(innerBreakableRegionHeader);

    targetBlock->replaceUsesWith(innerBreakableRegionHeader);

    // Move decorations and params from original targetBlock to innerBreakableRegionHeader.
    moveParams(innerBreakableRegionHeader, targetBlock);

    builder.setInsertInto(innerBreakableRegionHeader);
    builder.emitLoop(targetBlock, innerBreakableRegionBreakBlock, targetBlock);

    continueBlock->replaceUsesWith(innerBreakableRegionBreakBlock);

    builder.setInsertInto(innerBreakableRegionBreakBlock);
    moveParams(innerBreakableRegionBreakBlock, continueBlock);
    builder.emitBranch(continueBlock);

    // If the original loop can be executed up to N times, the new loop may be executed
    // upto N+1 times (although most insts are skipped in the last traversal)
    //
    if (auto maxItersDecoration = loopInst->findDecoration<IRLoopMaxItersDecoration>())
    {
        auto maxIters = maxItersDecoration->getMaxIters();
        maxItersDecoration->removeAndDeallocate();
        builder.addLoopMaxItersDecoration(loopInst, maxIters + 1);
    }
}

void eliminateContinueBlocksInFunc(IRModule* module, IRGlobalValueWithCode* func)
{
    List<IRLoop*> loops = collectLoopsInFunc(
        func,
        [](IRLoop* l) { return l->getContinueBlock() != l->getTargetBlock(); });

    if (loops.getCount() == 0)
        return;

    for (auto loop : loops)
    {
        eliminateContinueBlocks(module, loop);
    }
}

} // namespace Slang
