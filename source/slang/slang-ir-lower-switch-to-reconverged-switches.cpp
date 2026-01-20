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

// Sentinel value for _ft_stage when a break is executed.
// Setting _ft_stage to this value ensures all subsequent stage guards
// (stageIdx >= _ft_stage) will fail, skipping remaining stages.
// This requires that no fallthrough group has this many or more cases.
static const IRIntegerValue kBreakStageSentinel = INT32_MAX;

// Forward declarations
static bool shouldLowerSwitch(IRSwitch* switchInst, TargetProgram* targetProgram);
static bool isInstructionSafeForFallthrough(IRInst* inst);
/// Build the set of "stop blocks" for switch case traversal.
///
/// Stop blocks are blocks we shouldn't traverse into during reachability analysis:
/// - The break label (end of switch)
/// - The source block itself (cycle detection)
/// - The switch's parent block (prevents loop back-edge traversal)
/// - All other case labels except the target (prevents cross-case traversal)
static void buildSwitchStopBlocks(
    IRSwitch* switchInst,
    IRBlock* sourceBlock,
    IRBlock* targetBlock,
    IRBlock* breakLabel,
    HashSet<IRBlock*>& outStopBlocks)
{
    outStopBlocks.add(breakLabel);
    outStopBlocks.add(sourceBlock);

    auto switchBlock = as<IRBlock>(switchInst->getParent());
    if (switchBlock)
        outStopBlocks.add(switchBlock);

    UInt caseCount = switchInst->getCaseCount();
    for (UInt i = 0; i < caseCount; i++)
    {
        auto caseLabel = switchInst->getCaseLabel(i);
        if (caseLabel != targetBlock)
            outStopBlocks.add(caseLabel);
    }
    auto defaultLabel = switchInst->getDefaultLabel();
    if (defaultLabel && defaultLabel != targetBlock)
        outStopBlocks.add(defaultLabel);
}

/// Check if caseLabel1 can reach caseLabel2 (i.e., falls through to it).
///
/// Performs a reachability analysis from caseLabel1's successors.
/// Returns true if caseLabel2 is reachable without going through stop blocks.
static bool caseFallsThroughTo(
    IRSwitch* switchInst,
    IRBlock* caseLabel1,
    IRBlock* caseLabel2,
    IRBlock* breakLabel)
{
    if (!caseLabel1 || !caseLabel2)
        return false;

    HashSet<IRBlock*> stopBlocks;
    buildSwitchStopBlocks(switchInst, caseLabel1, caseLabel2, breakLabel, stopBlocks);

    HashSet<IRBlock*> visited;
    List<IRBlock*> workList;

    for (auto succ : caseLabel1->getSuccessors())
        workList.add(succ);

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

        if (block == caseLabel2)
            return true;

        visited.add(block);

        for (auto succ : block->getSuccessors())
            workList.add(succ);
    }

    return false;
}

/// Collect blocks reachable between two case labels (the fallthrough path).
/// Includes caseLabel1 and caseLabel2 in the output if reachable.
static void collectFallthroughBlocks(
    IRSwitch* switchInst,
    IRBlock* caseLabel1,
    IRBlock* caseLabel2,
    IRBlock* breakLabel,
    HashSet<IRBlock*>& outBlocks)
{
    HashSet<IRBlock*> stopBlocks;
    buildSwitchStopBlocks(switchInst, caseLabel1, caseLabel2, breakLabel, stopBlocks);

    HashSet<IRBlock*> visited;
    List<IRBlock*> workList;
    workList.add(caseLabel1);

    while (workList.getCount() > 0)
    {
        auto block = workList.getLast();
        workList.removeLast();

        if (!block)
            continue;
        if (visited.contains(block))
            continue;
        // Don't cross into stop blocks (except the source block)
        if (stopBlocks.contains(block) && block != caseLabel1)
            continue;

        visited.add(block);
        outBlocks.add(block);

        // Stop at caseLabel2 - don't traverse further
        if (block == caseLabel2)
            continue;

        for (auto succ : block->getSuccessors())
            workList.add(succ);
    }
}

