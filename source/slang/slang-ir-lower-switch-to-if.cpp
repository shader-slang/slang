// slang-ir-lower-switch-to-if.cpp
//
// Lowers switch statements to if-else chains wrapped in do-while(false).
// This provides deterministic reconvergence behavior for switches with
// non-trivial fallthrough.
//
// See GitHub issue #6441 and docs/design/switch-to-if-lowering-plan.md

#include "slang-ir-lower-switch-to-if.h"

#include "slang-ir-eliminate-phis.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// Forward declarations
static bool shouldLowerSwitchToIf(IRSwitch* switchInst, TargetProgram* targetProgram);

/// Information about a case group in the switch.
/// Multiple case values can share the same label (e.g., case 1: case 2: body).
struct CaseInfo
{
    /// All case values that map to this label
    List<IRInst*> values;

    /// The target block for this case
    IRBlock* label = nullptr;

    /// True if this is the default case
    bool isDefault = false;

    /// True if this case falls through to the next (doesn't end with break)
    bool fallsThrough = false;

    /// The predicate for entering this case directly (selector matches one of values)
    IRInst* directPredicate = nullptr;

    /// The cumulative predicate (direct OR any predecessor that falls through to here)
    IRInst* reachPredicate = nullptr;
};

/// Context for lowering a single switch statement.
struct SwitchLoweringContext
{
    IRSwitch* switchInst;
    IRBuilder* builder;
    TargetProgram* targetProgram;

    /// Cases in source order (order they appear in the IR)
    List<CaseInfo> cases;

    /// Index of the default case in the cases list (-1 if no default)
    Index defaultIndex = -1;

    /// Collect case information from the switch instruction.
    void collectCases()
    {
        auto breakLabel = switchInst->getBreakLabel();
        auto defaultLabel = switchInst->getDefaultLabel();

        // First, collect all unique case labels and their values
        Dictionary<IRBlock*, Index> labelToIndex;

        // Process cases in the order they appear in the switch operands
        UInt caseCount = switchInst->getCaseCount();
        for (UInt i = 0; i < caseCount; i++)
        {
            auto caseValue = switchInst->getCaseValue(i);
            auto caseLabel = switchInst->getCaseLabel(i);

            Index existingIndex;
            if (labelToIndex.tryGetValue(caseLabel, existingIndex))
            {
                // This label already exists, add the value to it
                cases[existingIndex].values.add(caseValue);
            }
            else
            {
                // New label
                CaseInfo info;
                info.values.add(caseValue);
                info.label = caseLabel;
                info.isDefault = (caseLabel == defaultLabel);

                Index newIndex = cases.getCount();
                cases.add(info);
                labelToIndex[caseLabel] = newIndex;

                if (info.isDefault)
                    defaultIndex = newIndex;
            }
        }

        // If default wasn't already added (can happen if default has no explicit case values)
        if (defaultLabel && defaultIndex < 0)
        {
            // Check if default is already in our list (shared with a case value)
            Index existingIndex;
            if (labelToIndex.tryGetValue(defaultLabel, existingIndex))
            {
                cases[existingIndex].isDefault = true;
                defaultIndex = existingIndex;
            }
            else
            {
                CaseInfo info;
                info.label = defaultLabel;
                info.isDefault = true;
                defaultIndex = cases.getCount();
                cases.add(info);
                labelToIndex[defaultLabel] = defaultIndex;
            }
        }

        // Sort cases by their block order in the function to get source order
        // This is important for determining fallthrough relationships
        sortCasesByBlockOrder();

        // Determine which cases fall through
        for (Index i = 0; i < cases.getCount(); i++)
        {
            if (i + 1 < cases.getCount())
            {
                HashSet<IRBlock*> visited;
                cases[i].fallsThrough = caseBlocksFallThroughTo(
                    cases[i].label, cases[i + 1].label, breakLabel, visited);
            }
            else
            {
                // Last case can't fall through
                cases[i].fallsThrough = false;
            }
        }
    }

