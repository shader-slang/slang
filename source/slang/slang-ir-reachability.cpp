#include "slang-ir-reachability.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
// Computes whether block1 can reach block2.
// A block is considered not reachable from itself unless there is a backedge in the CFG.
ReachabilityContext::ReachabilityContext(IRGlobalValueWithCode* code)
{
    int id = 0;
    for (auto block : code->getBlocks())
    {
        mapBlockToId[block] = id++;
        allBlocks.add(block);
    }

    sourceBlocks.setCount(allBlocks.getCount());
    for (auto& srcBlock : sourceBlocks)
        srcBlock.resizeAndUnsetAll(allBlocks.getCount());

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

bool ReachabilityContext::isInstReachable(IRInst* from, IRInst* to)
{
    // If inst1 and inst2 are in the same block,
    // we test if inst2 appears after inst1.
    auto fromBlock = getBlock(from);
    auto toBlock = getBlock(to);
    if (fromBlock == toBlock)
    {
        // `to` may be a nested child instruction (e.g. inside a SPIRVAsm block).
        // Walk up to find the direct child of the block that is an ancestor of `to`,
        // so the sibling scan can find it.
        IRInst* toInBlock = to;
        while (toInBlock->getParent() != fromBlock)
            toInBlock = toInBlock->getParent();

        for (auto inst = from->getNextInst(); inst; inst = inst->getNextInst())
        {
            if (inst == toInBlock)
                return true;
        }
    }

    return isBlockReachable(fromBlock, toBlock);
}

bool ReachabilityContext::isBlockReachable(IRBlock* from, IRBlock* to)
{
    if (!from)
        return false;

    if (!to)
        return false;

    int* fromId = mapBlockToId.tryGetValue(from);
    int* toId = mapBlockToId.tryGetValue(to);
    if (!fromId || !toId)
        return true;

    return sourceBlocks[*toId].contains(*fromId);
}
} // namespace Slang
