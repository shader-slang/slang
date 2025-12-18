# Implementation Plan: Lower to IRVar for Fall-Through Cases

## Status: In Progress

## Overview

When lowering a `switch` statement, detect variables that are used in case blocks where fall-through from a previous case can occur. For these variables, use `IRVar` (memory-based storage) instead of on-the-fly SSA values to ensure correct semantics and preserve fall-through structure in output code.

---

## Progress Summary

- [x] Phase 1: Create tests for fall-through variable semantics
- [x] Phase 2: Commit tests
- [x] Phase 3: Pre-scan switch body for fall-through detection
- [x] Phase 4: Add NoSSAPromotionDecoration to IR instruction set
- [x] Phase 5: Make SSA pass respect NoSSAPromotionDecoration
- [x] Phase 6: Decorate IRVars affected by fall-through during lowering
- [ ] Phase 7: Modify restructure pass to preserve fall-through structure
- [ ] Phase 8: Verify tests pass

---

## Completed Work

### Phase 1-2: Tests Created ✓

Created comprehensive test suite in `tests/language-feature/switch-fallthrough/`:

**Trivial Fall-Through Tests (passing now):**
- `trivial-fallthrough-case-grouping.slang` - Basic case grouping
- `trivial-fallthrough-with-default.slang` - Case grouped with default  
- `trivial-fallthrough-var-different-init.slang` - Variables with different init per case

**True Fall-Through Tests (disabled until restructure pass is fixed):**
- `basic-fallthrough-var.slang` - Basic variable modification across fall-through
- `fallthrough-chain.slang` - Chain of fall-through cases
- `fallthrough-to-default.slang` - Fall-through from case to default
- And more...

### Phase 3: Pre-Scan Switch Body ✓

Added to `source/slang/slang-lower-to-ir.cpp`:

1. **`SwitchStmtInfo` updated** with:
   - `HashSet<VarDeclBase*> fallThroughVars` - Variables needing IRVar treatment
   - `HashSet<Index> fallThroughTargetIndices` - Case indices receiving fall-through

2. **Helper functions added**:
   - `stmtTerminatesCase()` - Checks if stmt definitively terminates (break/return/etc.)
   - `CaseClauseInfo` struct - Tracks case label, body statements, termination status
   - `extractCaseClauses()` - Extracts case clauses from switch body
   - `FallThroughVarCollector` - AST visitor to collect assigned/used variables
   - `detectFallThroughAndCollectVars()` - Main pre-scan function

3. **Detection working correctly**:
   - Properly identifies which cases fall through
   - Identifies variables assigned in source case and used in target case
   - Debug output shows: "Fall-through detection: found 1 fall-through target indices, 1 variables"

### Phase 4-5: NoSSAPromotionDecoration ✓

1. **Added new IR decoration** in `source/slang/slang-ir-insts.lua`:
   ```lua
   noSSAPromotion = {
       -- Mark a variable (IRVar) as not promotable to SSA form.
       -- Used for variables that need to remain as memory due to fall-through
       -- in switch statements, so the fall-through structure is preserved.
       struct_name = "NoSSAPromotionDecoration",
   },
   ```

2. **Added stable name** in `source/slang/slang-ir-insts-stable-names.lua`:
   ```lua
   ["Decoration.noSSAPromotion"] = 718
   ```

3. **Updated SSA pass** in `source/slang/slang-ir-ssa.cpp`:
   - `isPromotableVar()` now checks for `IRNoSSAPromotionDecoration`
   - If decoration is present, variable is NOT promoted to SSA (remains as memory)

### Phase 6: Decorate IRVars During Lowering ✓

In `visitSwitchStmt()`, after detecting fall-through variables:
1. Look up each variable's corresponding `IRVar` in the environment
2. Apply `kIROp_NoSSAPromotionDecoration` to prevent SSA promotion

**Result verified** - IR dump shows decoration applied:
```
[noSSAPromotion]
[nameHint("sum")]
let %sum : Ptr(UInt) = var
```

---

## The Problem: Why This Approach is Needed

### Background: On-The-Fly SSA in Lowering

