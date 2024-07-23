#include "slang-ir-reachability.h"
#include "slang-ir-insts.h"

namespace Slang
{
// Computes whether block1 can reach block2.
// A block is considered not reachable from itself unless there is a backedge in the CFG.

    ReachabilityContext::ReachabilityContext(IRGlobalValueWithCode* code)
    {
        int id = 0;
        for (auto block : code->getBlocks())
        {
            mapBlockToId[block] = id;
            id++;
            allBlocks.add(block);
        }
        sourceBlocks.setCount(allBlocks.getCount());
        for (auto &srcBlock : sourceBlocks)
            srcBlock.resizeAndClear(allBlocks.getCount());

        if (allBlocks.getCount() == 0)
            return;

        List<IRBlock*> workList;
        List<IRBlock*> pendingWorkList;
        workList.add(allBlocks[0]);
        while (workList.getCount())
        {
            pendingWorkList.clear();
            for (Index i = 0; i < workList.getCount(); i++)
            {
                auto src = workList[i];
                auto srcId = mapBlockToId.getValue(src);
                for (auto successor : src->getSuccessors())
                {
                    auto successorId = mapBlockToId.getValue(successor);
                    auto& blockSet = sourceBlocks[successorId];
                    bool changed = false;
                    if (!blockSet.contains(srcId))
                    {
                        blockSet.add(srcId);
                        changed = true;
                    }
                    if (!blockSet.contains(sourceBlocks[srcId]))
                    {
                        blockSet.unionWith(sourceBlocks[srcId]);
                        changed = true;
                    }
                    if (changed)
                        pendingWorkList.add(successor);
                }
            }
            workList.swapWith(pendingWorkList);
        }

    }
    
    bool ReachabilityContext::isInstReachable(IRInst* inst1, IRInst* inst2)
    {
        // If inst1 and inst2 are in the same block,
        // we test if inst2 appears after inst1.
        if (inst1->getParent() == inst2->getParent())
        {
            for (auto inst = inst1->getNextInst(); inst; inst = inst->getNextInst())
            {
                if (inst == inst2)
                    return true;
            }
        }

        // Special cases

        // Target switches; treat as reachable from its cases
        if (auto tswitch = as<IRTargetSwitch>(inst2))
        {
            for (Slang::UInt i = 0; i < tswitch->getCaseCount(); i++)
            {
                IRBlock* block = tswitch->getCaseBlock(i);
                if (isWithinBlock(block, inst1))
                    return true;
            }
        }

        return isBlockReachable(as<IRBlock>(inst1->getParent()), as<IRBlock>(inst2->getParent()));
    }

    bool ReachabilityContext::isBlockReachable(IRBlock* from, IRBlock* to)
    {
        if (!from) return false;
        if (!to) return false;
        int* fromId = mapBlockToId.tryGetValue(from);
        int* toId = mapBlockToId.tryGetValue(to);
        if (!fromId || !toId)
            return true;
        return sourceBlocks[*toId].contains(*fromId);
    }

    bool ReachabilityContext::isWithinBlock(IRBlock* block, IRInst* tinst)
    {
        for (auto inst = block->getFirstInst(); inst; inst = inst->next)
        {
            if (inst == tinst)
                return true;
        }

        return false;
    }
}
