// slang-ir-lower-switch-to-reconverged-switches.cpp
//
// Lowers switch statements with fallthrough to use reconverged control flow.
// This splits switches into two: the first sets selector/stage variables,
// the second executes fallthrough sequences with if-guards.
//
// See GitHub issue #6441 and local-tests/6441/selector-example2.slang

#include "slang-ir-lower-switch-to-reconverged-switches.h"

#include "slang-ir-eliminate-phis.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// Forward declarations
static bool shouldLowerSwitch(IRSwitch* switchInst, TargetProgram* targetProgram);

/// Information about a case in the switch.
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

    /// Index of the fallthrough group this case belongs to (-1 if no fallthrough)
    Index fallthroughGroupIndex = -1;

    /// Stage within the fallthrough group (0 = first case in group)
    Index stageIndex = 0;
};

/// Information about a fallthrough group.
struct FallthroughGroup
{
    /// Indices into cases array for cases in this group (in execution order)
    List<Index> caseIndices;

    /// The canonical selector value for this group (first explicit case value)
    IRInst* canonicalValue = nullptr;
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

    /// Fallthrough groups
    List<FallthroughGroup> fallthroughGroups;

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
        sortCasesByBlockOrder();

        // Determine which cases fall through by checking for shared blocks
        for (Index i = 0; i < cases.getCount(); i++)
        {
            if (i + 1 < cases.getCount())
            {
                cases[i].fallsThrough = casesShareBlocks(
                    cases[i].label, cases[i + 1].label, breakLabel);
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

    /// Collect all blocks reachable from startBlock without hitting breakLabel.
    static void collectReachableBlocks(
        IRBlock* startBlock,
        IRBlock* breakLabel,
        HashSet<IRBlock*>& visited,
        HashSet<IRBlock*>& outBlocks)
    {
        if (!startBlock)
            return;
        if (visited.contains(startBlock))
            return;
        if (startBlock == breakLabel)
            return;

        visited.add(startBlock);
        outBlocks.add(startBlock);

        auto terminator = startBlock->getTerminator();
        if (!terminator)
            return;

        for (auto succ : startBlock->getSuccessors())
        {
            collectReachableBlocks(succ, breakLabel, visited, outBlocks);
        }
    }

    /// Check if case i "falls through" to case i+1.
    /// We detect this by checking if they share any blocks (other than the break label).
    /// If case i reaches some block X, and case i+1 also reaches X, they share that block,
    /// meaning threads entering via case i and case i+1 could converge at X.
    static bool casesShareBlocks(
        IRBlock* caseLabel1,
        IRBlock* caseLabel2,
        IRBlock* breakLabel)
    {
        if (!caseLabel1 || !caseLabel2)
            return false;

        // Collect blocks reachable from case 1 (excluding case 1's own label)
        HashSet<IRBlock*> visited1;
        HashSet<IRBlock*> blocks1;
        visited1.add(caseLabel1); // Don't include the entry block itself
        auto terminator1 = caseLabel1->getTerminator();
        if (terminator1)
        {
            for (auto succ : caseLabel1->getSuccessors())
            {
                collectReachableBlocks(succ, breakLabel, visited1, blocks1);
            }
        }

        // Collect blocks reachable from case 2 (excluding case 2's own label)
        HashSet<IRBlock*> visited2;
        HashSet<IRBlock*> blocks2;
        visited2.add(caseLabel2);
        auto terminator2 = caseLabel2->getTerminator();
        if (terminator2)
        {
            for (auto succ : caseLabel2->getSuccessors())
            {
                collectReachableBlocks(succ, breakLabel, visited2, blocks2);
            }
        }

        // Check for intersection
        for (auto block : blocks1)
        {
            if (blocks2.contains(block))
                return true;
        }

        return false;
    }

    /// Build fallthrough groups from the collected cases.
    void buildFallthroughGroups()
    {
        // Assign cases to fallthrough groups
        Index currentGroupIndex = -1;
        Index currentStage = 0;

        for (Index i = 0; i < cases.getCount(); i++)
        {
            auto& caseInfo = cases[i];

            // Check if this case starts a new fallthrough group or continues one
            bool startNewGroup = false;

            if (i == 0)
            {
                // First case: start a group if it falls through
                if (caseInfo.fallsThrough)
                    startNewGroup = true;
            }
            else if (cases[i - 1].fallsThrough)
            {
                // Previous case falls through to us: continue the group
                // (group was already started)
            }
            else if (caseInfo.fallsThrough)
            {
                // Previous case doesn't fall through, but we do: start new group
                startNewGroup = true;
            }
            else
            {
                // Neither predecessor nor we fall through: not in a group
                currentGroupIndex = -1;
            }

            if (startNewGroup)
            {
                FallthroughGroup group;
                currentGroupIndex = fallthroughGroups.getCount();
                fallthroughGroups.add(group);
                currentStage = 0;
            }

            if (currentGroupIndex >= 0 || cases[i - 1 >= 0 ? i - 1 : 0].fallsThrough)
            {
                // We're in a fallthrough group
                if (i > 0 && cases[i - 1].fallsThrough && currentGroupIndex < 0)
                {
                    // Predecessor fell through but we didn't start a group yet
                    // This shouldn't happen given our logic, but handle it
                    currentGroupIndex = fallthroughGroups.getCount() - 1;
                }

                if (currentGroupIndex >= 0)
                {
                    caseInfo.fallthroughGroupIndex = currentGroupIndex;
                    caseInfo.stageIndex = currentStage++;
                    fallthroughGroups[currentGroupIndex].caseIndices.add(i);
                }
            }

            // If this case doesn't fall through, end the current group
            if (!caseInfo.fallsThrough)
            {
                currentGroupIndex = -1;
                currentStage = 0;
            }
        }

        // Determine canonical selector value for each group
        for (auto& group : fallthroughGroups)
        {
            // Find the first explicit case value (skip default)
            for (Index caseIdx : group.caseIndices)
            {
                auto& caseInfo = cases[caseIdx];
                if (!caseInfo.isDefault && caseInfo.values.getCount() > 0)
                {
                    group.canonicalValue = caseInfo.values[0];
                    break;
                }
            }

            // If no explicit case value found (all defaults?), this group is odd
            // In practice this shouldn't happen with well-formed switches
        }
    }

    /// Check if the switch has any fallthrough that needs handling.
    bool hasFallthrough() const
    {
        for (const auto& caseInfo : cases)
        {
            if (caseInfo.fallsThrough)
                return true;
        }
        return false;
    }

    /// Check if this switch needs transformation for proper reconvergence.
    /// Currently we only transform if any case has fallthrough (shared blocks).
    /// 
    /// Note: We intentionally do not consider the presence of a default case
    /// as a trigger for transformation. While default cases can theoretically
    /// cause reconvergence issues (since multiple unmatched selector values
    /// can enter via default), there are practical reasons to skip them:
    /// 
    /// 1. The parser-to-IR lowering often generates implicit default cases
    ///    for variable initialization (e.g., `int result = 0; switch(val) {...}`
    ///    becomes a default case that stores the initial value). These are
    ///    difficult to distinguish from explicit user-written defaults.
    /// 
    /// 2. We assume that reasonable GPU drivers will properly reconverge
    ///    threads entering via the default case, since this is a common
    ///    and well-understood control flow pattern.
    /// 
    /// If reconvergence issues are observed with default cases in practice,
    /// this logic may need to be revisited.
    bool needsTransformation() const
    {
        return hasFallthrough();
    }

    /// Collect all blocks reachable from entryBlock that are part of this case's body.
    static void collectCaseBodyBlocks(
        IRBlock* entryBlock,
        IRBlock* nextCaseBlock,
        IRBlock* breakLabel,
        HashSet<IRBlock*>& outBlocks)
    {
        List<IRBlock*> workList;
        workList.add(entryBlock);

        while (workList.getCount() > 0)
        {
            auto block = workList.getLast();
            workList.removeLast();

            if (!block)
                continue;
            if (outBlocks.contains(block))
                continue;
            if (block == nextCaseBlock)
                continue;
            if (block == breakLabel)
                continue;

            outBlocks.add(block);

            auto terminator = block->getTerminator();
            if (terminator)
            {
                for (auto succ : block->getSuccessors())
                {
                    workList.add(succ);
                }
            }
        }
    }

    /// Rewrite branches in caseBodyBlocks: replace branches to breakLabel with
    /// fallthroughStage = MAX_INT; branch to mergeBlock.
    /// Replace branches to nextCaseBlock with branch to mergeBlock.
    void rewriteCaseBodyBranches(
        const HashSet<IRBlock*>& caseBodyBlocks,
        IRBlock* nextCaseBlock,
        IRBlock* breakLabel,
        IRBlock* mergeBlock,
        IRInst* fallthroughStageVar,
        IRInst* maxStageValue)
    {
        for (auto block : caseBodyBlocks)
        {
            auto terminator = block->getTerminator();
            if (!terminator)
                continue;

            switch (terminator->getOp())
            {
            case kIROp_UnconditionalBranch:
                {
                    auto branch = as<IRUnconditionalBranch>(terminator);
                    auto target = branch->getTargetBlock();

                    if (target == breakLabel)
                    {
                        // break -> set stage to MAX, branch to merge
                        builder->setInsertBefore(terminator);
                        builder->emitStore(fallthroughStageVar, maxStageValue);
                        builder->emitBranch(mergeBlock);
                        terminator->removeAndDeallocate();
                    }
                    else if (target == nextCaseBlock)
                    {
                        // fallthrough -> branch to merge (continue to next stage)
                        builder->setInsertBefore(terminator);
                        builder->emitBranch(mergeBlock);
                        terminator->removeAndDeallocate();
                    }
                }
                break;

            case kIROp_ConditionalBranch:
                {
                    auto condBranch = as<IRConditionalBranch>(terminator);
                    auto trueTarget = condBranch->getTrueBlock();
                    auto falseTarget = condBranch->getFalseBlock();

                    // For conditional branches where one target is break/fallthrough,
                    // we need to create wrapper blocks that set the stage appropriately

                    bool trueIsBreak = (trueTarget == breakLabel);
                    bool trueIsFallthrough = (trueTarget == nextCaseBlock);
                    bool falseIsBreak = (falseTarget == breakLabel);
                    bool falseIsFallthrough = (falseTarget == nextCaseBlock);

                    if (trueIsBreak || trueIsFallthrough || falseIsBreak || falseIsFallthrough)
                    {
                        auto condition = condBranch->getCondition();
                        builder->setInsertBefore(terminator);

                        IRBlock* newTrueTarget = trueTarget;
                        IRBlock* newFalseTarget = falseTarget;

                        // Create wrapper blocks as needed
                        if (trueIsBreak)
                        {
                            auto wrapperBlock = builder->createBlock();
                            wrapperBlock->insertBefore(mergeBlock);
                            builder->setInsertInto(wrapperBlock);
                            builder->emitStore(fallthroughStageVar, maxStageValue);
                            builder->emitBranch(mergeBlock);
                            newTrueTarget = wrapperBlock;
                            builder->setInsertBefore(terminator);
                        }
                        else if (trueIsFallthrough)
                        {
                            newTrueTarget = mergeBlock;
                        }

                        if (falseIsBreak)
                        {
                            auto wrapperBlock = builder->createBlock();
                            wrapperBlock->insertBefore(mergeBlock);
                            builder->setInsertInto(wrapperBlock);
                            builder->emitStore(fallthroughStageVar, maxStageValue);
                            builder->emitBranch(mergeBlock);
                            newFalseTarget = wrapperBlock;
                            builder->setInsertBefore(terminator);
                        }
                        else if (falseIsFallthrough)
                        {
                            newFalseTarget = mergeBlock;
                        }

                        builder->emitBranch(condition, newTrueTarget, newFalseTarget);
                        terminator->removeAndDeallocate();
                    }
                }
                break;
            }
        }
    }

    /// Perform the transformation.
    void transform()
    {
        collectCases();

        if (cases.getCount() == 0)
            return;

        // Build fallthrough groups
        buildFallthroughGroups();

        // Only transform switches that need it for proper reconvergence
        if (!needsTransformation())
            return;

        auto parentBlock = as<IRBlock>(switchInst->getParent());
        auto breakLabel = switchInst->getBreakLabel();
        auto originalSelector = switchInst->getCondition();
        auto intType = originalSelector->getDataType();

        // Create variables for fallthroughSelector and fallthroughStage
        builder->setInsertBefore(switchInst);

        auto fallthroughSelectorVar = builder->emitVar(intType);
        builder->emitStore(fallthroughSelectorVar, originalSelector);

        auto fallthroughStageVar = builder->emitVar(intType);
        auto zeroValue = builder->getIntValue(intType, 0);
        builder->emitStore(fallthroughStageVar, zeroValue);

        // MAX_INT for early exit (stage will never be <= MAX_INT)
        auto maxStageValue = builder->getIntValue(intType, INT32_MAX);

        // A special "skip" value for fallthroughSelector when non-fallthrough case ran
        // We use a value that won't match any case in the second switch
        auto skipSelectorValue = builder->getIntValue(intType, INT32_MIN);

        // --- Build the first switch ---
        // All cases in the first switch set fallthroughSelector and fallthroughStage, then break.
        // Non-fallthrough cases execute their body directly.

        // Block after first switch, before second switch
        auto betweenSwitchesBlock = builder->createBlock();
        betweenSwitchesBlock->insertAfter(parentBlock);

        // Build case args for first switch (interleaved: value, label, value, label, ...)
        List<IRInst*> firstSwitchCaseArgs;

        // Track which blocks we've created for first switch cases
        Dictionary<Index, IRBlock*> caseIndexToFirstSwitchBlock;

        for (Index i = 0; i < cases.getCount(); i++)
        {
            auto& caseInfo = cases[i];

            auto caseBlock = builder->createBlock();
            caseBlock->insertBefore(betweenSwitchesBlock);
            caseIndexToFirstSwitchBlock[i] = caseBlock;

            builder->setInsertInto(caseBlock);

            if (caseInfo.fallthroughGroupIndex >= 0)
            {
                // This case is part of a fallthrough group
                auto& group = fallthroughGroups[caseInfo.fallthroughGroupIndex];

                // Set fallthroughSelector to the group's canonical value
                if (group.canonicalValue)
                {
                    builder->emitStore(fallthroughSelectorVar, group.canonicalValue);
                }

                // Set fallthroughStage to this case's stage index
                auto stageValue = builder->getIntValue(intType, caseInfo.stageIndex);
                builder->emitStore(fallthroughStageVar, stageValue);

                // Break to between-switches block
                builder->emitBranch(betweenSwitchesBlock);
            }
            else
            {
                // Non-fallthrough case: execute body directly, then skip second switch
                // Set fallthroughSelector to skip value so second switch doesn't match
                builder->emitStore(fallthroughSelectorVar, skipSelectorValue);

                // Branch to original case body
                builder->emitBranch(caseInfo.label);

                // Retarget original case body's break to betweenSwitchesBlock
                // (so it continues to second switch, which will skip due to selector)
                HashSet<IRBlock*> bodyBlocks;
                IRBlock* nextCaseBlock = (i + 1 < cases.getCount()) ? cases[i + 1].label : nullptr;
                collectCaseBodyBlocks(caseInfo.label, nextCaseBlock, breakLabel, bodyBlocks);

                for (auto block : bodyBlocks)
                {
                    auto terminator = block->getTerminator();
                    if (auto branch = as<IRUnconditionalBranch>(terminator))
                    {
                        if (branch->getTargetBlock() == breakLabel)
                        {
                            builder->setInsertBefore(terminator);
                            builder->emitBranch(betweenSwitchesBlock);
                            terminator->removeAndDeallocate();
                        }
                    }
                }
            }

            // Add case values to first switch (interleaved: value, label)
            for (auto value : caseInfo.values)
            {
                firstSwitchCaseArgs.add(value);
                firstSwitchCaseArgs.add(caseBlock);
            }
        }

        // Default case for first switch
        IRBlock* firstSwitchDefaultBlock = nullptr;
        if (defaultIndex >= 0)
        {
            firstSwitchDefaultBlock = caseIndexToFirstSwitchBlock[defaultIndex];
        }
        else
        {
            // No default: create a block that just branches to between-switches
            firstSwitchDefaultBlock = builder->createBlock();
            firstSwitchDefaultBlock->insertBefore(betweenSwitchesBlock);
            builder->setInsertInto(firstSwitchDefaultBlock);
            builder->emitStore(fallthroughSelectorVar, skipSelectorValue);
            builder->emitBranch(betweenSwitchesBlock);
        }

        // Emit first switch at end of parent block
        builder->setInsertBefore(switchInst);
        builder->emitSwitch(
            originalSelector,
            betweenSwitchesBlock,  // break label
            firstSwitchDefaultBlock,
            (UInt)firstSwitchCaseArgs.getCount(),
            firstSwitchCaseArgs.getBuffer());

        // --- Build the second switch ---
        // One case per fallthrough group, with if-guards for stages

        builder->setInsertInto(betweenSwitchesBlock);
        auto fallthroughSelectorLoad2 = builder->emitLoad(fallthroughSelectorVar);

        // Case args for second switch (interleaved: value, label, ...)
        List<IRInst*> secondSwitchCaseArgs;

        // Block after second switch
        auto afterSecondSwitchBlock = breakLabel;

        for (Index groupIdx = 0; groupIdx < fallthroughGroups.getCount(); groupIdx++)
        {
            auto& group = fallthroughGroups[groupIdx];

            if (!group.canonicalValue)
                continue;

            // Create the case block for this group
            auto groupCaseBlock = builder->createBlock();
            groupCaseBlock->insertBefore(afterSecondSwitchBlock);

            secondSwitchCaseArgs.add(group.canonicalValue);
            secondSwitchCaseArgs.add(groupCaseBlock);

            // Build the if-chain for stages within this group
            IRBlock* currentBlock = groupCaseBlock;

            for (Index stageIdx = 0; stageIdx < group.caseIndices.getCount(); stageIdx++)
            {
                Index caseIdx = group.caseIndices[stageIdx];
                auto& caseInfo = cases[caseIdx];

                // Create merge block for this stage's if
                auto stageMergeBlock = builder->createBlock();
                stageMergeBlock->insertBefore(afterSecondSwitchBlock);

                // Build condition: fallthroughStage <= stageIdx
                builder->setInsertInto(currentBlock);
                auto fallthroughStageLoad = builder->emitLoad(fallthroughStageVar);
                auto stageValue = builder->getIntValue(intType, stageIdx);
                // fallthroughStage <= stageIdx is equivalent to stageIdx >= fallthroughStage
                auto condition = builder->emitGeq(stageValue, fallthroughStageLoad);

                // Emit if-else: if (condition) goto caseBody else goto merge
                builder->emitIfElse(condition, caseInfo.label, stageMergeBlock, stageMergeBlock);

                // Rewrite the case body's branches
                HashSet<IRBlock*> bodyBlocks;
                IRBlock* nextCaseBlock = (caseIdx + 1 < cases.getCount())
                    ? cases[caseIdx + 1].label
                    : nullptr;
                collectCaseBodyBlocks(caseInfo.label, nextCaseBlock, breakLabel, bodyBlocks);

                rewriteCaseBodyBranches(
                    bodyBlocks,
                    nextCaseBlock,
                    breakLabel,
                    stageMergeBlock,
                    fallthroughStageVar,
                    maxStageValue);

                // Set up for next stage
                currentBlock = stageMergeBlock;
            }

            // After all stages, branch to break label
            builder->setInsertInto(currentBlock);
            builder->emitBranch(afterSecondSwitchBlock);
        }

        // Default for second switch: just branch to break (no fallthrough group matched)
        auto secondSwitchDefaultBlock = builder->createBlock();
        secondSwitchDefaultBlock->insertBefore(afterSecondSwitchBlock);
        builder->setInsertInto(secondSwitchDefaultBlock);
        builder->emitBranch(afterSecondSwitchBlock);

        // Emit second switch
        builder->setInsertInto(betweenSwitchesBlock);
        builder->emitSwitch(
            fallthroughSelectorLoad2,
            afterSecondSwitchBlock,
            secondSwitchDefaultBlock,
            (UInt)secondSwitchCaseArgs.getCount(),
            secondSwitchCaseArgs.getBuffer());

        // Remove the original switch instruction
        switchInst->removeAndDeallocate();
    }
};

/// Determine if a switch should be considered for lowering.
/// We always return true here - the actual decision about whether to transform
/// happens inside SwitchLoweringContext::transform() based on needsTransformation().
/// 
/// TODO: In the future, we may want to restrict this to SPIR-V targets only,
/// since other targets may handle reconvergence correctly in hardware.
static bool shouldLowerSwitch(IRSwitch* switchInst, TargetProgram* targetProgram)
{
    SLANG_UNUSED(switchInst);
    SLANG_UNUSED(targetProgram);

    // Always consider switches for potential lowering.
    // The transform() method will check needsTransformation() and early-exit if not needed.
    return true;
}

/// Lower all switch statements in a function.
static void lowerSwitchInFunc(
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
            if (shouldLowerSwitch(switchInst, targetProgram))
            {
                switches.add(switchInst);
            }
        }
    }

    if (switches.getCount() == 0)
        return;

    // Note: eliminatePhis is called before this pass in slang-emit.cpp,
    // so we expect to see IR without block parameters here.

    // Process each switch
    IRBuilder builder(irModule);

    for (auto switchInst : switches)
    {
        SwitchLoweringContext ctx;
        ctx.switchInst = switchInst;
        ctx.builder = &builder;
        ctx.targetProgram = targetProgram;

        ctx.transform();
    }
}

void lowerSwitchToReconvergedSwitches(IRModule* module, TargetProgram* targetProgram)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (auto func = as<IRGlobalValueWithCode>(globalInst))
        {
            lowerSwitchInFunc(module, func, targetProgram);
        }
    }
}

} // namespace Slang
