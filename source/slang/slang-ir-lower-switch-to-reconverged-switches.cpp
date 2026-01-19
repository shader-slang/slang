// slang-ir-lower-switch-to-reconverged-switches.cpp
//
// Lowers switch statements with fallthrough to use reconverged control flow.
// See GitHub issue #6441 and local-tests/6441/selector-example2.slang
//
// ## Problem
//
// SPIR-V's OpSwitch has undefined reconvergence behavior for fallthrough cases.
// When threads in a wave/subgroup enter different cases that share code via
// fallthrough, they may not properly reconverge at the shared code, causing
// incorrect results for wave operations (WaveActiveSum, etc.).
//
// The SPV_KHR_maximal_reconvergence extension guarantees reconvergence at
// structured control flow merge points (like if-else merges), but OpSwitch
// fallthrough is NOT structured - threads can enter the same code from
// different case labels.
//
// ## Solution: Two-Switch Transformation
//
// We split a switch with fallthrough into two switches:
//
// ### First Switch (Dispatch Switch)
// - Each case sets two variables: `fallthroughSelector` and `fallthroughStage`
// - ALL cases break (no fallthrough in the IR structure)
// - Cases in the same fallthrough group get the same selector value
// - The stage indicates where in the fallthrough sequence to start
//
// ### Second Switch (Execution Switch)  
// - Dispatches on `fallthroughSelector`
// - One case per fallthrough group
// - Each case contains if-guarded stages: `if (fallthroughStage <= N) { ... }`
// - This ensures all threads in a fallthrough group reconverge at each if-merge
//
// ### Non-Fallthrough Cases
// - Get selector = MIN_INT (a sentinel value)
// - Their code executes directly after the first switch dispatch
// - They do NOT go through the second switch's case structure
// - Their break targets must be redirected to the final break label
//
// ## Example
//
// Original (with conditional break in case0):
//   switch(val, breakLabel, defaultLabel, 0:case0, 1:case1, 2:case2)
//   case0: a(); if(cond) branch(breakLabel) else branch(case0_continue)
//   case0_continue: a2(); branch(case1)     // fallthrough
//   case1: b(); branch(breakLabel)          // break
//   case2: c(); branch(breakLabel)          // break
//   defaultLabel: d(); branch(breakLabel)
//   breakLabel: ...
//
// Transformed:
//   switch(val, secondSwitchEntry, newDefault, 0:dispatch0, 1:dispatch1, 2:dispatch2)
//   dispatch0: selector=0; stage=0; branch(secondSwitchEntry)
//   dispatch1: selector=0; stage=1; branch(secondSwitchEntry)
//   dispatch2: selector=MIN_INT; branch(case2Body)
//   newDefault: selector=MIN_INT; branch(defaultBody)
//   
//   case2Body: c(); branch(secondSwitchEntry)
//   defaultBody: d(); branch(secondSwitchEntry)
//   
//   secondSwitchEntry:
//   switch(selector, finalBreak, finalBreak, 0:fallthroughGroup0)
//   fallthroughGroup0:
//     ifElse(stage <= 0, stage0Body, stage0Merge, stage0Merge)
//     stage0Body:
//       a();
//       ifElse(cond, condBreak, condContinue, stage0Merge)
//       condBreak: stage=MAX_INT; branch(stage0Merge)  // skip remaining stages
//       condContinue: a2(); branch(stage0Merge)
//     stage0Merge:
//     ifElse(stage <= 1, stage1Body, stage1Merge, stage1Merge)
//     stage1Body: b(); branch(stage1Merge)
//     stage1Merge:
//     branch(finalBreak)
//   finalBreak: ...
//
// The key insight for conditional breaks: setting stage=MAX_INT causes all
// subsequent "if (stage <= N)" checks to fail, effectively skipping the
// remaining stages while maintaining structured control flow.
//
// ## Loops Inside Fallthrough Cases
//
// Loops inside case bodies are preserved as-is (they have their own structured
// control flow). However, any branch from inside the loop that targets the
// original switch's break label must be transformed:
//
// Original:
//   case0:
//     loop(header, loopBreak, continue)
//       header: if(done) branch(loopBreak) else branch(body)
//       body: ...; if(earlyExit) branch(switchBreak); branch(continue)
//       continue: branch(header)
//     loopBreak: branch(case1)  // fallthrough
//
// Transformed (inside stage0Body):
//   loop(header, loopBreak, continue)
//     header: if(done) branch(loopBreak) else branch(body)
//     body: ...;
//       ifElse(earlyExit, earlyExitBlock, noEarlyExit, loopMerge)
//       earlyExitBlock: stage=MAX_INT; branch(loopMerge)
//       noEarlyExit: branch(loopMerge)
//     loopMerge: branch(continue)
//     continue: branch(header)
//   loopBreak: branch(stage0Merge)  // continues to next stage check
//
// The loop's own break (loopBreak) is NOT a switch break - it just exits
// the loop and continues the case body. Only branches that originally
// targeted the switch's break label need transformation.
//
// ## Key Invariants
//
// 1. First switch's break label becomes the second switch entry
// 2. Fallthrough cases: dispatch blocks set selector/stage, branch to second switch
// 3. Non-fallthrough cases: dispatch blocks set selector=MIN_INT, branch to body
// 4. Non-fallthrough case bodies branch to second switch entry when done
// 5. Second switch routes selector values to fallthrough groups
// 6. Second switch's default = break = finalBreak (MIN_INT goes straight to break)
// 7. All control flow converges at finalBreak

