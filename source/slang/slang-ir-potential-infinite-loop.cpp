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
#include "slang-ir-sccp.h" // Added for tryConstantFoldInst
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// Helper to check if an IRInst is a boolean constant, and its value.
// Returns true if it's a bool constant, and sets outVal.
// Returns false otherwise.
// Note: This relies on prior IR passes (like constant folding) to simplify
// conditions like cmp(const, const) into an IRBoolLit.
static bool getConstantBoolValue(IRModule* module, IRInst* inst, bool& outVal)
{
    if (!inst)
        return false;

    // Attempt to fold the instruction to a constant.
    // tryConstantFoldInst may replace 'inst' with the folded version in the IR.
    IRInst* foldedInst = tryConstantFoldInst(module, inst);

    if (auto boolLit = as<IRBoolLit>(foldedInst))
    {
        outVal = boolLit->getValue();
        return true;
    }
    return false;
}

/// Check if a loop body has any path that can reach the break block
static bool canLoopExit(IRLoop* loop)
{
    IRBlock* targetBlock = loop->getTargetBlock();
    IRBlock* breakBlock = loop->getBreakBlock();
    IRBlock* continueBlock = loop->getContinueBlock(); // Used later for DFS
    IRModule* module = loop->getModule();

    // If there's no break block, the loop cannot exit
    if (!breakBlock)
    {
        return false;
    }

    // Analyze the terminator of the loop's target block (header)
    if (targetBlock)
    {
        IRInst* terminator = targetBlock->getTerminator();
        if (!terminator)
        {
            // If targetBlock has no terminator, it's unusual but might mean it falls through.
            // The DFS might handle it, or it's an ill-formed loop.
            // For safety, if it's non-null and has no terminator, assume it might not exit.
            // However, a well-formed targetBlock for a loop should have a terminator.
            // If it doesn't, it might fall through to the next block in sequence,
            // which is not standard for loop structures. Let DFS try to figure it out.
        }
        else if (auto ub = as<IRUnconditionalBranch>(terminator))
        {
            // If the loop header unconditionally branches to the break block, it can exit.
            if (ub->getTargetBlock() == breakBlock)
            {
                return true;
            }
            // Otherwise, it unconditionally goes somewhere else (e.g., loop body).
            // The DFS will determine if that path can reach breakBlock.
        }
        else if (auto condBranch = as<IRConditionalBranch>(terminator))
        {
            IRInst* conditionInst = condBranch->getCondition();
            bool constantConditionValue;

            if (getConstantBoolValue(module, conditionInst, constantConditionValue))
            {
                // The loop condition is a compile-time boolean constant.
                IRBlock* alwaysTakenBlock = constantConditionValue ? condBranch->getTrueBlock()
                                                                   : condBranch->getFalseBlock();
                IRBlock* neverTakenBlock = constantConditionValue ? condBranch->getFalseBlock()
                                                                  : condBranch->getTrueBlock();

                if (alwaysTakenBlock == breakBlock)
                {
                    // The constant condition forces the loop to take a path that is the break
                    // block.
                    return true;
                }

                if (neverTakenBlock == breakBlock)
                {
                    // The constant condition forces the loop *away* from the break block path.
                    // Thus, this loop cannot exit via this conditional branch mechanism.
                    return false;
                }
                // If neither the alwaysTakenBlock nor the neverTakenBlock is the breakBlock,
                // the constant condition doesn't immediately guarantee an exit or an infinite loop
                // based solely on this branch's direct targets.
                // We fall through to the DFS to see if alwaysTakenBlock can reach breakBlock.
            }
            // If condition is not constant, fall through to DFS.
        }
        // Other terminator types (e.g., switch, return within targetBlock itself)
        // will be handled by the DFS. Note that a 'return' in targetBlock is an exit.
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
    enqueue(continueBlock); // Include continue block path if present

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
                sink->diagnose(loop, Diagnostics::potentialInfiniteLoop);
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
