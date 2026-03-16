---
name: slang-test-development
description: Quick reference for writing Slang compiler tests (.slang files with //TEST directives). Use when writing individual tests, adding test cases, or needing test syntax help. For comprehensive coverage analysis of a feature, use the slang-test-coverage skill instead.
---

# Slang Test Development

Quick reference for test syntax and patterns. For systematic coverage analysis, see the `slang-test-coverage` skill.

## Quick Reference

### Test File Location
Place tests under `tests/` directory, organized by category:
- `tests/language-feature/` - Language features (generics, interfaces, lambdas, etc.)
- `tests/compute/` - Compute shader tests
- `tests/diagnostics/` - Error message tests
- `tests/bugs/` - Bug regression tests

### Running Tests

**Important**: `slang-test` must be run from the repository root directory.

```bash
# Run specific test
./build/Release/bin/slang-test tests/path/to/test.slang

# Run all tests in a directory
./build/Release/bin/slang-test tests/language-feature/dynamic-dispatch/

# Run with multiple servers (faster)
./build/Release/bin/slang-test -use-test-server -server-count 8
```

## Choosing a Test Type

- **"Does this code produce the right output?"** → `COMPARE_COMPUTE` with `-cpu`
- **"Does this code compile to correct target code?"** → `SIMPLE(filecheck=CHECK)` with `-target spirv`
- **"Does this code produce the right error/warning?"** → `DIAGNOSTIC_TEST:SIMPLE(diag=CHECK)`
- **"Does this code run correctly without GPU?"** → `INTERPRET`
- **"Does this code work on multiple backends?"** → Multiple `COMPARE_COMPUTE` lines with different targets

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
- `-shaderobj` - Use shader-object based parameter binding (preferred for new tests)
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
- Default (exhaustive): test fails if any diagnostic lacks an annotation
- `non-exhaustive` option: only checks annotated diagnostics, ignores extras
```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):-target spirv
```

**Block comments** for early columns:
```slang
if (x == y);
/*CHECK:
^ don't use if here
          ^ empty statement
*/
```

See `docs/diagnostics.md` for full details on the diagnostic annotation system.

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
   ./build/Release/bin/slang-test tests/your/test.slang
   ```

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

- **Coverage methodology**: Use the `slang-test-coverage` skill for 7-phase workflow, gap analysis, and test value scoring
- **Diagnostic test annotations**: See `docs/diagnostics.md` for DIAGNOSTIC_TEST directives, caret matching, exhaustive/non-exhaustive modes
- **Command-line options**: See `tools/slang-test/README.md`
- **Debugging**: See `CLAUDE.md` for `-dump-ir` usage