#include "slang-ir-lower-switch-to-reconverged-switches.h"

#include "slang-ir-clone.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-insts.h"
#include "slang-ir-loop-unroll.h"
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
                cases[i].fallsThrough = caseFallsThrough(
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
    /// 
    /// After simplifyCFG runs, empty intermediate blocks are fused into
    /// the switch operands, so fallthrough is detected simply by checking
    /// if case i can reach case i+1's label.
    /// 
    /// We stop traversal at:
    /// - The break label (switch exit)
    /// - ALL case labels from this switch (to avoid following control flow
    ///   that exits the switch, like 'continue' to an outer loop, which would
    ///   re-enter the switch via loop back-edge)
    /// - The switch block itself (for loop back-edge avoidance)
    /// - Blocks already visited (cycle detection)
    bool caseFallsThrough(
        IRBlock* caseLabel1,
        IRBlock* caseLabel2,
        IRBlock* breakLabel)
    {
        if (!caseLabel1 || !caseLabel2)
            return false;

        // Build a set of "stop blocks" - blocks we shouldn't traverse into
        HashSet<IRBlock*> stopBlocks;
        stopBlocks.add(breakLabel);
        stopBlocks.add(caseLabel1);

        // Add the block containing the switch instruction as a stop block.
        // This prevents following control flow that exits the switch (like continue
        // to an outer loop) and then re-enters via the loop back-edge.
        auto switchBlock = as<IRBlock>(switchInst->getParent());
        if (switchBlock)
            stopBlocks.add(switchBlock);

        // Add all other case labels as stop blocks (except caseLabel2, our target)
        UInt caseCount = switchInst->getCaseCount();
        for (UInt i = 0; i < caseCount; i++)
        {
            auto caseLabel = switchInst->getCaseLabel(i);
            if (caseLabel != caseLabel2)
                stopBlocks.add(caseLabel);
        }
        auto defaultLabel = switchInst->getDefaultLabel();
        if (defaultLabel && defaultLabel != caseLabel2)
            stopBlocks.add(defaultLabel);

        // Check if case1 can reach case2's label
        HashSet<IRBlock*> visited;
        List<IRBlock*> workList;

        for (auto succ : caseLabel1->getSuccessors())
        {
            workList.add(succ);
        }

        while (workList.getCount() > 0)
        {
            auto block = workList.getLast();
            workList.removeLast();

            if (!block)
                continue;
            if (visited.contains(block))
                continue;
            if (stopBlocks.contains(block))
                continue;

            // Found case2's label - this is fallthrough
            if (block == caseLabel2)
                return true;

            visited.add(block);

            for (auto succ : block->getSuccessors())
            {
                workList.add(succ);
            }
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

    /// Clone case body blocks into new blocks.
    /// 
    /// This creates new blocks for each block in bodyBlocks, sets up the IRCloneEnv
    /// to redirect branches appropriately, and clones all instructions.
    /// 
    /// The cloning approach is essential because simply branching to existing blocks
    /// and rewriting their terminators doesn't work - SSA optimization passes 
    /// (simplifyIR, eliminatePhis, simplifyNonSSAIR) will merge our dispatch blocks
    /// with the case bodies and coalesce our selector/stage variables with unrelated
    /// variables like loop counters and results.
    /// 
    /// By cloning, we create completely new IR that the optimizer treats as distinct
    /// from the original blocks. The original blocks become dead code and are removed
    /// by dead code elimination.
    /// 
    /// @param bodyBlocks Set of blocks to clone
    /// @param entryBlock The entry block of the case body (first block to execute)
    /// @param redirections Map of old blocks to redirect to new blocks (e.g., breakLabel -> newTarget)
    /// @param insertBeforeBlock Block to insert new blocks before
    /// @returns The cloned entry block
    IRBlock* cloneCaseBodyBlocks(
        const HashSet<IRBlock*>& bodyBlocks,
        IRBlock* entryBlock,
        const Dictionary<IRBlock*, IRBlock*>& redirections,
        IRBlock* insertBeforeBlock)
    {
        if (bodyBlocks.getCount() == 0)
            return nullptr;

        IRCloneEnv cloneEnv;

        // IMPORTANT: Add redirections FIRST, before creating block mappings.
        // This ensures that blocks like breakLabel and nextCaseBlock are properly
        // redirected when encountered during cloning, rather than being cloned.
        for (const auto& [oldBlock, newBlock] : redirections)
        {
            cloneEnv.mapOldValToNew[oldBlock] = newBlock;
        }

        // Create new empty blocks and register mappings for body blocks
        // Skip any blocks that are already in redirections (they shouldn't be cloned)
        Dictionary<IRBlock*, IRBlock*> oldToNew;
        for (auto block : bodyBlocks)
        {
            // Don't create a clone for blocks that should be redirected
            if (redirections.containsKey(block))
                continue;

            auto clonedBlock = builder->createBlock();
            clonedBlock->insertBefore(insertBeforeBlock);
            cloneEnv.mapOldValToNew[block] = clonedBlock;
            oldToNew[block] = clonedBlock;
        }

        // Second pass: clone instructions from old blocks to new blocks
        for (auto block : bodyBlocks)
        {
            // Skip blocks that are redirected (not cloned)
            if (redirections.containsKey(block))
                continue;

            auto clonedBlock = oldToNew[block];
            builder->setInsertInto(clonedBlock);

            for (auto inst : block->getChildren())
            {
                cloneInst(&cloneEnv, builder, inst);
            }
        }

        // Return the cloned entry block
        // First check if it was redirected, then check if it was cloned
        IRBlock* clonedEntry = nullptr;
        if (!oldToNew.tryGetValue(entryBlock, clonedEntry))
        {
            // Entry block might have been redirected (unusual but possible)
            redirections.tryGetValue(entryBlock, clonedEntry);
        }
        return clonedEntry;
    }

    /// Clone case body for non-fallthrough cases.
    /// Redirects breakLabel -> targetBlock.
    IRBlock* cloneNonFallthroughCaseBody(
        IRBlock* entryBlock,
        IRBlock* nextCaseBlock,
        IRBlock* breakLabel,
        IRBlock* targetBlock,
        IRBlock* insertBeforeBlock)
    {
        HashSet<IRBlock*> bodyBlocks;
        collectCaseBodyBlocks(entryBlock, nextCaseBlock, breakLabel, bodyBlocks);

        if (bodyBlocks.getCount() == 0)
            return nullptr;

        Dictionary<IRBlock*, IRBlock*> redirections;
        redirections[breakLabel] = targetBlock;

        return cloneCaseBodyBlocks(bodyBlocks, entryBlock, redirections, insertBeforeBlock);
    }

    /// Clone case body for fallthrough cases in the second switch.
    /// Creates break handling (sets stage to MAX_INT) and redirects appropriately.
    IRBlock* cloneFallthroughCaseBody(
        IRBlock* entryBlock,
        IRBlock* nextCaseBlock,
        IRBlock* breakLabel,
        IRBlock* stageMergeBlock,
        IRInst* fallthroughStageVar,
        IRInst* maxStageValue,
        IRBlock* insertBeforeBlock)
    {
        HashSet<IRBlock*> bodyBlocks;
        collectCaseBodyBlocks(entryBlock, nextCaseBlock, breakLabel, bodyBlocks);

        if (bodyBlocks.getCount() == 0)
            return nullptr;

        // For fallthrough cases, we need special handling:
        // - breakLabel branches need to set stage=MAX_INT, then branch to stageMergeBlock
        // - nextCaseBlock branches (fallthrough) need to branch to stageMergeBlock
        //
        // We handle breakLabel by creating a break handler block.

        auto breakHandlerBlock = builder->createBlock();
        breakHandlerBlock->insertBefore(insertBeforeBlock);
        builder->setInsertInto(breakHandlerBlock);
        builder->emitStore(fallthroughStageVar, maxStageValue);
        builder->emitBranch(stageMergeBlock);

        Dictionary<IRBlock*, IRBlock*> redirections;
        redirections[breakLabel] = breakHandlerBlock;
        if (nextCaseBlock)
        {
            redirections[nextCaseBlock] = stageMergeBlock;
        }

        return cloneCaseBodyBlocks(bodyBlocks, entryBlock, redirections, insertBeforeBlock);
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
        // We add name hints to prevent register allocation from coalescing these
        // with other variables (which could cause incorrect code generation).
        builder->setInsertBefore(switchInst);

        auto fallthroughSelectorVar = builder->emitVar(intType);
        builder->addNameHintDecoration(fallthroughSelectorVar, UnownedStringSlice("_ft_selector"));
        builder->emitStore(fallthroughSelectorVar, originalSelector);

        auto fallthroughStageVar = builder->emitVar(intType);
        builder->addNameHintDecoration(fallthroughStageVar, UnownedStringSlice("_ft_stage"));
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
                // Non-fallthrough case: execute body directly, then skip second switch.
                // 
                // IMPORTANT: We CLONE the case body blocks rather than branching to them.
                // Simply branching to existing blocks and rewriting terminators doesn't work
                // because SSA optimization passes (simplifyIR, eliminatePhis, simplifyNonSSAIR)
                // will merge our dispatch blocks with the case bodies and coalesce our
                // selector/stage variables with unrelated variables like loop counters.
                //
                // By cloning, we create completely new IR that the optimizer treats as
                // distinct from the original blocks.

                builder->emitStore(fallthroughSelectorVar, skipSelectorValue);

                IRBlock* nextCaseBlock = (i + 1 < cases.getCount()) ? cases[i + 1].label : nullptr;

                // Clone the case body with breakLabel redirected to betweenSwitchesBlock
                auto clonedEntry = cloneNonFallthroughCaseBody(
                    caseInfo.label,
                    nextCaseBlock,
                    breakLabel,
                    betweenSwitchesBlock,
                    betweenSwitchesBlock);

                if (clonedEntry)
                {
                    builder->setInsertInto(caseBlock);
                    builder->emitBranch(clonedEntry);
                }
                else
                {
                    // Empty body - just branch to between switches
                    builder->emitBranch(betweenSwitchesBlock);
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
            // 
            // IMPORTANT: We CLONE the case body blocks rather than branching to them.
            // This is essential because simply branching and rewriting terminators
            // doesn't work - SSA passes will merge/coalesce our structures.
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

                // Clone the case body for this stage
                IRBlock* nextCaseBlock = (caseIdx + 1 < cases.getCount())
                    ? cases[caseIdx + 1].label
                    : nullptr;

                auto clonedEntry = cloneFallthroughCaseBody(
                    caseInfo.label,
                    nextCaseBlock,
                    breakLabel,
                    stageMergeBlock,
                    fallthroughStageVar,
                    maxStageValue,
                    stageMergeBlock);

                if (clonedEntry)
                {
                    // Emit if-else: if (condition) goto clonedEntry else goto merge
                    builder->setInsertInto(currentBlock);
                    builder->emitIfElse(condition, clonedEntry, stageMergeBlock, stageMergeBlock);
                }
                else
                {
                    // Empty body - just branch to merge
                    builder->setInsertInto(currentBlock);
                    builder->emitBranch(stageMergeBlock);
                }

                // Set up for next stage
                currentBlock = stageMergeBlock;
            }

            // After all stages, branch to break label
            builder->setInsertInto(currentBlock);
            builder->emitBranch(afterSecondSwitchBlock);
        }

        // Emit second switch
        // Default = break = afterSecondSwitchBlock (MIN_INT selector goes straight to break)
        builder->setInsertInto(betweenSwitchesBlock);
        builder->emitSwitch(
            fallthroughSelectorLoad2,
            afterSecondSwitchBlock,
            afterSecondSwitchBlock,  // default = break (no intermediate block needed)
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
    // First, eliminate continue blocks in this function.
    // This transforms `continue` statements (which might target outer loops from 
    // inside switches) into breaks to inner regions. This simplifies the control 
    // flow analysis for fallthrough detection, preventing us from incorrectly 
    // detecting fallthrough when a case uses `continue` to exit to an outer loop.
    eliminateContinueBlocksInFunc(irModule, func);

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
