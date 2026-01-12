# Switch-to-If Lowering Plan

## Problem Statement

SPIR-V's `OpSwitch` has undefined reconvergence behavior for non-trivial fallthroughs (cases that fall through and contain wave operations). This can cause incorrect behavior when different lanes take different paths through a switch with fallthrough.

## Proposed Approach

Lower switch statements with non-trivial fallthrough to a series of if-statements wrapped in a `do { ... } while(false)` construct. This provides deterministic reconvergence at the cost of losing the O(1) jump table optimization of `OpSwitch`.

### Core Transformation

Given:
```hlsl
switch(selector) {
  case 0:
    first();
    break;
  case 4:
    bool cond = checkSomething();
    if (cond)
      break;
    // fallthrough into default
  default:
    something();
  case 1:
  case 3:
    somethingElse();
    break;
  case 2:
    anotherThing();
    break;
}
```

Transform to:
```hlsl
do {
  // Pre-compute base predicates for each case group
  bool isCase0 = selector == 0;
  bool isCase4 = selector == 4;
  bool isCase1or3 = (selector == 1) | (selector == 3);
  bool isCase2 = selector == 2;
  bool isDefault = !(isCase1or3 | isCase2);  // inverse of cases AFTER default
  
  // Cumulative predicates: include all cases that can reach this one
  // (If a predecessor breaks, we exit before reaching this check anyway)
  bool reachesDefault = isCase4 | isDefault;
  bool reachesCase1or3 = reachesDefault | isCase1or3;
  
  // case 0: nothing falls through to here
  if (isCase0) {
    first();
    break;
  }
  
  // case 4: conditional break, can fall through into default
  if (isCase4) {
    bool cond = checkSomething();
    if (cond)
      break;
    // fallthrough (no explicit tracking needed)
  }
  
  // default: reached by case 4 fallthrough OR direct match
  if (reachesDefault) {
    something();
    // fallthrough (no explicit tracking needed)
  }
  
  // case 1, case 3: reached by default fallthrough OR direct match
  if (reachesCase1or3) {
    somethingElse();
    break;
  }
  
  // case 2: nothing falls through to here
  if (isCase2) {
    anotherThing();
    break;
  }
} while(false)
```

### Logic Error in Original Proposal

llvm-beanz's Feb 25 comment used `!(selector == 1 && selector == 2 && selector == 3)` for the default condition. This is always true (a value can't equal 1, 2, AND 3 simultaneously). The correct condition is `!((selector == 1) | (selector == 2) | (selector == 3))`.

### Key Design Decisions

1. **Use `|` instead of `||`**: No short-circuit evaluation needed for combining case predicates—all predicates are pure comparisons.

2. **Cumulative predicates instead of runtime fallthrough tracking**: Each case's predicate includes all predecessor cases that can fall through to it. If a predecessor breaks, we exit before reaching this case's check anyway—so including it is harmless. If a predecessor falls through, the predicate correctly triggers. This eliminates the need for a runtime `bool ft` variable.

3. **Pre-computed predicates**: Compute predicates for each case group up front, then build cumulative predicates for cases that can be reached via fallthrough. This makes the generated IR easier to understand. Downstream compilers will optimize placement.
   ```hlsl
   // Base predicates
   bool isCase0 = selector == 0;
   bool isCase4 = selector == 4;
   bool isCase1or3 = (selector == 1) | (selector == 3);
   bool isCase2 = selector == 2;
   bool isDefault = !(isCase1or3 | isCase2);  // inverse of cases AFTER default
   
   // Cumulative predicates (include predecessors that can fall through)
   bool reachesDefault = isCase4 | isDefault;
   bool reachesCase1or3 = reachesDefault | isCase1or3;
   ```

4. **`do { } while(false)` for break handling**: `break` statements become breaks from the do-loop, naturally exiting the transformed switch.

## Scope

- **Target**: All targets initially (for testing), then SPIRV-only once validated
- **Placement**: Late in the IR transform stack, after most optimizations
- **Trigger**: Determined by a predicate function (see below)
- **Preserve**: Switches that don't meet the trigger criteria remain as native switch