    /// Sort cases by their block order in the parent function.
    void sortCasesByBlockOrder()
    {
        if (cases.getCount() <= 1)
            return;

        // Build a map from block to its order in the function
        Dictionary<IRBlock*, Index> blockOrder;
        auto parentBlock = as<IRBlock>(switchInst->getParent());
        auto func = as<IRGlobalValueWithCode>(parentBlock->getParent());
        if (!func)
            return;

        Index order = 0;
        for (auto block : func->getBlocks())
        {
            blockOrder[block] = order++;
        }

        // Sort cases by block order
        cases.sort([&](const CaseInfo& a, const CaseInfo& b) {
            Index orderA = 0, orderB = 0;
            blockOrder.tryGetValue(a.label, orderA);
            blockOrder.tryGetValue(b.label, orderB);
            return orderA < orderB;
        });

        // Update defaultIndex after sorting
        for (Index i = 0; i < cases.getCount(); i++)
        {
            if (cases[i].isDefault)
            {
                defaultIndex = i;
                break;
            }
        }
    }

    /// Check if a case block falls through to target.
    static bool caseBlocksFallThroughTo(
        IRBlock* caseLabel,
        IRBlock* targetBlock,
        IRBlock* breakLabel,
        HashSet<IRBlock*>& visited)
    {
        if (!caseLabel || !targetBlock)
            return false;

        if (visited.contains(caseLabel))
            return false;
        visited.add(caseLabel);

        if (caseLabel == breakLabel)
            return false;

        if (caseLabel == targetBlock)
            return true;

        auto terminator = caseLabel->getTerminator();
        if (!terminator)
            return false;

        switch (terminator->getOp())
        {
        case kIROp_UnconditionalBranch:
            {
                auto branch = as<IRUnconditionalBranch>(terminator);
                auto target = branch->getTargetBlock();
                if (target == targetBlock)
                    return true;
                if (target == breakLabel)
                    return false;
                return caseBlocksFallThroughTo(target, targetBlock, breakLabel, visited);
            }
        case kIROp_ConditionalBranch:
            {
                auto branch = as<IRConditionalBranch>(terminator);
                if (caseBlocksFallThroughTo(
                        branch->getTrueBlock(), targetBlock, breakLabel, visited))
                    return true;
                if (caseBlocksFallThroughTo(
                        branch->getFalseBlock(), targetBlock, breakLabel, visited))
                    return true;
                return false;
            }
        case kIROp_Switch:
        case kIROp_Loop:
            return false;
        default:
            return false;
        }
    }

    /// Build predicates for each case.
    void buildPredicates()
    {
        auto selector = switchInst->getCondition();
        auto boolType = builder->getBoolType();

        // Build direct predicates for each case
        for (auto& caseInfo : cases)
        {
            if (caseInfo.isDefault)
            {
                // Default predicate is the inverse of all cases AFTER the default
                // (Cases before default either break or fall through, both handled correctly)
                IRInst* afterDefaultPred = nullptr;

                for (Index i = defaultIndex + 1; i < cases.getCount(); i++)
                {
                    if (cases[i].isDefault)
                        continue;

                    // Build predicate for this case
                    IRInst* casePred = nullptr;
                    for (auto value : cases[i].values)
                    {
                        auto eq = builder->emitEql(selector, value);
                        if (casePred)
                            casePred = builder->emitOr(boolType, casePred, eq);
                        else
                            casePred = eq;
                    }

                    if (casePred)
                    {
                        if (afterDefaultPred)
                            afterDefaultPred = builder->emitOr(boolType, afterDefaultPred, casePred);
                        else
                            afterDefaultPred = casePred;
                    }
                }

                if (afterDefaultPred)
                    caseInfo.directPredicate = builder->emitNot(boolType, afterDefaultPred);
                else
                    caseInfo.directPredicate =
                        builder->getBoolValue(true); // No cases after default
            }
            else
            {
                // Regular case: OR together all matching values
                IRInst* pred = nullptr;
                for (auto value : caseInfo.values)
                {
                    auto eq = builder->emitEql(selector, value);
                    if (pred)
                        pred = builder->emitOr(boolType, pred, eq);
                    else
                        pred = eq;
                }
                caseInfo.directPredicate = pred;
            }
        }

        // Build cumulative predicates (include predecessors that fall through)
        for (Index i = 0; i < cases.getCount(); i++)
        {
            IRInst* reachPred = cases[i].directPredicate;

            // Check all predecessors that can fall through to this case
            for (Index j = 0; j < i; j++)
            {
                if (cases[j].fallsThrough)
                {
                    // Check if case j can reach case i through a chain of fallthroughs
                    bool canReach = true;
                    for (Index k = j; k < i; k++)
                    {
                        if (!cases[k].fallsThrough)
                        {
                            canReach = false;
                            break;
                        }
                    }

                    if (canReach && cases[j].directPredicate)
                    {
                        reachPred = builder->emitOr(boolType, reachPred, cases[j].directPredicate);
                    }
                }
            }

            cases[i].reachPredicate = reachPred;
        }
    }