The Slang lowering (`slang-lower-to-ir.cpp`) uses "on-the-fly SSA" for local variables. Instead of always creating an `IRVar` and later promoting to SSA via `constructSSA`, it:

1. **Tracks constant values directly** in the lowering context
2. **When control flow merges** (e.g., at a switch case reachable via multiple paths), creates **block parameters** to handle different incoming values
3. This is efficient but **destroys the fall-through structure**

### Current IR Structure (After Our Changes)

The IR structure is now correct for fall-through:
```
switch(%val, %breakLabel, %defaultLabel, 1:UInt, %case1Block)

block %case1Block:
    store(%sum, 1:UInt)              // stores to var
    unconditionalBranch(%defaultLabel)  // FALLS THROUGH to default!

block %defaultLabel:
    let %tmp = load(%sum)            // loads current value
    let %result = add(%tmp, 1:UInt)
    store(%sum, %result)
    unconditionalBranch(%breakLabel)
```

**Case 1 now branches DIRECTLY to default's label!** This is the correct structure for fall-through.

### Remaining Issue: Restructure Pass

The restructure pass (`slang-ir-restructure.cpp`) doesn't recognize this as fall-through.

**Current behavior** (lines 559-562):
```cpp
if (caseIndex < caseCount)
{
    caseEndLabel = switchInst->getCaseLabel(caseIndex);
}
```

This only sets `caseEndLabel` to the next *explicit* case label. When processing the last explicit case (case 1), there's no "next case" in the list, so `caseEndLabel` is `nullptr`.

When `generateRegionsForIRBlocks` encounters the unconditional branch from case 1 to default, it treats default's block as **part of case 1's body** rather than a fall-through destination.

**Result**: The HLSL output duplicates code:
- case 1: contains `sum=1; sum+=1; break;` (default's code inlined)
- default: contains `sum+=1; break;`

---

## Phase 7: Modify Restructure Pass (TODO)

**Location**: `source/slang/slang-ir-restructure.cpp`

### Option A: Add Fall-Through to LabelStack

Add a new `LabelStack::Op::Fallthrough` operation. For each case:
1. Register the next case label (or default label) as a potential fall-through target
2. When processing case body and encountering a branch to fall-through label, emit a `FallthroughRegion` instead of inlining

### Option B: Modify caseEndLabel Logic

When setting `caseEndLabel`:
1. If no more explicit cases, check if this case should fall through to default
2. Set `caseEndLabel = defaultLabel` if appropriate

This requires knowing at IR level which cases are supposed to fall through.

### Option C: Use IR Decoration

Add a decoration to switch/case blocks indicating fall-through. The restructure pass can then respect this decoration.

### Recommendation

Option B is simplest but requires IR-level information about fall-through intent.
Option A is more principled but requires adding new region types.
Option C is cleanest from a design perspective.

---

## Files Modified

| File | Changes |
|------|---------|
| `source/slang/slang-lower-to-ir.cpp` | Pre-scan phase, IRVar detection, decoration application |
| `source/slang/slang-ir-insts.lua` | Added NoSSAPromotionDecoration |
| `source/slang/slang-ir-insts-stable-names.lua` | Added stable name for decoration |
| `source/slang/slang-ir-ssa.cpp` | Check for NoSSAPromotionDecoration in isPromotableVar() |
| `source/slang/slang-ir-restructure.cpp` | (TODO) Recognize fall-through patterns |

---

## Testing Strategy

1. Enable the disabled fall-through tests one at a time
2. Run with `-dump-ir` to verify IR structure
3. Check HLSL output preserves fall-through structure
4. Verify semantic correctness (compute results match expected)

---

## Debug Output (to be removed before commit)

Currently the lowering prints debug output showing fall-through detection:
```
[DEBUG] Extracted 2 case clauses from switch
[DEBUG]   Clause 0: 1 body stmts, terminates=0
[DEBUG]   Clause 1: 2 body stmts, terminates=1
[DEBUG] Fall-through detection: found 1 fall-through target indices, 1 variables
[DEBUG]   var: sum
```

This should be removed before merging.
