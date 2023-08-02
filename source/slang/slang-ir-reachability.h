// slang-ir-reachability.h
#pragma once

#include "slang-ir.h"

namespace Slang
{

// A context for computing and caching reachability between blocks on the CFG.
struct ReachabilityContext
{
    struct BlockPair
    {
        IRBlock* first;
        IRBlock* second;
        bool operator == (const BlockPair& other) const
        {
            return first == other.first && second == other.second;
        }
        HashCode getHashCode() const
        {
            Hasher h;
            h.hashValue(first);
            h.hashValue(second);
            return h.getResult();
        }
    };
    Dictionary<BlockPair, bool> reachabilityResults;

    List<IRBlock*> workList;
    HashSet<IRBlock*> reachableBlocks;

    // Computes whether block1 can reach block2.
    // A block is considered not reachable from itself unless there is a backedge in the CFG.
    bool computeReachability(IRBlock* block1, IRBlock* block2);

    bool isBlockReachable(IRBlock* from, IRBlock* to);

    bool isInstReachable(IRInst* inst1, IRInst* inst2)
    {
        if (isBlockReachable(as<IRBlock>(inst1->getParent()), as<IRBlock>(inst2->getParent())))
            return true;

        // If the parent blocks are not reachable, but inst1 and inst2 are in the same block,
        // we test if inst2 appears after inst1.
        if (inst1->getParent() == inst2->getParent())
        {
            for (auto inst = inst1->getNextInst(); inst; inst = inst->getNextInst())
            {
                if (inst == inst2)
                    return true;
            }
        }

        return false;
    }
};

}