### Rollout Strategy

1. **Development**: Enable for all targets unconditionally
   - Allows testing on CPU backend (easiest to debug)
   - Allows comparing output across HLSL, GLSL, Metal, SPIRV
   - Run full test suite to validate correctness

2. **Final**: Restrict to SPIRV only
   - Only apply when `isKhronosTarget(targetRequest)` or similar
   - Other targets keep native switch for performance

### Transformation Criteria

The decision to lower a switch is made by a predicate function with this signature:

```cpp
// TODO: Refine this predicate. Currently returns true unconditionally.
// Future refinements may include:
// - Only lower switches with non-trivial fallthrough (non-empty case bodies that fall through)
bool shouldLowerSwitchToIf(IRSwitch* switchInst, TargetRequest* targetRequest)
{
    // PLACEHOLDER: Lower all switches for now to enable testing on all targets.
    // Final version should restrict to SPIRV:
    //   if (!isKhronosTarget(targetRequest))
    //       return false;
    return true;
}
```

This design keeps the door open for refining the criteria without restructuring the pass.

## Edge Cases to Handle

| Case | Handling |
|------|----------|
| `continue` inside switch (inside loop) | Continue targets outer loop, not the do-while. Handled by `eliminateMultiLevelBreak`. |
| Nested switches | Each gets its own do-while and predicates |
| Loops inside switch | Work normally—contained within case body |
| Switch inside loop | The do-while is inside the loop; break/continue semantics preserved |
| Default not at end | Process cases in source order; default condition excludes all explicit cases |
| Empty case (label only) | Combine with next case's predicate via `|` |

## Implementation Plan

### Phase 1: Detection
- Add analysis pass to identify switches with non-trivial fallthrough
- A "trivial" fallthrough is an empty case body (just a label)

### Phase 2: IR Transformation
- Create loop-like IR construct for do-while(false)
- Build base predicates for each case group:
  - `isCase<N> = (selector == val1) | (selector == val2) | ...`
  - `isDefault = !(cases after default ORed together)`
- Build cumulative predicates by walking cases in source order:
  - For each case, include all predecessor cases that fall through to it
  - If a predecessor breaks, it's harmless (we exit before reaching this check)
- For each case in source order:
  - Transform case body to if-block using appropriate predicate
  - If case breaks: keep break (exits do-while)
  - If case falls through: no special handling needed (cumulative predicate handles it)

### Pass Placement in `slang-emit.cpp`

Insert **before** `eliminateMultiLevelBreak`:

```cpp
SLANG_PASS(performForceInlining);

if (emitSpirvDirectly)
{
    SLANG_PASS(performIntrinsicFunctionInlining);
}

// NEW: Switch-to-if lowering (creates synthetic loops that may have multi-level breaks)
SLANG_PASS(lowerSwitchToIf, targetProgram);

// Existing: Handles multi-level breaks introduced by our transformation
SLANG_PASS(eliminateMultiLevelBreak, targetProgram);
```

This ordering ensures:
1. Our synthetic `IRLoop` is created
2. Any `continue` statements that now cross our loop boundary become multi-level
3. `eliminateMultiLevelBreak` handles them automatically

### Phase 3: Testing
- Unit tests for predicate generation
- Integration tests with wave operations
- Nesting tests (switch-in-switch, switch-in-loop, loop-in-switch)
- CPU backend tests for logic correctness
- SPIRV validation tests

## Continue Statement Handling

**The Problem**: When we wrap a switch in `do { } while(false)`, if that `do-while` is represented as an `IRLoop`, then `continue` statements inside could incorrectly target the synthetic loop instead of an outer enclosing loop.