    /// Perform the transformation.
    void transform()
    {
        collectCases();

        if (cases.getCount() == 0)
            return;

        auto parentBlock = as<IRBlock>(switchInst->getParent());
        auto breakLabel = switchInst->getBreakLabel();

        // Create the loop header block for do-while(false)
        auto loopHeaderBlock = builder->createBlock();
        loopHeaderBlock->insertAfter(parentBlock);

        // Set insert point at the end of the parent block (before the switch)
        builder->setInsertBefore(switchInst);

        // Build predicates first (they go before the loop)
        buildPredicates();

        // Emit loop instruction: do { ... } while(false)
        // The loop's break block is the original switch's break label.
        // The continue block is the header (so continue restarts, but condition is false so we exit).
        builder->emitLoop(loopHeaderBlock, breakLabel, loopHeaderBlock);

        // Now build the if-chain inside the loop header
        builder->setInsertInto(loopHeaderBlock);

        // Emit if-statements for each case in source order.
        // Each if-else must have its own unique afterBlock (merge point) to avoid
        // SPIRV validation errors ("already a merge block for another header").
        for (Index i = 0; i < cases.getCount(); i++)
        {
            auto& caseInfo = cases[i];

            if (!caseInfo.reachPredicate)
                continue;

            // Create the next check block - this is where we go if the predicate is false
            auto nextCheckBlock = builder->createBlock();
            nextCheckBlock->insertAfter(builder->getBlock());

            // Emit: if (reachPredicate) { goto caseBody } else { goto nextCheck }
            // The afterBlock (merge point) is nextCheckBlock, NOT breakLabel.
            // This ensures each if-else has a unique merge block.
            builder->emitIfElse(caseInfo.reachPredicate, caseInfo.label, nextCheckBlock, nextCheckBlock);

            builder->setInsertInto(nextCheckBlock);
        }

        // After all checks fail, branch to break label (unreachable if all cases covered)
        builder->emitBranch(breakLabel);

        // Remove the original switch instruction
        switchInst->removeAndDeallocate();
    }
};

/// Determine if a switch should be lowered to if-else chain.
static bool shouldLowerSwitchToIf(IRSwitch* switchInst, TargetProgram* targetProgram)
{
    SLANG_UNUSED(switchInst);
    SLANG_UNUSED(targetProgram);

    // For now, always lower. This is primarily intended for SPIRV targets
    // to address undefined reconvergence behavior with fallthrough.
    // Other targets like Metal already handle fallthrough correctly with
    // proper wave convergence in their native switch implementation.
    //
    // TODO: Consider restricting to SPIRV targets only if other targets
    // show regressions. The pass should still be semantically correct
    // for all targets, even if not strictly necessary.
    return true;
}

/// Lower all switch statements in a function.
static void lowerSwitchToIfInFunc(
    IRModule* irModule,
    IRGlobalValueWithCode* func,
    TargetProgram* targetProgram)
{
    // Collect all switch instructions first (since we'll be modifying the IR)
    List<IRSwitch*> switches;

    for (auto block : func->getBlocks())
    {
        if (auto switchInst = as<IRSwitch>(block->getTerminator()))
        {
            if (shouldLowerSwitchToIf(switchInst, targetProgram))
            {
                switches.add(switchInst);
            }
        }
    }

    if (switches.getCount() == 0)
        return;

    // To make things easy, eliminate Phis before performing transformations.
    // This follows the pattern established by eliminateMultiLevelBreak.
    // SSA form will be reconstructed later by simplifyIR.
    eliminatePhisInFunc(
        LivenessMode::Disabled,
        irModule,
        func,
        PhiEliminationOptions::getFast());

    // Process each switch
    IRBuilder builder(irModule);

    for (auto switchInst : switches)
    {
        SwitchLoweringContext ctx;
        ctx.switchInst = switchInst;
        ctx.builder = &builder;
        ctx.targetProgram = targetProgram;

        // Set insert point before the switch's parent block terminator
        builder.setInsertBefore(switchInst);

        ctx.transform();
    }
}

void lowerSwitchToIf(IRModule* module, TargetProgram* targetProgram)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (auto func = as<IRGlobalValueWithCode>(globalInst))
        {
            lowerSwitchToIfInFunc(module, func, targetProgram);
        }
    }
}

} // namespace Slang
