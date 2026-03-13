# Quick Reference: Coverage-Driven Test Development

**For**: AI agents and developers analyzing coverage reports to create high-quality tests

**Input**: 
- Slang source file (e.g., `source/slang/slang-check-conformance.cpp`)
- Coverage report for that file (HTML or text showing covered/uncovered lines)

**Goal**: Increase test coverage of the source file by creating high-quality, non-redundant tests

**Core Principle**: Write tests that verify behavior and document intent, not tests that just increase coverage numbers.

---

## Getting Started

1. **Open the coverage report**: View HTML coverage for your source file
2. **Identify uncovered lines**: Look for red/uncovered lines in the report
3. **Note function names**: Identify which functions have gaps
4. **Follow the 5-step process below** for each uncovered function

**Example**:
```
Coverage report: coverage-html/source/slang/slang-check-conformance.cpp.html
Current coverage: 86%
Uncovered functions: isInterfaceSafeForTaggedUnion (lines 10-19)
```

---

## The 5-Step Process

### Step 1: Verify Code Is Reachable

```bash
grep -rn "functionName" source/
```

**Decision**:
- Only definition appears → **DEAD CODE** → STOP (file issue, don't test)
- Multiple call sites → Reachable → Continue

**Red flags**: Related tests `DISABLED`, `TODO` comments, incomplete features

---

### Step 2: Check Existing Coverage

```bash
find tests/ -name "*feature*.slang"
grep -r "pattern" tests/ --include="*.slang"
```

**Assess coverage quality**:
- **Explicit**: Clear test of this functionality → Likely SKIP
- **Incidental**: Tested as side effect → Maybe (continue to Step 3)
- **None**: Not covered → Continue

**Consider scope**: Core operations (buffers, interfaces, generics) are heavily tested everywhere

---

### Step 3: Evaluate Test Value

**Score each criterion (0-2 points, total /10)**:

| Criterion | 0 pts | 1 pt | 2 pts |
|-----------|-------|------|-------|
| **Coverage** | Explicitly tested | Incidentally covered | Not covered |
| **Clarity** | Existing tests clear | Existing unclear | No existing tests |
| **Errors** | Errors well-tested | Only success tested | Errors untested |
| **Docs** | Well-documented | Minimal docs | No documentation |
| **Risk** | Low regression risk | Medium risk | High risk |

**Decision**:
- **0-4**: SKIP (redundant or low value)
- **5**: Maybe (only if test would be notably clearer)
- **6-10**: WRITE TEST

**Critical question**: Would this test be notably clearer than existing alternatives?

---

### Step 4: Design Functional Test

**Transform coverage target into functional requirement**:

❌ "Test line 245 in slang-check-conformance.cpp"  
✅ "Verify extracting existential values preserves interface conformance"

**Test types**:
- **Compute**: Functional verification with output checking (`COMPARE_COMPUTE`)
- **Diagnostic**: Error message validation (`DIAGNOSTIC_TEST`, `CHECK: error`)
- **Interpret**: CPU-only semantic tests (`INTERPRET`)

**Test must**:
- Verify behavior (check outputs/errors), not just execute code
- Stand alone (no external dependencies)
- Document intent clearly (comments explaining what/why/how)
- Include edge cases where relevant
- **Run on multiple backends/platforms when possible** (see below)

**Cross-platform best practices**:
- Test on both CPU and GPU when applicable:
  ```slang
  //TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj
  //TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj
  //TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj
  ```
- Diagnostic tests work on all platforms by default
- GPU-only tests: Add to `tests/expected-failure-no-gpu.txt` for macOS CI
- Platform-specific features: Use appropriate test guards

**Template**:
```slang
// Test that [specific behavior being verified]
// Related: source/slang/file.cpp:lines or proposal-NNN

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj

[Setup interfaces/types]

[Function exercising feature]

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain() {
    // CHECK: [expected]
    outputBuffer[0] = testFunction();
}
```

---

### Step 5: Implement and Validate

**Implementation**:
1. Place near related tests (`tests/language-feature/`, `tests/diagnostics/`)
2. Follow existing patterns (study 2-3 similar tests first)
3. Match naming convention: `feature-description.slang` or `error-condition.slang`

**Validation**:
```bash
# Run your test (tests all backends specified in test directives)
./build/RelWithDebInfo/bin/slang-test tests/path/to/test.slang

# Run full suite (ensure no breakage)
./build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 8
```

**Self-review questions**:
- Is there an existing test just as good? (If yes → DELETE)
- Would I understand this in 6 months? (If no → IMPROVE)
- Does this test justify its maintenance cost? (If no → DELETE)
- Does it run on multiple backends/platforms? (If no, why not?)

---

## Quick Decision Guide

**SKIP if**:
- ❌ Code unreachable (dead code)
- ❌ Heavily tested (core operations: buffers, basic interfaces, etc.)
- ❌ Test would be redundant (not notably clearer than existing)
- ❌ Value score <5

**WRITE if**:
- ✅ Reachable code + no existing coverage (score ≥6)
- ✅ Error cases untested (score ≥6)
- ✅ Existing tests unclear + your test notably better (score ≥6)

---

## Examples

**SKIP - Dead Code**:
```bash
$ grep -rn "isInterfaceSafeForTaggedUnion" source/
source/slang/file.cpp:10: Function definition
# No callers → SKIP
```

**SKIP - Redundant**:
```
Target: ConstantBuffer extraction
Check: 500+ tests use ConstantBuffer<T>
Value: 2/10 (heavily tested)
Decision: SKIP
```

**WRITE - Valuable**:
```
Target: ExtractExistentialType conformance
Check: Only incidental coverage in autodiff tests
Score: Coverage=1, Clarity=2, Errors=1, Docs=1, Risk=1 = 6/10
Decision: WRITE (clearer than alternatives)
```

---

## Anti-Patterns

1. **Coverage %-driven**: Writing tests to hit percentage targets
2. **Dead code testing**: Not checking reachability first
3. **Test duplication**: Not checking existing coverage
4. **Execution-only tests**: Tests that run but don't verify behavior
5. **Unclear intent**: Test purpose not obvious

---

## Checklist

**Before writing**:
- [ ] Verified code is reachable (grep showed callers)
- [ ] Checked existing tests (find/grep showed coverage)
- [ ] Scored value ≥5/10
- [ ] Designed functional test (behavior, not lines)

**After writing**:
- [ ] Test passes with expected output
- [ ] Runs on CPU and GPU backends (when applicable)
- [ ] Full test suite passes
- [ ] Not redundant with existing
- [ ] Clear documentation
- [ ] Honest: "Would I want this test?"

---

## Key Commands

```bash
# Check reachability
grep -rn "functionName" source/

# Find related tests
find tests/ -name "*feature*.slang"
grep -r "pattern" tests/ --include="*.slang"

# Run tests
./build/RelWithDebInfo/bin/slang-test tests/path/test.slang
./build/RelWithDebInfo/bin/slang-test -use-test-server -server-count 8
```

---

## Expected Outcomes

**Successful coverage increase**:
- Created tests that exercise previously uncovered code
- Coverage percentage increases
- Tests verify behavior and document intent

**No coverage increase** (also valuable):
- Discovered dead code (file issue, don't test)
- Found existing comprehensive coverage (mature test suite)
- Identified redundant test attempts (saved maintenance burden)

**Remember**: Sometimes the most valuable outcome is understanding **why** coverage can't be increased (dead code, already well-tested, etc.)

---

**For detailed methodology**: See `tmp/check-conformance-tests/COVERAGE-DRIVEN-TEST-DEVELOPMENT-GUIDE.md`
