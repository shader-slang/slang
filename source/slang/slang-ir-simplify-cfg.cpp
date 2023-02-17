#include "slang-ir-simplify-cfg.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-ir-dominators.h"
#include "slang-ir-restructure.h"

namespace Slang
{

struct CFGSimplificationContext
{
    RefPtr<RegionTree> regionTree;
    RefPtr<IRDominatorTree> domTree;
};

static BreakableRegion* findBreakableRegion(Region* region)
{
    for (;;)
    {
        if (auto b = as<BreakableRegion>(region))
            return b;
        region = region->getParent();
        if (!region)
            return nullptr;
    }
}

// Test if a loop is trivial: a trivial loop runs for a single iteration without any back edges, and
// there is only one break out of the loop at the very end. The function generates `regionTree` if
// it is needed and hasn't been generated yet.
static bool isTrivialSingleIterationLoop(
    IRGlobalValueWithCode* func,
    IRLoop* loop,
    CFGSimplificationContext& inoutContext)
{
    auto targetBlock = loop->getTargetBlock();
    if (targetBlock->getPredecessors().getCount() != 1) return false;
    if (*targetBlock->getPredecessors().begin() != loop->getParent()) return false;

    int useCount = 0;
    for (auto use = loop->getBreakBlock()->firstUse; use; use = use->nextUse)
    {
        if (use->getUser() == loop)
            continue;
        useCount++;
        if (useCount > 1)
            return false;
    }

    // The loop has passed simple test.
    // 
    // We need to verify this is a trivial loop by checking if there is any multi-level breaks
    // that skips out of this loop.

    if (!inoutContext.domTree)
        inoutContext.domTree = computeDominatorTree(func);
    if (!inoutContext.regionTree)
        inoutContext.regionTree = generateRegionTreeForFunc(func, nullptr);

    SimpleRegion* targetBlockRegion = nullptr;
    if (!inoutContext.regionTree->mapBlockToRegion.TryGetValue(targetBlock, targetBlockRegion))
        return false;
    BreakableRegion* loopBreakableRegion = findBreakableRegion(targetBlockRegion);
    LoopRegion* loopRegion = as<LoopRegion>(loopBreakableRegion);
    if (!loopRegion)
        return false;
    for (auto block : func->getBlocks())
    {
        if (!inoutContext.domTree->dominates(loop->getTargetBlock(), block))
            continue;
        if (inoutContext.domTree->dominates(loop->getBreakBlock(), block))
            continue;
        SimpleRegion* region = nullptr;
        if (!inoutContext.regionTree->mapBlockToRegion.TryGetValue(block, region))
            return false;

        for (auto branchTarget : block->getSuccessors())
        {
            SimpleRegion* targetRegion = nullptr;
            if (!inoutContext.regionTree->mapBlockToRegion.TryGetValue(branchTarget, targetRegion))
                return false;
            // If multi-level break out that skips over this loop exists, then this is not a trivial loop.
            if (targetRegion->isDescendentOf(loopRegion))
                continue;
            if (targetBlock != loop->getBreakBlock())
                return false;
            if (findBreakableRegion(region) != loopRegion)
            {
                // If the break is initiated from a nested region, this is not trivial.
                return false;
            }
        }
    }

    return true;
}

static bool removeDeadBlocks(IRGlobalValueWithCode* func)
{
    bool changed = false;
    List<IRBlock*> workList;
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return false;

    for (auto block = firstBlock->getNextBlock(); block; block = block->getNextBlock())
    {
        workList.add(block);
    }

    HashSet<IRBlock*> workListSet;
    List<IRBlock*> nextWorkList;
    for (;;)
    {
        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto block = workList[i];
            if (!block->hasUses() && as<IRTerminatorInst>(block->getFirstInst()))
            {
                for (auto succ : block->getSuccessors())
                {
                    if (workListSet.Add(succ))
                    {
                        nextWorkList.add(succ);
                    }
                }
                block->removeAndDeallocate();
                changed = true;
            }
        }
        if (nextWorkList.getCount())
        {
            workList = _Move(nextWorkList);
            workListSet.Clear();
        }
        else
        {
            break;
        }
    }
    return changed;
}

static bool processFunc(IRGlobalValueWithCode* func)
{
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return false;

    // Lazily generated region tree.
    CFGSimplificationContext simplificationContext;

    IRBuilder builder(func->getModule());

    bool changed = false;
    for (;;)
    {
        List<IRBlock*> workList;
        HashSet<IRBlock*> processedBlock;
        workList.add(func->getFirstBlock());
        while (workList.getCount())
        {
            auto block = workList.getFirst();
            workList.fastRemoveAt(0);
            while (block)
            {
                if (auto loop = as<IRLoop>(block->getTerminator()))
                {
                    // If continue block is unreachable, remove it.
                    auto continueBlock = loop->getContinueBlock();
                    if (continueBlock && !continueBlock->hasMoreThanOneUse())
                    {
                        loop->continueBlock.set(loop->getTargetBlock());
                        continueBlock->removeAndDeallocate();
                    }

                    // If there isn't any actual back jumps into loop target and there is a trivial
                    // break at the end of the loop, we can remove the header and turn it into
                    // a normal branch.
                    auto targetBlock = loop->getTargetBlock();
                    if (isTrivialSingleIterationLoop(func, loop, simplificationContext))
                    {
                        builder.setInsertBefore(loop);
                        List<IRInst*> args;
                        for (UInt i = 0; i < loop->getArgCount(); i++)
                        {
                            args.add(loop->getArg(i));
                        }
                        builder.emitBranch(targetBlock, args.getCount(), args.getBuffer());
                        loop->removeAndDeallocate();
                    }
                }

                // If `block` does not end with an unconditional branch, bail.
                if (block->getTerminator()->getOp() != kIROp_unconditionalBranch)
                    break;
                auto branch = as<IRUnconditionalBranch>(block->getTerminator());
                auto successor = branch->getTargetBlock();
                // Only perform the merge if `block` is the only predecessor of `successor`.
                // We also need to make sure not to merge a block that serves as the
                // merge point in CFG. Such blocks will have more than one use.
                if (successor->hasMoreThanOneUse())
                    break;
                if (block->hasMoreThanOneUse())
                    break;
                changed = true;
                Index paramIndex = 0;
                auto inst = successor->getFirstDecorationOrChild();
                while (inst)
                {
                    auto next = inst->getNextInst();
                    if (inst->getOp() == kIROp_Param)
                    {
                        inst->replaceUsesWith(branch->getArg(paramIndex));
                        paramIndex++;
                    }
                    else
                    {
                        inst->removeFromParent();
                        inst->insertAtEnd(block);
                    }
                    inst = next;
                }
                branch->removeAndDeallocate();
                assert(!successor->hasUses());
                successor->removeAndDeallocate();
            }
            for (auto successor : block->getSuccessors())
            {
                if (processedBlock.Add(successor))
                {
                    workList.add(successor);
                }
            }
        }
        bool blocksRemoved = removeDeadBlocks(func);
        changed |= blocksRemoved;
        if (!blocksRemoved)
            break;
    }
    return changed;
}

bool simplifyCFG(IRModule* module)
{
    bool changed = false;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto genericInst = as<IRGeneric>(inst))
        {
            inst = findGenericReturnVal(genericInst);
        }
        if (auto func = as<IRFunc>(inst))
        {
            changed |= processFunc(func);
        }
    }
    return changed;
}

bool simplifyCFG(IRGlobalValueWithCode* func)
{
    return processFunc(func);
}

} // namespace Slang
