---
name: slang-run-tests
description: Platform-aware test runner for the Slang compiler. Knows which targets work on each OS, detects skipped-as-pass false positives, and handles SPIRV validation. Referenced by other skills.
---

# Slang Run Tests

**For**: Running Slang compiler tests with platform awareness.

**Usage**: Referenced by other skills. Can also be invoked directly: `/slang-run-tests [test-path]`

---

## Running Tests

**Important**: `slang-test` must run from the repository root directory.

```bash
# Run a specific test
./build/<preset>/bin/slang-test tests/path/to/test.slang

# Run all tests in a directory
./build/<preset>/bin/slang-test tests/language-feature/generics/

# Run full suite with parallel servers
./build/<preset>/bin/slang-test -use-test-server -server-count 8
```

Where `<preset>` is `Debug`, `RelWithDebInfo`, or `Release` matching your build (see `slang-build` skill).

---

## Platform-Aware Target Selection

Not all targets work on every platform. Before running tests, know what will actually execute:

| Target flag | macOS | Linux | Windows/WSL |
|-------------|-------|-------|-------------|
| `-cpu` | yes | yes | yes |
| `-vk` | limited | yes | yes |
| `-cuda` | **no** | yes | yes |
| `-dx12` | **no** | **no** | yes |
| `-metal` | yes | **no** | **no** |
| `-wgsl` | yes | yes | yes |

### Critical: Detect Skipped Tests

**A skipped test is NOT a passed test.** On platforms that lack a backend (e.g., macOS + CUDA),
`slang-test` silently skips the test and reports success. This can hide real failures.

After running tests, always check the output for skip counts:

```
Total: 100, Passed: 60, Failed: 0, Skipped: 40
```

**If the skip count is high relative to total**, verify that the tests you care about
actually ran. For target-specific fixes (SPIRV, CUDA, D3D), skipped tests mean
**you cannot validate locally** — leave it to CI.

When writing new tests for GPU-less environments, prefer `-cpu` or `INTERPRET` test types.

---

## SPIRV Validation

For SPIRV-related work, enable validation:

```bash
SLANG_RUN_SPIRV_VALIDATION=1 ./build/<preset>/bin/slangc -target spirv test.slang
```

Do NOT use the system's `spirv-val` tool — it may be outdated. Slang bundles its own.

To see SPIRV output even when validation fails:

```bash
./build/<preset>/bin/slangc -target spirv-asm -skip-spirv-validation test.slang
```

To generate reference SPIRV via GLSL for comparison:

```bash
./build/<preset>/bin/slangc -target spirv-asm -emit-spirv-via-glsl test.slang
```

---

## Test Types at a Glance

| Question | Test Type | GPU Required? |
|----------|-----------|---------------|
| Does this produce correct output? | `COMPARE_COMPUTE` with `-cpu` | No |
| Does this compile to correct target code? | `SIMPLE(filecheck=CHECK)` | No |
| Does this produce the right error? | `DIAGNOSTIC_TEST:SIMPLE(diag=CHECK)` | No |
| Does this run correctly? | `INTERPRET` | No |
| Does this work on GPU backends? | `COMPARE_COMPUTE` with `-vk`/`-cuda`/`-dx12` | Yes |

For test syntax details, see the `slang-write-test` skill.

---

## Troubleshooting

### Test not found
- File must be under `tests/` directory
- Extension must be `.slang`
- Must run from repo root

### All tests skipped
- Check platform capabilities table above
- Use `-cpu` for platform-independent testing

### FileCheck failures
- Run with verbose output to see mismatches
- Check exact whitespace and formatting in CHECK lines

### Binary not found
- Build first: see `slang-build` skill
- Verify preset matches: `ls build/<preset>/bin/slang-test`
