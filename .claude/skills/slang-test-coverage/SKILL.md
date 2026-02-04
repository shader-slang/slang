---
name: slang-test-coverage
description: Comprehensive test coverage analysis and gap-filling for Slang language features. Use when the user wants to increase test coverage, maximize testing for a feature, perform coverage analysis, identify test gaps, or systematically test all aspects of a language feature.
---

# Feature-Oriented Test Coverage

**For**: Systematic test coverage improvement for specific Slang language features

**Core Principle**: Write tests that verify behavior and document intent, not tests that just increase coverage numbers.

## Workflow Overview: 7 Phases

1. **Understanding** → Read reference docs, extract concepts/restrictions/error conditions
2. **Analysis** → Inventory existing tests, check coverage reports, verify code reachability
3. **Gap Identification** → Compare reference vs tests, prioritize gaps, score test value
4. **Test Design** → Transform coverage targets into functional requirements
5. **Implementation** → Create tests, run and validate, ensure cross-platform compatibility
6. **Bug Investigation** → Investigate failures, document bugs, fix if straightforward
7. **Documentation** → Update coverage analysis, create summary

---

## Phase 1: Understanding

```bash
# Find existing docs
rg "feature-keyword" docs/ examples/
```

**Extract from reference**: Core concepts, syntax, restrictions, error conditions

**Create** `tmp/feature-name/README.md` with feature overview, concepts, behaviors, restrictions.

---

## Phase 2: Analysis

```bash
# Find related tests
find tests/ -name "*feature-name*" -type f
rg "feature-keyword" tests/ --files-with-matches

# Verify code reachability
grep -rn "functionName" source/
```

**Reachability decision**:
- Only definition appears → **DEAD CODE** → STOP (file issue, don't test)
- Multiple call sites → Reachable → Continue

**Create** `tmp/feature-name/test-coverage.md` categorizing: Basic functionality, Error handling, Edge cases, Integration.

---

## Phase 3: Gap Identification

### Checklist

**For each capability, check**:
- [ ] Positive case tested?
- [ ] Negative/error case tested?
- [ ] Edge cases tested?
- [ ] All targets tested? (cpu, cuda, vk, hlsl, metal, wgsl)

**For interface-typed variables/parameters/return-values**:
- [ ] Direct interface type
- [ ] Composite containing interface (struct, array, tuple)

**For function return values**:
- [ ] Direct return
- [ ] Return via `out`/`inout` parameter

### Prioritize Gaps

- **HIGH**: Core functionality untested, error cases missing
- **MEDIUM**: Edge cases partial
- **LOW**: Rare combinations

### Evaluate Test Value (score 0-10)

| Criterion | 0 pts | 1 pt | 2 pts |
|-----------|-------|------|-------|
| **Coverage** | Explicitly tested | Incidentally covered | Not covered |
| **Clarity** | Existing tests clear | Existing unclear | No existing tests |
| **Errors** | Errors well-tested | Only success tested | Errors untested |
| **Docs** | Well-documented | Minimal docs | No documentation |
| **Risk** | Low regression risk | Medium risk | High risk |

**Decision**: 0-4 SKIP, 5 Maybe, 6-10 WRITE

---

## Phase 4: Test Design

### Requirements

- ❌ "Test line 245 in file.cpp" → ✅ "Verify behavior X preserves property Y"
- Verify behavior (check outputs/errors), not just execute
- Run on all targets: `-cpu`, `-cuda`, `-vk`, `-hlsl`, `-metal`, `-wgsl`
- Cover variable positions: local, parameter, return, struct field, array element, global

### Compute Test Template

```slang
// Test: [specific behavior]
// Gap: [which gap this addresses]

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgsl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -shaderobj

[Setup interfaces/types]

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain() {
    // CHECK: [expected]
    outputBuffer[0] = testFunction();
}
```

### Diagnostic Test Template

```slang
// Test: [what error should occur]
// Gap: [which gap this addresses]

//TEST:SIMPLE(filecheck=CHECK): -target spirv

[Code that should trigger error]

// CHECK: ([[# @LINE+1]]): error [number]
badCode();
```

---

## Phase 5: Implementation

**Placement**: `tests/language-feature/` or `tests/diagnostics/`
**Naming**: `feature-scenario.slang` or `diagnose-error-condition.slang`

```bash
# Run test
./build/Release/bin/slang-test tests/path/to/test.slang

# Run full suite
./build/Release/bin/slang-test -use-test-server -server-count 8
```

**Self-review**: Is there an existing test just as good? Would I understand this in 6 months? Does it run on all targets?

---

## Phase 6: Bug Investigation

**Decision tree**:
```
Test fails
├─ Wrong expected value → Fix test
├─ Wrong setup → Fix test
├─ Behavior matches docs → Fix test
└─ Behavior violates docs → BUG → Document → Fix if possible
```

**Fix only if**: Root cause clear, fix straightforward, follows existing patterns, can validate no regressions.

```bash
# Locate and fix
rg "functionName" source/

# Validate
cmake --build build --config Release --target slangc
./build/Release/bin/slang-test tests/path/failing-test.slang
./build/Release/bin/slang-test -use-test-server -server-count 8
```

---

## Phase 7: Documentation

Update `tmp/feature-name/test-coverage.md` with final status.

Create `tmp/feature-name/SUMMARY.md`:
- Tests created (list with descriptions)
- Bugs found (list with status)
- Validation results

---

## Decision Rules

### WRITE test if:
- ✅ Reference describes behavior but no test exists (score ≥6)
- ✅ Error case should trigger but untested
- ✅ Existing tests unclear + your test notably better

### SKIP test if:
- ❌ Code unreachable (dead code)
- ❌ Same scenario already tested
- ❌ Heavily tested core operations
- ❌ Value score <5

### Investigate as BUG if:
- Behavior violates documented semantics
- Error should trigger but doesn't

---

## Anti-Patterns

1. **Coverage %-driven**: Writing tests to hit percentage targets
2. **Dead code testing**: Not checking reachability first
3. **Test duplication**: Not checking existing coverage
4. **Execution-only tests**: Tests that run but don't verify behavior

---

## Output Structure

```
tmp/feature-name/                    # Working directory (do NOT check in)
├── README.md          # Feature overview
├── test-coverage.md   # Analysis with gap status
└── SUMMARY.md         # Final results

tests/language-feature/feature-name/ # Test files (check in)
├── scenario-1.slang
├── scenario-2.slang
└── diagnose-error-case.slang
```

**Note**: The `tmp/` directory is for your reference during the coverage work and is gitignored. Only the test files under `tests/` should be committed.

---

## Core Principles

1. **Quality over quantity** - 5 excellent tests > 20 mediocre
2. **Test failures are valuable** - They reveal bugs or gaps in understanding
3. **Verify everything** - Run tests, check outputs, validate fixes
4. **No coverage increase is also valuable** - Dead code or already well-tested

**Goal**: Ensure features work correctly in all documented scenarios and fail gracefully in error cases.