**Background from codebase analysis**:
- `IRSwitch` has a `breakLabel` but no continue block (switches don't support `continue`)
- `IRLoop` has both `breakBlock` and `continueBlock`
- `continue` in IR is just an unconditional branch to the loop's registered continue block
- Existing utilities: `eliminateContinueBlocks()` (in `slang-ir-loop-unroll.cpp`), `eliminateMultiLevelBreak()` (in `slang-ir-eliminate-multilevel-break.cpp`)

### Solution: Rely on Existing `eliminateMultiLevelBreak` Infrastructure

The existing `eliminateMultiLevelBreak` pass already handles this exact case. From `slang-ir-eliminate-multilevel-break.cpp` lines 137-140:

```cpp
// Add continueBlock to the exitBlocks stack so nested constructs
// (e.g., switch with continue) treat it as an exit point.
if (info.continueBlock)
    exitBlocks.add(info.continueBlock);
```

The comment explicitly mentions "switch with continue" - our exact scenario.

**How it works**:
1. We create a synthetic `IRLoop` for do-while(false)
2. Any existing `continue` in the original switch body already targets the outer loop's continue block
3. After transformation, those continues are inside our synthetic loop but still target the outer loop
4. This makes them "multi-level continues" (branch crosses our synthetic loop boundary)
5. The existing `eliminateMultiLevelBreak` pass (already run in `slang-emit.cpp` before emission) detects and handles them automatically

**Advantages**:
- Zero new code for continue handling
- Battle-tested infrastructure
- Handles all edge cases (deeply nested continues, multiple levels, etc.)

**No disadvantages** - this is the clean solution

## Implementability Review

### ✅ Available Infrastructure

| Need | Available | Location |
|------|-----------|----------|
| Fallthrough detection | `caseBlocksFallThroughTo()` | `slang-ir-restructure.cpp:86-140` |
| Switch IR structure | `IRSwitch` with `getCaseCount()`, `getCaseValue()`, `getCaseLabel()` | `slang-ir-insts.h:2028-2046` |
| Loop creation | `IRBuilder::emitLoop(target, breakBlock, continueBlock)` | `slang-ir.cpp:5682-5690` |
| If-else creation | `IRBuilder::emitIfElse()`, `emitIf()` | `slang-ir.cpp:5725-5774` |
| Comparison emission | `IRBuilder::emitEql()`, `emitNeq()`, `emitOr()`, `emitNot()` | `slang-ir.cpp` |
| Multi-level break handling | `eliminateMultiLevelBreak()` | `slang-ir-eliminate-multilevel-break.cpp` |
| Block iteration | `IRSwitch::getCaseCount()`, iteration patterns | Throughout codebase |

### ✅ Key Implementation Patterns

1. **Creating do-while(false)**:
   ```cpp
   auto loopHeaderBlock = builder.createBlock();
   auto breakBlock = switchInst->getBreakLabel();
   // continueBlock == loopHeaderBlock for do-while behavior
   builder.emitLoop(loopHeaderBlock, breakBlock, loopHeaderBlock);
   ```

2. **Building predicates**:
   ```cpp
   auto selector = switchInst->getCondition();
   auto caseVal = switchInst->getCaseValue(i);
   auto pred = builder.emitEql(selector, caseVal);
   // Combine multiple values: pred = builder.emitOr(pred1, pred2);
   ```

3. **Detecting fallthrough**:
   ```cpp
   HashSet<IRBlock*> visited;
   bool fallsThrough = caseBlocksFallThroughTo(
       caseLabel, nextCaseLabel, breakLabel, visited);
   ```

### ⚠️ Implementation Considerations

1. **Case ordering**: `IRSwitch` stores cases as (value, label) pairs but doesn't preserve source order. Need to reconstruct order from block layout or use `defaultLabel` position.

2. **Block cloning/moving**: Case bodies are existing blocks. Options:
   - Clone blocks into the new structure (safer but duplicates code)
   - Re-wire existing blocks (more efficient but complex)
   - Recommendation: Re-wire since predicates handle the new entry conditions

3. **Break handling**: Existing breaks target `switchInst->getBreakLabel()`. After transformation:
   - Our do-while's break block should BE the original switch's break label
   - This way, existing break branches remain valid

4. **Predicate computation order**: Must process cases in source order to build cumulative predicates. May need to sort case labels by block order in the function.

### ❌ Potential Issues

1. **Default label finding "cases after default"**: The IR doesn't directly encode source order. Solution: Walk blocks in function order, match against case labels.

2. **Empty cases (label grouping)**: Multiple case values can point to the same label. Already handled by `IRSwitch::getCaseLabel()` returning the same block.

3. **Nested switches**: Each switch transformation is independent. The do-while wrapper is local to each switch.

## IR Transformation Pseudocode

The key insight is that we **cannot simply branch to the original case body blocks**. The original blocks have terminators (branch to next case for fallthrough, or branch to breakLabel for break) that violate SPIRV structured control flow when placed inside if-else constructs.

**Solution**: Clone/inline the case body instructions into new structured blocks.

### Input IR Structure

```python
# Original IR (example: case 0 falls through to default, default breaks)
switch_inst:
    condition: %selector
    break_label: breakBlock
    default_label: defaultBlock
    cases: [(0, case0Block)]

case0Block:
    %1 = wave_op(1)
    store(%sum, %1)
    branch(defaultBlock)      # <-- fallthrough terminator

defaultBlock:
    %2 = wave_op(10)
    store(%sum, %2)
    branch(breakBlock)        # <-- break terminator

breakBlock:
    ... rest of function ...
```

### Output IR Structure (after transformation)

```python
# Transformed IR
parentBlock:
    loop(loopBodyBlock, breakBlock, loopBodyBlock)

loopBodyBlock:
    # Predicates computed inside loop body (at the top)
    %pred0 = icmp_eq(%selector, 0)
    %predDefault = or(%pred0, %isDefault)  # reaches default via fallthrough OR direct
    
    # First case check
    ifElse(%pred0, thenBlock0, mergeBlock0, mergeBlock0)

thenBlock0:
    # CLONED instructions from case0Block (excluding terminator)
    %1 = wave_op(1)
    store(%sum, %1)
    # case0 falls through, so we just continue to merge (no break)
    branch(mergeBlock0)

mergeBlock0:
    # Check if we reach default (via fallthrough OR direct match)
    ifElse(%predDefault, thenBlockDefault, mergeBlockDefault, mergeBlockDefault)

thenBlockDefault:
    # CLONED instructions from defaultBlock (excluding terminator)
    %2 = wave_op(10)
    store(%sum, %2)
    # default breaks, so we exit the loop
    branch(breakBlock)

mergeBlockDefault:
    # All checks done, fall off the end
    branch(breakBlock)

breakBlock:
    ... rest of function (unchanged) ...
```

### Transformation Algorithm

```python
def transform_switch(switch_inst):
    parent_block = switch_inst.parent
    break_label = switch_inst.break_label
    selector = switch_inst.condition
    
    # Step 1: Collect cases in source order (analyze fallthrough)
    cases = collect_cases_in_source_order(switch_inst)
    
    # Step 2: Create the loop wrapper
    loop_body = create_block()
    emit_loop(target=loop_body, break_block=break_label, continue_block=loop_body)
    
    # Step 3: Build predicates inside loop body (at the top)
    set_insert_point(loop_body)
    
    for i, case in enumerate(cases):
        # Direct predicate: selector matches this case's values
        case.direct_pred = emit_case_predicate(selector, case.values)
        
        # Reach predicate: direct match OR reached via fallthrough from earlier case
        case.reach_pred = case.direct_pred
        for j in range(i):
            if cases[j].falls_through:
                case.reach_pred = emit_or(case.reach_pred, cases[j].direct_pred)
    
    # Step 4: Build the if-chain
    for i, case in enumerate(cases):
        then_block = create_block()
        merge_block = create_block()
        
        # Emit: if (reach_pred) { cloned_body } else { skip }
        emit_if_else(case.reach_pred, then_block, merge_block, merge_block)
        
        # Clone case body into then_block
        set_insert_point(then_block)
        clone_block_contents(case.body_block, exclude_terminator=True)
        
        # Determine how to exit then_block
        if case_always_breaks(case.body_block, break_label):
            # Case breaks: branch to break_label (exits loop)
            emit_branch(break_label)
        else:
            # Case falls through: branch to merge (continue to next check)
            emit_branch(merge_block)
        
        # Continue building in merge_block
        set_insert_point(merge_block)
    
    # Step 5: After all cases, branch to break (unreachable if all paths covered)
    emit_branch(break_label)
    
    # Step 6: Remove original switch and case blocks
    switch_inst.remove()
    for case in cases:
        if case.body_block.has_no_uses():
            case.body_block.remove()


def collect_cases_in_source_order(switch_inst):
    """
    Walk the function's blocks in order, identify which are case labels.
    Return list of CaseInfo in the order they appear.
    """
    cases = []
    seen_labels = set()
    
    # Collect all case labels and default
    for i in range(switch_inst.case_count):
        seen_labels.add(switch_inst.get_case_label(i))
    seen_labels.add(switch_inst.default_label)
    
    # Walk blocks in function order
    for block in switch_inst.parent.parent.blocks:
        if block in seen_labels:
            # Find which case values point here
            values = []
            for i in range(switch_inst.case_count):
                if switch_inst.get_case_label(i) == block:
                    values.append(switch_inst.get_case_value(i))
            
            is_default = (block == switch_inst.default_label)
            
            cases.append(CaseInfo(
                block=block,
                values=values,
                is_default=is_default,
                falls_through=detect_fallthrough(block, cases, switch_inst.break_label)
            ))
    
    return cases


def emit_case_predicate(selector, values):
    """
    Emit predicate: (selector == val1) | (selector == val2) | ...
    For default: inverse of all cases AFTER this one.
    """
    if not values:  # default case with no explicit values
        # Computed later after we know which cases come after
        return None
    
    pred = emit_icmp_eq(selector, values[0])
    for val in values[1:]:
        cmp = emit_icmp_eq(selector, val)
        pred = emit_or(pred, cmp)
    return pred


def detect_fallthrough(block, earlier_cases, break_label):
    """
    Check if this block falls through to the next case (vs breaking).
    """
    terminator = block.terminator
    if is_branch(terminator):
        target = terminator.target
        # Falls through if target is not the break label and not a case we've seen
        return target != break_label
    elif is_conditional_branch(terminator):
        # Complex: could conditionally fall through
        # For safety, assume fallthrough if ANY path doesn't go to break
        return not all_paths_reach(block, break_label)
    return False


def clone_block_contents(source_block, exclude_terminator):
    """
    Clone all instructions from source_block to current insert point,
    excluding the terminator (branch/break).
    """
    for inst in source_block.instructions:
        if exclude_terminator and is_terminator(inst):
            continue
        clone_inst(inst)
```

### Why This Works for SPIRV

1. **Each if-else has its own merge block**: No sharing of merge blocks between constructs.

2. **All paths from within an if-else go to its merge**: 
   - True branch either breaks (→ breakLabel) or continues (→ merge)
   - False branch always goes to merge

3. **The loop provides the break target**: `branch breakLabel` inside the loop exits correctly.

4. **Wave reconvergence**: All threads see the same sequence of if-else constructs. Wave operations execute at the same program point regardless of which path each thread took.

### Key Insight: Clone, Don't Branch

The critical difference from the failed approach:
- **Failed**: Branch TO original case blocks, which have unstructured terminators
- **Correct**: Clone case body instructions INTO new structured blocks with correct terminators

This ensures:
- Each if-else is self-contained with proper merge semantics
- Original case blocks can be deleted (no dangling references)
- SPIRV structured control flow is maintained

## Open Questions

1. ~~For SPIRV structured control flow: does the synthetic `IRLoop` emit correctly?~~ 
   **Resolved**: Yes, but we must clone case body contents rather than branching to original blocks.

2. For cases with complex internal control flow (if-else, loops inside case), we need to clone entire subgraphs, not just individual instructions. May need `cloneRegion()` or similar.

## Files to Investigate

- `source/slang/slang-ir-insts.h` - Switch IR representation
- `source/slang/slang-ir-restructure.cpp` - Has `caseBlocksFallThroughTo()` utility
- `source/slang/slang-ir-eliminate-multilevel-break.cpp` - Pattern for breakable region handling
- `source/slang/slang-emit-spirv.cpp` - Current OpSwitch emission
