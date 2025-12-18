# Implementation Plan: Lower to IRVar for Fall-Through Cases

## Status: In Progress

## Overview

When lowering a `switch` statement, detect variables that are used in case blocks where fall-through from a previous case can occur. For these variables, use `IRVar` (memory-based storage) instead of `IRParam` (SSA block parameters) to ensure correct semantics across fall-through boundaries.

---

## Progress

- [x] Phase 1: Create tests for fall-through variable semantics
- [ ] Phase 2: Analyze fall-through during lowering
- [ ] Phase 3: Modify variable lowering for fall-through cases
- [ ] Phase 4: Scope and lifetime handling
- [ ] Phase 5: Verify tests pass

### Phase 1 Complete

Created comprehensive test suite in `tests/language-feature/switch-fallthrough/`:

**Trivial Fall-Through Tests (passing now):**
- `trivial-fallthrough-case-grouping.slang` - Basic case grouping
- `trivial-fallthrough-with-default.slang` - Case grouped with default  
- `trivial-fallthrough-var-different-init.slang` - Variables with different init per case

**True Fall-Through Tests (disabled until implemented):**
- `basic-fallthrough-var.slang` - Basic variable modification across fall-through
- `fallthrough-chain.slang` - Chain of fall-through cases
- `fallthrough-to-default.slang` - Fall-through from case to default
- `fallthrough-var-declared-in-case.slang` - Variable scoping with fall-through
- `fallthrough-mixed-break-and-fallthrough.slang` - Mix of break and fall-through
- `fallthrough-nested-switch.slang` - Nested switch with fall-through
- `fallthrough-with-conditional.slang` - Conditional logic in fall-through cases
- `fallthrough-loop-interaction.slang` - Loops in fall-through cases
- `fallthrough-multiple-vars.slang` - Multiple variables across fall-through
- `fallthrough-only-some-vars.slang` - Optimization: only some vars need IRVar
- `fallthrough-break-label.slang` - Variable access after switch (at break label)

---

## Phase 1: Create Tests

### Test Cases to Create

1. **Basic fall-through with variable use** - Variable initialized before switch, modified in case 0, used in case 1 (fall-through)
2. **Variable declared in case, used after fall-through** - Variable declared in case 0, used in case 1
3. **Multiple fall-through chain** - Variable flows through cases 0 → 1 → 2
4. **Mixed: some variables with fall-through, some without** - Some variables only used in one case, others span fall-through
5. **Fall-through to default case**

---

## Phase 2: Analyze Fall-Through During Lowering

**Location**: `source/slang/slang-lower-to-ir.cpp`

### 2.1 Add Fall-Through Detection to SwitchStmtInfo (~line 7540)

```cpp
struct SwitchStmtInfo
{
    // ... existing fields ...
    
    // Track which case blocks can be reached via fall-through
    HashSet<IRBlock*> fallThroughTargets;
    
    // Track variables that need IRVar treatment due to fall-through
    HashSet<VarDeclBase*> fallThroughVars;
};
```

### 2.2 Pre-Scan Switch Body for Fall-Through

Before emitting case blocks, walk the switch body to identify:
1. Which cases fall through to other cases (no `break`/`return`/etc. before next `case`)
2. Which variables are referenced in fall-through target cases

---

## Phase 3: Modify Variable Lowering for Fall-Through Cases

**Location**: `source/slang/slang-lower-to-ir.cpp`

### 3.1 Update Variable Declaration Lowering

When lowering a variable declaration inside a switch:
- Check if the current case block is a fall-through target
- If so, AND the variable is used after a potential fall-through point, use `IRVar` instead of `IRParam`

### 3.2 Update getLabelForCase (~line 7559)

When creating a new case block that follows a non-terminated previous block:
- Mark the new block as a fall-through target in `SwitchStmtInfo`
- Track that fall-through is occurring

---

## Phase 4: Scope and Lifetime Handling

### 4.1 Ensure IRVar is Scoped to Switch

The `IRVar` should be emitted at the switch entry block (before the switch instruction), not inside the case block, so it's visible across all cases.

### 4.2 Update Variable Lookup

When looking up a variable during lowering, check if it's a switch-scoped var.

---

## Files to Modify

| File | Changes |
|------|---------|
| `source/slang/slang-lower-to-ir.cpp` | Add fall-through detection, modify variable lowering |

---

## Notes

(Add implementation notes here as work progresses)

