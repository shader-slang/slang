// slang-ir-reachability.h
#pragma once

#include "slang-ir.h"

namespace Slang
{

// A context for computing and caching reachability between blocks on the CFG.
struct ReachabilityContext
{
    Dictionary<IRBlock*, int> mapBlockToId;
    List<IRBlock*> allBlocks;
    List<UIntSet> sourceBlocks; // sourcesBlocks[i] stores the set of blocks from which block i can be reached.

    ReachabilityContext() = default;
    ReachabilityContext(IRGlobalValueWithCode* code);

    bool isBlockReachable(IRBlock* from, IRBlock* to);

    bool isInstReachable(IRInst* inst1, IRInst* inst2)
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

        return isBlockReachable(as<IRBlock>(inst1->getParent()), as<IRBlock>(inst2->getParent()));
    }
};

}