/// Check if a fallthrough path contains any operations that require reconvergence.
/// Returns true if there are unsafe operations (needs lowering).
static bool fallthroughPathNeedsLowering(
    IRSwitch* switchInst,
    IRBlock* caseLabel1,
    IRBlock* caseLabel2,
    IRBlock* breakLabel)
{
    HashSet<IRBlock*> fallthroughBlocks;
    collectFallthroughBlocks(switchInst, caseLabel1, caseLabel2, breakLabel, fallthroughBlocks);

    for (auto block : fallthroughBlocks)
    {
        for (auto inst : block->getChildren())
        {
            if (!isInstructionSafeForFallthrough(inst))
                return true;
        }
    }

    return false;
}

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

    /// The group's index (0-based), used as the selector value in the second switch.
    /// This avoids potential collisions with actual case values.
    Index groupIndex = -1;

    /// True if this group contains operations that require reconvergence lowering.
    /// Groups with only safe operations (math, load, store, etc.) can keep
    /// their original fallthrough behavior without going through the second switch.
    bool needsLowering = false;
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
        return caseFallsThroughTo(switchInst, caseLabel1, caseLabel2, breakLabel);
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

    }

    /// Check each fallthrough group to determine if it needs reconvergence lowering.
    /// Groups containing only safe operations (math, load, store, etc.) can keep
    /// their original fallthrough behavior.
    /// 
    /// Also assigns contiguous 0-based group indices only to groups that need lowering.
    /// This ensures the second switch has cases 0, 1, 2, ... with -1 as the skip value.
    void checkGroupsForLowering()
    {
        auto breakLabel = switchInst->getBreakLabel();

        for (auto& group : fallthroughGroups)
        {
            group.needsLowering = false;

            // Check fallthrough paths between consecutive cases (skip first case - nothing 
            // falls into it). We start at i=1 to check the path INTO each case, since threads
            // from earlier cases will reconverge there.
            for (Index i = 1; i < group.caseIndices.getCount(); i++)
            {
                Index prevCaseIdx = group.caseIndices[i - 1];
                Index caseIdx = group.caseIndices[i];

                auto& prevCaseInfo = cases[prevCaseIdx];
                auto& caseInfo = cases[caseIdx];

                // Check if the fallthrough path from previous case to this case contains
                // unsafe operations
                if (checkFallthroughPathNeedsLowering(prevCaseInfo.label, caseInfo.label, breakLabel))
                {
                    group.needsLowering = true;
                    break;
                }
            }

            // Also check the last case's body (from its label to break).
            // This is critical because threads from all earlier cases reconverge here,
            // so any wave ops in the last case's body require reconvergence.
            if (!group.needsLowering && group.caseIndices.getCount() > 0)
            {
                Index lastCaseIdx = group.caseIndices.getLast();
                auto& lastCaseInfo = cases[lastCaseIdx];
                if (checkCaseBodyNeedsLowering(lastCaseInfo.label, breakLabel))
                {
                    group.needsLowering = true;
                }
            }
        }

        // Assign contiguous 0-based indices only to groups that need lowering
        Index nextGroupIndex = 0;
        for (auto& group : fallthroughGroups)
        {
            if (group.needsLowering)
            {
                group.groupIndex = nextGroupIndex++;
            }
            // Groups that don't need lowering keep groupIndex = -1
        }
    }

    /// Check if a fallthrough path contains any operations that require reconvergence.
    bool checkFallthroughPathNeedsLowering(
        IRBlock* caseLabel1,
        IRBlock* caseLabel2,
        IRBlock* breakLabel)
    {
        return fallthroughPathNeedsLowering(switchInst, caseLabel1, caseLabel2, breakLabel);
    }

    /// Check if a case body (from label to break) contains any unsafe operations.
    /// This checks the FULL body of the case, not just the path to the next case.
    bool checkCaseBodyNeedsLowering(IRBlock* caseLabel, IRBlock* breakLabel)
    {
        HashSet<IRBlock*> stopBlocks;
        // Stop at break label and all other case labels
        stopBlocks.add(breakLabel);
        for (UInt i = 0; i < switchInst->getCaseCount(); i++)
        {
            auto otherLabel = switchInst->getCaseLabel(i);
            if (otherLabel != caseLabel)
                stopBlocks.add(otherLabel);
        }
        auto defaultLabel = switchInst->getDefaultLabel();
        if (defaultLabel && defaultLabel != caseLabel)
            stopBlocks.add(defaultLabel);

        // Collect all blocks reachable from caseLabel (the case body)
        HashSet<IRBlock*> visited;
        List<IRBlock*> workList;
        workList.add(caseLabel);

        while (workList.getCount() > 0)
        {
            auto block = workList.getLast();
            workList.removeLast();

            if (!block || visited.contains(block))
                continue;
            // Don't cross into stop blocks (except the source block)
            if (stopBlocks.contains(block) && block != caseLabel)
                continue;

            visited.add(block);

            // Check this block for unsafe operations
            for (auto inst : block->getChildren())
            {
                if (!isInstructionSafeForFallthrough(inst))
                    return true;
            }

            for (auto succ : block->getSuccessors())
                workList.add(succ);
        }

        return false;
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

    /// Check if any fallthrough group requires reconvergence lowering.
    bool hasGroupNeedingLowering() const
    {
        for (const auto& group : fallthroughGroups)
        {
            if (group.needsLowering)
                return true;
        }
        return false;
    }

    /// Check if this switch needs transformation for proper reconvergence.
    /// We only transform if at least one fallthrough group contains unsafe
    /// operations (wave ops, function calls, etc.) that require reconvergence.
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
        return hasGroupNeedingLowering();
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
    /// Injects stage=MAX_INT stores before breaks and redirects appropriately.
    /// 
    /// For SPIR-V structured control flow, we handle breaks by:
    /// 1. Setting stage=MAX_INT in the break path
    /// 2. NOT redirecting directly to stageMergeBlock (which would violate structured exits)
    /// 3. Instead, letting the break flow to a common merge point with fallthrough
    /// 4. The stage guards will skip remaining stages when stage==MAX_INT
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

        // Identify which original blocks have breaks (branch to breakLabel)
        // We need this BEFORE cloning so we can inject stores in the cloned versions.
        HashSet<IRBlock*> blocksWithBreak;
        for (auto block : bodyBlocks)
        {
            auto terminator = block->getTerminator();
            if (!terminator)
                continue;

            for (UInt i = 0; i < terminator->getOperandCount(); i++)
            {
                if (terminator->getOperand(i) == breakLabel)
                {
                    blocksWithBreak.add(block);
                    break;
                }
            }
        }

        // Create a merge block that both break and fallthrough paths will join at.
        // This ensures SPIR-V structured control flow is maintained.
        // From this merge block, we branch to stageMergeBlock.
        auto caseMergeBlock = builder->createBlock();
        caseMergeBlock->insertBefore(insertBeforeBlock);
        builder->setInsertInto(caseMergeBlock);
        builder->emitBranch(stageMergeBlock);

        // Set up redirections for nextCaseBlock only.
        // IMPORTANT: We do NOT redirect breakLabel here because that breaks SPIR-V
        // structured control flow for conditional breaks (if-else where one path breaks).
        // Instead, we handle breakLabel manually after cloning.
        Dictionary<IRBlock*, IRBlock*> redirections;
        if (nextCaseBlock)
        {
            redirections[nextCaseBlock] = caseMergeBlock;
        }

        // Clone with block mapping so we can find cloned versions
        IRCloneEnv cloneEnv;
        for (const auto& [oldBlock, newBlock] : redirections)
        {
            cloneEnv.mapOldValToNew[oldBlock] = newBlock;
        }

        Dictionary<IRBlock*, IRBlock*> oldToNew;
        for (auto block : bodyBlocks)
        {
            if (redirections.containsKey(block))
                continue;

            auto clonedBlock = builder->createBlock();
            clonedBlock->insertBefore(caseMergeBlock);
            cloneEnv.mapOldValToNew[block] = clonedBlock;
            oldToNew[block] = clonedBlock;
        }

        // Clone instructions
        for (auto block : bodyBlocks)
        {
            if (redirections.containsKey(block))
                continue;

            auto clonedBlock = oldToNew[block];
            builder->setInsertInto(clonedBlock);

            for (auto inst : block->getChildren())
            {
                cloneInst(&cloneEnv, builder, inst);
            }
        }

        // Now handle blocks that had breaks. For each such block:
        // 1. Inject _ft_stage = MAX_INT store before the terminator
        // 2. Redirect branches to breakLabel appropriately
        //
        // For SPIR-V structured control flow, we need to be careful with conditional breaks.
        // If a block branches to breakLabel and is the target of a conditional branch,
        // we should redirect to the if-else merge (the other target of the conditional),
        // not directly to caseMergeBlock. This preserves the selection structure.
        // The stage check will handle skipping subsequent code after the break.
        for (auto origBlock : blocksWithBreak)
        {
            IRBlock* clonedBlock = nullptr;
            if (!oldToNew.tryGetValue(origBlock, clonedBlock))
                continue;

            auto terminator = clonedBlock->getTerminator();
            if (!terminator)
                continue;

            // Insert store before the terminator
            builder->setInsertBefore(terminator);
            builder->emitStore(fallthroughStageVar, maxStageValue);

            // Determine where to redirect the break.
            // If this block is the target of a conditional branch (if-else),
            // redirect to the other target (the merge point) to preserve structure.
            // Otherwise, redirect to caseMergeBlock.
            IRBlock* breakRedirectTarget = caseMergeBlock;

            // Check if clonedBlock has a single predecessor that's a conditional branch
            if (clonedBlock->firstUse)
            {
                // Find predecessor blocks that branch to us
                for (auto use = clonedBlock->firstUse; use; use = use->nextUse)
                {
                    auto user = use->getUser();
                    if (auto condBranch = as<IRConditionalBranch>(user))
                    {
                        // This is a conditional branch that targets our block
                        // Find the other target (the if-else merge)
                        auto trueBlock = condBranch->getTrueBlock();
                        auto falseBlock = condBranch->getFalseBlock();
                        
                        IRBlock* otherTarget = nullptr;
                        if (trueBlock == clonedBlock)
                            otherTarget = falseBlock;
                        else if (falseBlock == clonedBlock)
                            otherTarget = trueBlock;
                        
                        if (otherTarget && otherTarget != breakLabel && otherTarget != caseMergeBlock)
                        {
                            // The other target is likely the if-else merge
                            // Redirect our break to there to preserve structure
                            breakRedirectTarget = otherTarget;
                            break;
                        }
                    }
                }
            }

            // Redirect breakLabel references
            for (UInt i = 0; i < terminator->getOperandCount(); i++)
            {
                if (terminator->getOperand(i) == breakLabel)
                {
                    terminator->setOperand(i, breakRedirectTarget);
                }
            }
        }

        // Return the cloned entry block
        IRBlock* clonedEntry = nullptr;
        if (!oldToNew.tryGetValue(entryBlock, clonedEntry))
        {
            redirections.tryGetValue(entryBlock, clonedEntry);
        }
        return clonedEntry;
    }

    /// Perform the transformation.
    void transform()
    {
        collectCases();

        if (cases.getCount() == 0)
            return;

        // Build fallthrough groups and check which need reconvergence lowering
        buildFallthroughGroups();
        checkGroupsForLowering();

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

        // Skip value for selector: -1 means "don't go through second switch"
        auto skipSelectorValue = builder->getIntValue(intType, -1);
        // Default stage is 0 (start of fallthrough sequence)
        auto zeroValue = builder->getIntValue(intType, 0);
        // Sentinel value for early exit (break). See kBreakStageSentinel definition.
        auto maxStageValue = builder->getIntValue(intType, kBreakStageSentinel);

        auto fallthroughSelectorVar = builder->emitVar(intType);
        builder->addNameHintDecoration(fallthroughSelectorVar, UnownedStringSlice("_ft_selector"));
        builder->emitStore(fallthroughSelectorVar, skipSelectorValue);

        auto fallthroughStageVar = builder->emitVar(intType);
        builder->addNameHintDecoration(fallthroughStageVar, UnownedStringSlice("_ft_stage"));
        builder->emitStore(fallthroughStageVar, zeroValue);

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

            // Determine if this case needs to go through the two-switch lowering
            bool needsDispatch = false;
            if (caseInfo.fallthroughGroupIndex >= 0)
            {
                auto& group = fallthroughGroups[caseInfo.fallthroughGroupIndex];
                needsDispatch = group.needsLowering;
            }

            if (needsDispatch)
            {
                // This case is part of a fallthrough group that needs lowering
                auto& group = fallthroughGroups[caseInfo.fallthroughGroupIndex];

                auto caseBlock = builder->createBlock();
                caseBlock->insertBefore(betweenSwitchesBlock);
                caseIndexToFirstSwitchBlock[i] = caseBlock;

                builder->setInsertInto(caseBlock);

                // Set fallthroughSelector to the group's index
                auto groupIndexValue = builder->getIntValue(intType, group.groupIndex);
                builder->emitStore(fallthroughSelectorVar, groupIndexValue);

                // Set fallthroughStage to this case's stage index
                auto stageValue = builder->getIntValue(intType, caseInfo.stageIndex);
                builder->emitStore(fallthroughStageVar, stageValue);

                // Break to between-switches block
                builder->emitBranch(betweenSwitchesBlock);

                // Add case values to first switch (interleaved: value, label)
                for (auto value : caseInfo.values)
                {
                    firstSwitchCaseArgs.add(value);
                    firstSwitchCaseArgs.add(caseBlock);
                }
            }
            else if (caseInfo.fallthroughGroupIndex >= 0)
            {
                // This case is part of a fallthrough group that does NOT need lowering.
                // Keep original fallthrough behavior by pointing directly to original label.
                // The original label's break will still target the original breakLabel,
                // but we need to rewrite those to go to betweenSwitchesBlock.

                // For simplicity, we clone the case body like non-fallthrough cases,
                // but preserve fallthrough by not stopping at the next case block.
                auto caseBlock = builder->createBlock();
                caseBlock->insertBefore(betweenSwitchesBlock);
                caseIndexToFirstSwitchBlock[i] = caseBlock;

                builder->setInsertInto(caseBlock);
                // fallthroughSelector already defaults to -1 (skip), no need to store

                // Find the last case in this group to determine where to stop cloning
                auto& group = fallthroughGroups[caseInfo.fallthroughGroupIndex];
                Index lastCaseInGroup = group.caseIndices[group.caseIndices.getCount() - 1];
                IRBlock* stopBlock = (lastCaseInGroup + 1 < cases.getCount()) 
                    ? cases[lastCaseInGroup + 1].label 
                    : nullptr;

                // Only clone from the first case in the group - subsequent cases will
                // share the cloned body via fallthrough
                if (caseInfo.stageIndex == 0)
                {
                    // Clone the entire group's body (including fallthrough)
                    auto clonedEntry = cloneNonFallthroughCaseBody(
                        caseInfo.label,
                        stopBlock,
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
                        builder->emitBranch(betweenSwitchesBlock);
                    }

                    // Add case values to first switch
                    for (auto value : caseInfo.values)
                    {
                        firstSwitchCaseArgs.add(value);
                        firstSwitchCaseArgs.add(caseBlock);
                    }
                }
                else
                {
                    // Non-first case in a safe fallthrough group.
                    // Clone this case's body separately - when the selector matches
                    // this case value, execution enters here and gets the remaining
                    // fallthrough sequence (from this point to the group's end).
                    IRBlock* nextCaseBlock = (i + 1 < cases.getCount()) ? cases[i + 1].label : nullptr;
                    
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
                        builder->emitBranch(betweenSwitchesBlock);
                    }

                    // Add case values to first switch
                    for (auto value : caseInfo.values)
                    {
                        firstSwitchCaseArgs.add(value);
                        firstSwitchCaseArgs.add(caseBlock);
                    }
                }
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

                auto caseBlock = builder->createBlock();
                caseBlock->insertBefore(betweenSwitchesBlock);
                caseIndexToFirstSwitchBlock[i] = caseBlock;

                builder->setInsertInto(caseBlock);
                // fallthroughSelector already defaults to -1 (skip), no need to store

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

                // Add case values to first switch (interleaved: value, label)
                for (auto value : caseInfo.values)
                {
                    firstSwitchCaseArgs.add(value);
                    firstSwitchCaseArgs.add(caseBlock);
                }
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
            // fallthroughSelector already defaults to -1 (skip), no need to store
            firstSwitchDefaultBlock = builder->createBlock();
            firstSwitchDefaultBlock->insertBefore(betweenSwitchesBlock);
            builder->setInsertInto(firstSwitchDefaultBlock);
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

            // Skip groups that don't need reconvergence lowering
            if (!group.needsLowering)
                continue;

            // Create the case block for this group
            auto groupCaseBlock = builder->createBlock();
            groupCaseBlock->insertBefore(afterSecondSwitchBlock);

            // Use the group index as the case value (0-based)
            auto groupIndexValue = builder->getIntValue(intType, group.groupIndex);
            secondSwitchCaseArgs.add(groupIndexValue);
            secondSwitchCaseArgs.add(groupCaseBlock);

            // Build the if-chain for stages within this group
            // 
            // IMPORTANT: We CLONE the case body blocks rather than branching to them.
            // This is essential because simply branching and rewriting terminators
            // doesn't work - SSA passes will merge/coalesce our structures.
            IRBlock* currentBlock = groupCaseBlock;

            // Validate that stage indices won't collide with our break sentinel.
            // This would only happen with an absurdly large switch (billions of cases).
            SLANG_ASSERT(group.caseIndices.getCount() < kBreakStageSentinel);

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

/// Check if an instruction is "safe" and doesn't require thread reconvergence.
/// Uses an ALLOWLIST approach: only return true if the instruction is explicitly
/// known to be safe. Unknown instructions are considered unsafe.
/// 
/// Safe operations include: basic math, loads, stores, control flow, comparisons,
/// type conversions, and other simple operations that don't have subgroup semantics.
static bool isInstructionSafeForFallthrough(IRInst* inst)
{
    // Block parameters and decorations are always safe
    if (as<IRParam>(inst) || as<IRDecoration>(inst))
        return true;

    auto op = inst->getOp();

    // Allowlist of safe operations
    switch (op)
    {
    // Control flow (terminators)
    case kIROp_UnconditionalBranch:
    case kIROp_ConditionalBranch:
    case kIROp_Switch:
    case kIROp_IfElse:
    case kIROp_Loop:
    case kIROp_Return:
    case kIROp_Unreachable:
    case kIROp_Discard:
    case kIROp_MissingReturn:

    // Memory operations
    case kIROp_Load:
    case kIROp_Store:
    case kIROp_Var:
    case kIROp_Alloca:

    // Pointer/address operations
    case kIROp_GetElementPtr:
    case kIROp_FieldAddress:
    case kIROp_FieldExtract:
    case kIROp_GetElement:
    case kIROp_GetAddress:
    case kIROp_ImageSubscript:
    case kIROp_RWStructuredBufferGetElementPtr:

    // Arithmetic operations
    case kIROp_Add:
    case kIROp_Sub:
    case kIROp_Mul:
    case kIROp_Div:
    case kIROp_IRem:
    case kIROp_FRem:
    case kIROp_Neg:

    // Bitwise operations
    case kIROp_BitAnd:
    case kIROp_BitOr:
    case kIROp_BitXor:
    case kIROp_BitNot:
    case kIROp_Lsh:
    case kIROp_Rsh:

    // Logical operations
    case kIROp_And:
    case kIROp_Or:
    case kIROp_Not:

    // Comparison operations
    case kIROp_Eql:
    case kIROp_Neq:
    case kIROp_Less:
    case kIROp_Greater:
    case kIROp_Leq:
    case kIROp_Geq:

    // Type conversions
    case kIROp_CastIntToFloat:
    case kIROp_CastFloatToInt:
    case kIROp_IntCast:
    case kIROp_FloatCast:
    case kIROp_CastIntToPtr:
    case kIROp_CastPtrToInt:
    case kIROp_CastPtrToBool:
    case kIROp_BitCast:
    case kIROp_Reinterpret:

    // Constructors
    case kIROp_MakeVector:
    case kIROp_MakeMatrix:
    case kIROp_MakeArray:
    case kIROp_MakeStruct:
    case kIROp_MakeTuple:
    case kIROp_MakeOptionalValue:
    case kIROp_MakeOptionalNone:

    // Vector/matrix access
    case kIROp_VectorReshape:
    case kIROp_MatrixReshape:
    case kIROp_Swizzle:
    case kIROp_SwizzleSet:
    case kIROp_SwizzledStore:

    // Misc safe operations
    case kIROp_Select:
    case kIROp_UpdateElement:
    case kIROp_GetTupleElement:
    case kIROp_GetOptionalValue:
    case kIROp_OptionalHasValue:
    case kIROp_LoadFromUninitializedMemory:
    case kIROp_Poison:
    case kIROp_DefaultConstruct:

    // Constants and literals
    case kIROp_IntLit:
    case kIROp_FloatLit:
    case kIROp_BoolLit:
    case kIROp_StringLit:
    case kIROp_VoidLit:

    // Debug info (always safe)
    case kIROp_DebugLine:
    case kIROp_DebugVar:
    case kIROp_DebugValue:
    case kIROp_DebugLocationDecoration:
    case kIROp_DebugSource:
        return true;

    default:
        // Unknown instruction - not on allowlist, considered unsafe
        return false;
    }
}

/// Check if a switch needs lowering due to fallthrough with unsafe operations.
/// Returns true if any fallthrough case contains operations requiring reconvergence.
static bool switchNeedsLowering(IRSwitch* switchInst)
{
    auto breakLabel = switchInst->getBreakLabel();

    // Collect unique case labels in block order
    HashSet<IRBlock*> seenLabels;
    List<IRBlock*> orderedLabels;

    auto func = as<IRGlobalValueWithCode>(switchInst->getParent()->getParent());
    if (!func)
        return false;

    // Add default label to seen set
    auto defaultLabel = switchInst->getDefaultLabel();
    if (defaultLabel)
        seenLabels.add(defaultLabel);

    // Add all case labels to seen set
    UInt caseCount = switchInst->getCaseCount();
    for (UInt i = 0; i < caseCount; i++)
        seenLabels.add(switchInst->getCaseLabel(i));

    // Walk blocks to get labels in order
    for (auto block : func->getBlocks())
    {
        if (seenLabels.contains(block))
            orderedLabels.add(block);
    }

    // Check each consecutive pair for fallthrough with unsafe operations
    for (Index i = 0; i + 1 < orderedLabels.getCount(); i++)
    {
        auto case1 = orderedLabels[i];
        auto case2 = orderedLabels[i + 1];

        if (caseFallsThroughTo(switchInst, case1, case2, breakLabel))
        {
            // Found fallthrough - check if it contains unsafe operations
            if (fallthroughPathNeedsLowering(switchInst, case1, case2, breakLabel))
                return true;
        }
    }

    return false;
}

/// Determine if a switch should be lowered.
/// Only lower switches that have fallthrough cases with operations that require
/// reconvergence (e.g., wave operations). Simple fallthrough with only safe
/// operations (math, loads, stores) doesn't need lowering.
/// 
/// TODO: In the future, we may want to also restrict this to SPIR-V targets only,
/// since other targets may handle reconvergence correctly in hardware.
static bool shouldLowerSwitch(IRSwitch* switchInst, TargetProgram* targetProgram)
{
    SLANG_UNUSED(targetProgram);

    return switchNeedsLowering(switchInst);
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
