---
name: slang-write-test
description: Quick reference for writing Slang compiler tests (.slang files with //TEST directives). Use when writing individual tests, adding test cases, or needing test syntax help. For comprehensive coverage analysis of a feature, use the slang-analyze-coverage skill instead.
---

# Slang Test Development

Quick reference for test syntax and patterns. For systematic coverage analysis, see the `slang-analyze-coverage` skill.

## Quick Reference

### Test File Location
Place tests under `tests/` directory, organized by category:
- `tests/language-feature/` - Language features (generics, interfaces, lambdas, etc.)
- `tests/compute/` - Compute shader tests
- `tests/diagnostics/` - Error message tests
- `tests/bugs/` - Bug regression tests

### Running Tests

See the `slang-run-tests` skill for platform-aware test running, skip detection, and SPIRV validation.
See the `slang-build` skill for building `slang-test` on your platform.

Quick reference (run from repo root):

```bash
./build/<preset>/bin/slang-test tests/path/to/test.slang
```

## Choosing a Test Type

- **"Does this code produce the right output?"** → `COMPARE_COMPUTE` with `-cpu`
- **"Does this code compile to correct target code?"** → `SIMPLE(filecheck=CHECK)` with `-target spirv`
- **"Does this code produce the right error/warning?"** → `DIAGNOSTIC_TEST:SIMPLE(diag=CHECK)`
- **"Does this code run correctly without GPU?"** → `INTERPRET`
- **"Does this code work on multiple backends?"** → Multiple `COMPARE_COMPUTE` lines with different targets
- **"Does this constraint/restriction actually reject invalid code?"** → `DIAGNOSTIC_TEST` companion (see Negative Testing below)

## Test Types

### 1. Compute Tests (Most Common)
Test shader execution and compare output values.

```slang
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = 42.0;
    // CHECK: 42.0
}
```

**Key elements:**
- `filecheck-buffer=CHECK` - Use FileCheck to verify buffer contents
- `-cpu` - Run on CPU (no GPU required)
- `-vk` - Run on Vulkan
- `-output-using-type` - Print typed values
- `-shaderobj` - Use shader-object-based parameter binding (preferred for new tests)
- `//TEST_INPUT:` - Declare input/output buffers

### 2. Simple Compilation Tests
Test that code compiles and verify generated output (SPIRV, HLSL, etc.).

```slang
//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

// Your shader code here

// CHECK: someExpectedOutput
```

### 3. Diagnostic Tests (Error/Warning Verification)
Test that specific errors or warnings are produced. Use `DIAGNOSTIC_TEST` with caret-based annotations.

```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

int foo = undefined;
//CHECK:  ^^^^^^^^^ undeclared identifier
```

**Caret-based matching**: Carets align with source columns on the preceding non-annotation line.

**Matching fields** — annotations can match against:
- Message text: `//CHECK: ^^^^^^^^^ undeclared identifier`
- Severity: `//CHECK: ^ error` or `//CHECK: ^ warning`
- Error code: `//CHECK: ^ E20101`
- Combined: `//CHECK: ^ warning E20101`

**Exhaustive vs non-exhaustive**:
- Default (exhaustive): test fails if any diagnostic lacks an annotation.
  **Prefer exhaustive mode** -- it catches unexpected diagnostic changes.
- `non-exhaustive` option: only checks annotated diagnostics, ignores extras.
  Use ONLY when the compiler emits additional cascading diagnostics that are
  not the focus of the test and may change between versions.
- The test harness **rejects** `non-exhaustive` when all diagnostics are
  already matched by annotations. Never use it "just in case".
```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):-target spirv
```

**Duplicate CHECK lines**: When the compiler emits the same diagnostic
twice (e.g., initial checking + re-checking phase), add one CHECK per
emission. Add a brief comment at the top of the file explaining why
duplicates are expected.

**Block comments** for early columns:
```slang
if (x == y);
/*CHECK:
^ don't use if here
          ^ empty statement
*/
```

See `docs/diagnostics.md` for full details on the diagnostic annotation system.

### Negative Testing for Constrained Features

When a positive test exercises a **constrained feature** (interface
conformance, where clauses, generic constraints, typealias constraints),
always create a companion negative diagnostic test that verifies the
compiler **rejects** constraint violations.

Without the negative test, the constraint could be silently ignored and
the positive test would still pass. This was flagged in PR review for
generic typealias tests that only tested valid types but never verified
that invalid types were rejected.

**Pattern**: Create a `-negative` companion file:

```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface IMyInterface { int getValue(); }
struct Wrapper<T : IMyInterface> { T inner; }
typealias Wrapped<T : IMyInterface> = Wrapper<T>;

struct NotConforming { int data; }

void test()
{
    Wrapped<NotConforming> w;
/*CHECK:
    ^^^^^^^^^^^^^^ type argument doesn't conform to interface
    ^^^^^^^^^^^^^^ type argument 'NotConforming' does not conform to the required interface 'IMyInterface'
*/
}
```

**Naming convention**:
- `feature-scenario.slang` (positive functional test)
- `feature-scenario-negative.slang` (negative diagnostic companion)

### 4. Interpreter Tests (No GPU)
For testing without any GPU backend.

```slang
//TEST:INTERPRET(filecheck=CHECK):

void main() {
    // CHECK: expected output
}
```

## Buffer Declaration Patterns

### Output Buffer
```slang
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;
```

### Input Buffer
```slang
//TEST_INPUT: set inputBuffer = ubuffer(data=[1 2 3 4], stride=4)
StructuredBuffer<int> inputBuffer;
```

### Alternative Syntax
```slang
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;
```

## FileCheck Patterns

### Basic Check
```slang
outputBuffer[0] = 42.0;
// CHECK: 42.0
```

### Check Pattern (regex)
```slang
// CHECK: {{.*}} generated {{.*}} dispatch code
```

### Named Check Groups
```slang
//TEST:SIMPLE(filecheck=REPORT): -report-dynamic-dispatch-sites
// REPORT: dispatch code
```

## Common Test Patterns

### Testing Language Version
```slang
#lang slang 2025

// Modern Slang features here
```

### Multiple Targets
```slang
//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target cuda
```

### Disabled Test
```slang
//DISABLE_TEST:COMPARE_COMPUTE: -cpu
```

### Test Categories
```slang
//TEST(smoke,compute):COMPARE_COMPUTE: -cpu
```

## Creating a New Test

1. **Choose test type** using the decision tree above

2. **Create file** in appropriate directory under `tests/`

3. **Add test directive** at the top

4. **Add CHECK comments** for expected output

5. **Run test** to verify:
   ```bash
   ./build/RelWithDebInfo/bin/slang-test tests/your/test.slang
   ```

## Pre-submission Checklist

Before committing any test file, verify:

1. **Filename matches content**: The filename must describe what the test
   actually verifies. If it tests "no applicable generic", name it
   `diagnose-no-applicable-generic.slang`, not `diagnose-existential.slang`.

2. **Comments match code**: Verify all interface names, error codes, and
   behavior descriptions in comments match the actual code. If a comment
   says "requires T to conform to IArithmetic" but the constraint is
   `IValueProvider`, the comment is wrong.

3. **No dead code**: Every declared function, struct, or variable must be
   called or used in the test. Remove or exercise unused declarations.

4. **No duplicate tests**: Search existing tests before adding new ones:
   ```bash
   rg "keyword" tests/language-feature/<feature>/ --files-with-matches
   ```
   If the same scenario is already tested, extend the existing test
   instead of creating a duplicate.

5. **Feature support verified**: For functional tests, confirm the feature
   compiles before writing the full test. Run a quick `slangc` check.
   Do not write tests for unsupported or unimplemented features.

6. **Negative companion exists**: If the test exercises a constrained
   feature (interface conformance, where clause, generic constraint),
   verify a companion `-negative` diagnostic test exists that proves
   the constraint is enforced by rejecting invalid types/values.

7. **Backend coverage**: Add `//TEST` lines for all applicable backends,
   not just one. If the feature is target-independent, test at minimum
   `-cpu` and `-spirv`. If it's target-specific, test the relevant
   target plus `-cpu` as a baseline. Use the platform capabilities table
   in `slang-run-tests` to know which targets run locally vs CI-only.

8. **Run the test**: Every test must pass locally before committing.

## Troubleshooting

### Test not found
- Ensure file is under `tests/` directory
- Check file extension is `.slang`

### FileCheck failures
- Run with `-v` for verbose output
- Check exact whitespace and formatting

### GPU tests failing
- Use `-cpu` for CPU-only testing
- Check GPU driver availability

## Additional Resources

- **Coverage methodology**: Use the `slang-analyze-coverage` skill for 7-phase workflow, gap analysis, and test value scoring
- **Diagnostic test annotations**: See `docs/diagnostics.md` for DIAGNOSTIC_TEST directives, caret matching, exhaustive/non-exhaustive modes
- **Command-line options**: See `tools/slang-test/README.md`
- **Debugging**: See `CLAUDE.md` for `-dump-ir` usage
