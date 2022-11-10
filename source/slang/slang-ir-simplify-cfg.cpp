#include "slang-ir-simplify-cfg.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-ir-restructure.h"

namespace Slang
{

BreakableRegion* findBreakableRegion(Region* region)
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

bool isTrivialSingleIterationLoop(IRGlobalValueWithCode* func, IRLoop* loop, RefPtr<RegionTree>& regionTree)
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

    if (!regionTree)
        regionTree = generateRegionTreeForFunc(func, nullptr);

    SimpleRegion* targetBlockRegion = nullptr;
    if (!regionTree->mapBlockToRegion.TryGetValue(targetBlock, targetBlockRegion))
        return false;
    BreakableRegion* loopBreakableRegion = findBreakableRegion(targetBlockRegion);
    LoopRegion* loopRegion = as<LoopRegion>(loopBreakableRegion);
    if (!loopRegion)
        return false;

    for (auto block : func->getBlocks())
    {
        SimpleRegion* region = nullptr;
        if (!regionTree->mapBlockToRegion.TryGetValue(block, region))
            return false;
        if (!region->isDescendentOf(loopRegion))
            continue;
        for (auto branchTarget : block->getSuccessors())
        {
            SimpleRegion* targetRegion = nullptr;
            if (!regionTree->mapBlockToRegion.TryGetValue(branchTarget, targetRegion))
                return false;
            // If multi-level break out that skips over this loop exists, then this is not a trivial loop.
            if (targetRegion->isDescendentOf(loopRegion))
                continue;
            if (targetBlock != loop->getBreakBlock())
                return false;
            if (findBreakableRegion(targetRegion) != loopRegion)
            {
                // If the break is initiated from a nested region, this is not trivial.
                return false;
            }
        }
    }

    return true;
}

bool processFunc(IRGlobalValueWithCode* func)
{
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return false;

    // Lazily generated region tree.
    RefPtr<RegionTree> regionTree = nullptr;

    SharedIRBuilder sharedBuilder(func->getModule());
    IRBuilder builder(&sharedBuilder);

    bool changed = false;

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
                // break at the end of the loop out of it, we can remove the header and turn it into
                // a normal branch.
                auto targetBlock = loop->getTargetBlock();
                if (isTrivialSingleIterationLoop(func, loop, regionTree))
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
    return changed;
}

bool simplifyCFG(IRModule* module)
{
    bool changed = false;
    for (auto inst : module->getGlobalInsts())
    {
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
