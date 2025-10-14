# Code Coverage Tools

This directory contains tools for generating and analyzing code coverage reports for the Slang compiler.

## Prerequisites

- LLVM coverage tools (`llvm-profdata`, `llvm-cov`) installed

## Quick Start

### 1. Build with Coverage

```bash
# Configure with coverage preset
cmake --preset coverage

# Build
cmake --build --preset coverage
```

Or manually:

```bash
cmake --preset coverage
cmake --build --preset coverage
```

### 2. Run Tests with Coverage

```bash
# From repository root
./tools/coverage/run-coverage.sh slang-unit-test-tool/

# Run specific tests
./tools/coverage/run-coverage.sh tests/language-feature/

# Generate HTML report
COVERAGE_HTML=1 ./tools/coverage/run-coverage.sh slang-unit-test-tool/

# Generate LCOV format (for CI/CD integration)
COVERAGE_LCOV=1 ./tools/coverage/run-coverage.sh slang-unit-test-tool/

# Regenerate reports from existing coverage data (without re-running tests)
COVERAGE_HTML=1 ./tools/coverage/run-coverage.sh --report-only
```

## Command-Line Options

- `--report-only` - Generate reports from existing coverage data without re-running tests. Requires that coverage data was collected previously (i.e., `build/coverage-data/slang-test.profdata` exists).

## Environment Variables

- `COVERAGE_HTML=1` - Generate HTML coverage report
- `COVERAGE_HTML_DIR` - Output directory for HTML report (default: `coverage-html/`)
- `COVERAGE_LCOV=1` - Generate LCOV format report
- `COVERAGE_LCOV_FILE` - Output file for LCOV report (default: `coverage.lcov`)
- `COVERAGE_DIR` - Directory for raw coverage data (default: `build/coverage-data/`)
- `BUILD_DIR` - Override build directory (default: `build`)
- `CONFIG` - Build configuration (default: `RelWithDebInfo`)
- `LLVM_PROFDATA` - Path to llvm-profdata tool (optional)
- `LLVM_COV` - Path to llvm-cov tool (optional)

## Platform Support

### macOS
Uses `xcrun` to automatically find the correct LLVM tools from Xcode Command Line Tools.

### Linux
Requires `llvm-profdata` and `llvm-cov` to be in PATH or set via environment variables.

Install on Ubuntu/Debian:
```bash
sudo apt-get install llvm
```

Install on Fedora/RHEL:
```bash
sudo dnf install llvm
```

### Using Homebrew LLVM (macOS/Linux)
```bash
brew install llvm
export LLVM_PROFDATA=/opt/homebrew/opt/llvm/bin/llvm-profdata
export LLVM_COV=/opt/homebrew/opt/llvm/bin/llvm-cov
```

## Manual Workflow

If you prefer to run coverage steps manually:

```bash
# 1. Set profile output location
export LLVM_PROFILE_FILE="slang-test-%p.profraw"

# 2. Run tests
./build/RelWithDebInfo/bin/slang-test slang-unit-test-tool/

# 3. Merge coverage data (macOS)
xcrun llvm-profdata merge -sparse slang-test-*.profraw -o slang-test.profdata

# 3. Merge coverage data (Linux)
llvm-profdata merge -sparse slang-test-*.profraw -o slang-test.profdata

# 4. Generate report (macOS)
xcrun llvm-cov report ./build/RelWithDebInfo/lib/libslang.dylib -instr-profile=slang-test.profdata

# 4. Generate report (Linux)
llvm-cov report ./build/RelWithDebInfo/lib/libslang.so -instr-profile=slang-test.profdata

# 5. Generate HTML report (macOS)
xcrun llvm-cov show ./build/RelWithDebInfo/lib/libslang.dylib \
    -instr-profile=slang-test.profdata \
    -format=html \
    -output-dir=coverage-html

# 5. Generate HTML report (Linux)
llvm-cov show ./build/RelWithDebInfo/lib/libslang.so \
    -instr-profile=slang-test.profdata \
    -format=html \
    -output-dir=coverage-html
```

## Output Files

All temporary coverage data is stored in `build/coverage-data/` by default to keep the repository root clean:

- `build/coverage-data/slang-test-*.profraw` - Raw profile data (automatically cleaned up after merging)
- `build/coverage-data/slang-test.profdata` - Merged and indexed profile data (kept for reuse)
- `coverage-html/` - HTML coverage report (if `COVERAGE_HTML=1`, in repo root by default)
- `coverage.lcov` - LCOV format report (if `COVERAGE_LCOV=1`, in repo root by default)

## Analyzing Coverage

### View coverage for specific file:
```bash
# macOS
xcrun llvm-cov show ./build/RelWithDebInfo/lib/libslang.dylib \
    -instr-profile=slang-test.profdata \
    source/slang/slang-parser.cpp

# Linux
llvm-cov show ./build/RelWithDebInfo/lib/libslang.so \
    -instr-profile=slang-test.profdata \
    source/slang/slang-parser.cpp
```

### Get function-level coverage:
```bash
# macOS
xcrun llvm-cov report ./build/RelWithDebInfo/lib/libslang.dylib \
    -instr-profile=slang-test.profdata \
    -show-functions

# Linux
llvm-cov report ./build/RelWithDebInfo/lib/libslang.so \
    -instr-profile=slang-test.profdata \
    -show-functions
```

## Troubleshooting

### No coverage data found
- Ensure binaries were built with `-DSLANG_ENABLE_COVERAGE=ON`
- Check that `*.profraw` files are being generated
- Verify the binary has coverage symbols: `nm build/RelWithDebInfo/lib/libslang.* | grep __llvm_profile`

### Mismatched data warnings
- Rebuild all binaries after enabling coverage
- Delete old `*.profraw` files before running tests

### Coverage tools not found (Linux)
- Install LLVM tools package
- Set `LLVM_PROFDATA` and `LLVM_COV` environment variables
