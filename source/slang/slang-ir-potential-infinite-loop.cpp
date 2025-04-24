// slang-ir-potential-infinite-loop.cpp
#include "slang-ir-potential-infinite-loop.h"

#include "core/slang-basic.h"
#include "core/slang-dictionary.h" // For HashSet used in DFS
#include "core/slang-io.h"
#include "core/slang-list.h" // For List used in DFS
#include "core/slang-type-text-util.h"
#include "slang-compiler.h"
#include "slang-diagnostics.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

/// Check if a loop body has any path that can reach the break block
static bool canLoopExit(IRLoop* loop)
{
    IRBlock* targetBlock = loop->getTargetBlock();
    IRBlock* breakBlock = loop->getBreakBlock();

    // If there's no break block, the loop cannot exit
    if (!breakBlock)
    {
        return false;
    }

    // Fast check: direct branch from header to break block
    for (auto inst : targetBlock->getChildren())
    {
        if (auto branch = as<IRConditionalBranch>(inst))
        {
            if (branch->getTrueBlock() == breakBlock || branch->getFalseBlock() == breakBlock)
                return true;
        }
        else if (auto unconditionalBranch = as<IRUnconditionalBranch>(inst))
        {
            if (unconditionalBranch->getTargetBlock() == breakBlock)
                return true;
        }
    }

    // Comprehensive but bounded DFS: explore blocks reachable from the loop header
    List<IRBlock*> workList;
    HashSet<IRBlock*> visited;

    auto enqueue = [&](IRBlock* b)
    {
        if (b && visited.add(b))
            workList.add(b);
    };

    enqueue(targetBlock);
    enqueue(loop->getContinueBlock()); // Include continue block path if present

    while (workList.getCount())
    {
        IRBlock* curr = workList.getLast();
        workList.removeLast();

        if (curr == breakBlock)
            return true; // Found a path

        IRInst* term = curr->getTerminator();
        if (!term)
            continue;

        // If the terminator has *no* successor blocks (e.g. return, discard, unreachable, etc.)
        // then the loop can exit via this path.
        if (!as<IRConditionalBranch>(term) && !as<IRUnconditionalBranch>(term) &&
            !as<IRSwitch>(term))
        {
            return true;
        }

        if (auto cb = as<IRConditionalBranch>(term))
        {
            enqueue(cb->getTrueBlock());
            enqueue(cb->getFalseBlock());
        }
        else if (auto ub = as<IRUnconditionalBranch>(term))
        {
            enqueue(ub->getTargetBlock());
        }
        else if (auto sw = as<IRSwitch>(term))
        {
            enqueue(sw->getDefaultLabel());
            for (UInt i = 0; i < sw->getCaseCount(); ++i)
                enqueue(sw->getCaseLabel(i));
        }
        // Other terminators (return, discard, unreachable, etc.) are exits from the loop body
    }

    // If we exhaust reachable blocks without hitting the break block, assume the loop cannot exit
    return false;
}

/// Get source location from the loop for diagnostics
static SourceLoc getLoopSourceLocation(IRLoop* loop)
{
    // Try to get source location from the loop itself
    if (loop->sourceLoc.isValid())
        return loop->sourceLoc;

    // Try to get from the target block
    IRBlock* targetBlock = loop->getTargetBlock();
    if (targetBlock && targetBlock->sourceLoc.isValid())
        return targetBlock->sourceLoc;

    // Try to get from instructions in the target block
    if (targetBlock)
    {
        for (auto inst : targetBlock->getOrdinaryInsts())
        {
            if (inst->sourceLoc.isValid())
                return inst->sourceLoc;
        }
    }

    return SourceLoc();
}

static void checkForPotentialInfiniteLoopsInInst(
    IRInst* inst,
    DiagnosticSink* sink,
    bool diagnoseWarning)
{
    // Check if this instruction is a loop
    if (auto loop = as<IRLoop>(inst))
    {
        // Check if this loop has any path that can exit
        bool canExit = canLoopExit(loop);
        if (!canExit)
        {
            if (diagnoseWarning)
            {
                SourceLoc loc = getLoopSourceLocation(loop);
                sink->diagnose(loc, Diagnostics::potentialInfiniteLoop);
            }
        }
    }

    // Recursively check children
    for (auto childInst : inst->getChildren())
    {
        checkForPotentialInfiniteLoopsInInst(childInst, sink, diagnoseWarning);
    }
}

void checkForPotentialInfiniteLoops(IRModule* module, DiagnosticSink* sink, bool diagnoseWarning)
{
    // Look for loops in the module
    checkForPotentialInfiniteLoopsInInst(module->getModuleInst(), sink, diagnoseWarning);
}

} // namespace Slang
