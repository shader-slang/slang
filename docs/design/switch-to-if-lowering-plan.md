# Switch Reconverged Lowering Design

## Problem Statement

SPIR-V's `OpSwitch` has undefined reconvergence behavior for non-trivial fallthroughs (cases that fall through and contain wave operations). This can cause incorrect behavior when different lanes take different paths through a switch with fallthrough.

See GitHub issue #6441 for details: https://github.com/shader-slang/slang/issues/6441

The SPV_KHR_maximal_reconvergence extension guarantees reconvergence at structured control flow merge points (like if-else merges), but `OpSwitch` fallthrough is NOT structured—threads can enter the same code from different case labels.

## Solution: Two-Switch Transformation

We split a switch with fallthrough into two switches that provide deterministic reconvergence:

### First Switch (Dispatch Switch)
- Each case sets two variables: `fallthroughSelector` and `fallthroughStage`
- ALL cases break (no fallthrough in the IR structure)
- Cases in the same fallthrough group get the same selector value
- The stage indicates where in the fallthrough sequence to start

### Second Switch (Execution Switch)
- Dispatches on `fallthroughSelector`
- One case per fallthrough group
- Each case contains if-guarded stages: `if (fallthroughStage <= N) { ... }`
- This ensures all threads in a fallthrough group reconverge at each if-merge

### Non-Fallthrough Cases
- Get `selector = MIN_INT` (a sentinel value)
- Their code executes directly after the first switch dispatch
- They do NOT go through the second switch's case structure
- The second switch's default immediately branches to finalBreak

## Example

### Original Code (with conditional break)

```hlsl
switch(val) {
  case 0:
    a();
    if (cond) break;
    a2();      // fallthrough to case 1
  case 1:
    b();
    break;
  case 2:
    c();
    break;
  default:
    d();
    break;
}
```

### Transformed IR Structure

```
// First Switch - Dispatch
switch(val, secondSwitchEntry, newDefault, 0:dispatch0, 1:dispatch1, 2:dispatch2)

dispatch0:
  fallthroughSelector = 0  // Group 0: case 0 and case 1
  fallthroughStage = 0     // Start at stage 0
  branch(secondSwitchEntry)

dispatch1:
  fallthroughSelector = 0  // Same group as case 0
  fallthroughStage = 1     // Start at stage 1
  branch(secondSwitchEntry)

dispatch2:
  fallthroughSelector = MIN_INT  // Non-fallthrough
  branch(case2Body)              // Execute directly

case2Body:
  c();
  branch(secondSwitchEntry)

newDefault:
  fallthroughSelector = MIN_INT  // Non-fallthrough
  branch(defaultBody)

defaultBody:
  d();
  branch(secondSwitchEntry)

// Second Switch - Execution
secondSwitchEntry:
switch(fallthroughSelector, finalBreak, finalBreak, 0:fallthroughGroup0)

fallthroughGroup0:
  // Stage 0: case 0's code
  if (fallthroughStage <= 0) {
    a();
    if (cond) {
      fallthroughStage = MAX_INT;  // Skip remaining stages
    } else {
      a2();
    }
  }
  // Stage 1: case 1's code
  if (fallthroughStage <= 1) {
    b();
  }
  branch(finalBreak)

finalBreak:
  ... rest of function ...
```

## Key Invariants

1. First switch's break label becomes the second switch entry
2. Fallthrough cases: dispatch blocks set selector/stage, branch to second switch
3. Non-fallthrough cases: dispatch blocks set selector=MIN_INT, branch to cloned body
4. Non-fallthrough case bodies branch to second switch entry when done
5. Second switch routes selector values to fallthrough groups
6. Second switch's default = break = finalBreak (MIN_INT goes straight to break)
7. `break` in fallthrough cases becomes `fallthroughStage = MAX_INT`
8. All control flow converges at finalBreak

## Implementation Details

### Fallthrough Detection

Cases are considered to "fall through" if they share blocks—detected by checking if any blocks reachable from case A can also reach case B (excluding the break label and other case labels).

### Block Cloning

Case body blocks are **cloned** rather than rewritten in place. This is essential because:
- SSA optimization passes would otherwise merge dispatch blocks with case bodies
- The optimizer's register allocator could coalesce selector/stage variables with unrelated variables
- Cloning creates distinct IR that the optimizer treats separately

### Handling Loops Inside Cases

Loops inside case bodies are preserved as-is. However:
- Branches inside loops that target the switch's break label are rewritten to set `fallthroughStage = MAX_INT`
- Loop-internal breaks (exiting the loop, not the switch) remain unchanged
- The pass runs `eliminateContinueBlocksInFunc` first to normalize continue statements

### Transformation Criteria

Currently, the pass transforms all switches with fallthrough for all targets. Future refinement will restrict this to SPIRV-only targets where the reconvergence issue exists.

The `needsTransformation()` predicate checks for fallthrough only. Default cases are not considered as triggers because the Slang frontend sometimes transforms variable initialization into a default case, and drivers handle default case reconvergence correctly.

## Pass Placement

In `slang-emit.cpp`, the pass runs:
1. After `eliminatePhis` (so IR uses explicit variables, not block parameters)
2. After `simplifyCFG` (so empty intermediate blocks are fused)
3. Before `eliminateMultiLevelBreak` (which handles any multi-level breaks we create)

## Files

- `source/slang/slang-ir-lower-switch-to-reconverged-switches.h` - Header
- `source/slang/slang-ir-lower-switch-to-reconverged-switches.cpp` - Implementation
- `tests/language-feature/switch-fallthrough/reconvergence/` - Tests

## Historical Note

An earlier design considered using a `do { } while(false)` wrapper with cumulative predicates. The two-switch approach was chosen instead because it:
- Preserves switch dispatch semantics in the IR (better for non-SPIRV targets)
- Provides clearer reconvergence points at each stage's if-merge
- Avoids creating synthetic loop constructs that could confuse optimizers
