#include "slang-ir-reachability.h"

namespace Slang
{
// Computes whether block1 can reach block2.
// A block is considered not reachable from itself unless there is a backedge in the CFG.

bool ReachabilityContext::computeReachability(IRBlock* block1, IRBlock* block2)
{
    workList.clear();
    reachableBlocks.Clear();
    workList.add(block1);
    for (Index i = 0; i < workList.getCount(); i++)
    {
        auto src = workList[i];
        for (auto successor : src->getSuccessors())
        {
            if (successor == block2)
                return true;
            if (reachableBlocks.Add(successor))
                workList.add(successor);
        }
    }
    return false;
}

bool ReachabilityContext::isBlockReachable(IRBlock* from, IRBlock* to)
{
    BlockPair pair;
    pair.first = from;
    pair.second = to;
    bool result = false;
    if (reachabilityResults.TryGetValue(pair, result))
        return result;
    result = computeReachability(from, to);
    reachabilityResults[pair] = result;
    return result;
}
}
