# Code Coverage Tools

This directory contains tools for generating and analyzing code coverage reports for the Slang compiler.

## Prerequisites

- LLVM coverage tools (`llvm-profdata`, `llvm-cov`) installed

## Quick Start (HTML report)

```bash
cmake --preset coverage
cmake --build --preset coverage
COVERAGE_HTML=1 ./tools/coverage/run-coverage.sh
# Report is at ./coverage-html/index.html
open ./coverage-html/index.html
```

## Command-Line Options and Environment Variables

- `--report-only` - Generate reports from existing coverage data without re-running tests. Requires that coverage data was collected previously (i.e., `build/coverage-data/slang-test.profdata` exists).

- `COVERAGE_HTML=1` - Generate HTML coverage report
- `COVERAGE_HTML_DIR` - Output directory for HTML report (default: `coverage-html/`)
- `COVERAGE_LCOV=1` - Generate LCOV format report
- `COVERAGE_LCOV_FILE` - Output file for LCOV report (default: `coverage.lcov`)
- `COVERAGE_DIR` - Directory for raw coverage data (default: `build/coverage-data/`)
- `BUILD_DIR` - Override build directory (default: `build`)
- `CONFIG` - Build configuration (default: `RelWithDebInfo`)
- `LLVM_PROFDATA` - Path to llvm-profdata tool (optional)
- `LLVM_COV` - Path to llvm-cov tool (optional)

## Output Files

All temporary coverage data is stored in `build/coverage-data/` by default to keep the repository root clean:

- `build/coverage-data/slang-test-*.profraw` - Raw profile data (automatically cleaned up after merging)
- `build/coverage-data/slang-test.profdata` - Merged and indexed profile data (kept for reuse)
- `coverage-html/` - HTML coverage report (if `COVERAGE_HTML=1`, in repo root by default)
- `coverage.lcov` - LCOV format report (if `COVERAGE_LCOV=1`, in repo root by default)

## Analyzing Coverage

### View coverage for specific file:
```bash
llvm-cov show ./build/RelWithDebInfo/lib/libslang.so \
    -instr-profile=slang-test.profdata \
    source/slang/slang-parser.cpp
```

### Get function-level coverage:
```bash
llvm-cov report ./build/RelWithDebInfo/lib/libslang.so \
    -instr-profile=slang-test.profdata \
    -show-functions
```

On macOS, use `xcrun llvm-cov` instead of plain `llvm-cov`.

## Troubleshooting

### No coverage data found
- Ensure binaries were built with coverage enabled
- Check that `*.profraw` files are being generated
- Verify the binary has coverage symbols: `nm build/RelWithDebInfo/lib/libslang.* | grep __llvm_profile`

### Mismatched data warnings
- Rebuild all binaries after enabling coverage
- Delete old `*.profraw` files before running tests
