# Code Coverage Tools

This directory contains tools for generating and analyzing code coverage reports for the Slang compiler.

## Prerequisites

- **Linux / macOS**: LLVM coverage tools (`llvm-profdata`, `llvm-cov`) installed
- **Windows**: [OpenCppCoverage](https://github.com/OpenCppCoverage/OpenCppCoverage/releases) installed (the `ci-slang-coverage.yml` workflow installs it via `choco install opencppcoverage --version 0.9.9.0`; for local dev either install the `.exe` from GitHub releases or `winget install --id OpenCppCoverage.OpenCppCoverage`)

## Quick Start (HTML report)

### Linux / macOS

```bash
cmake --preset coverage
cmake --build --preset coverage
COVERAGE_HTML=1 ./tools/coverage/run-coverage.sh
# Report is at ./coverage-html/index.html
open ./coverage-html/index.html
```

Wrapper script handles the full workflow on local workspace:

```bash
./tools/coverage/run-coverage-local.sh
```

Run `./tools/coverage/run-coverage-local.sh --help` for options.

### Windows

```powershell
# Configure + build slang-test in RelWithDebInfo with SLANG_ENABLE_COVERAGE=ON
# (enables /Ob0 for truthful PDB line mappings), run the test suite wrapped in
# OpenCppCoverage, and generate HTML + LCOV reports.
powershell -File tools\coverage\run-coverage-local.ps1

# Re-export reports from an existing run without re-testing:
powershell -File tools\coverage\run-coverage-local.ps1 -SkipBuild -SkipTest

# Outputs:
#   coverage-html\index.html              -- full library HTML report
#   coverage.lcov                         -- full library LCOV
#   coverage-slangc.lcov                  -- slangc compiler-only LCOV
#   build\coverage-data\full.cobertura.xml
```

Notes for Windows:
- Coverage targets `slang-compiler.dll` (the real compiler lib; `slang.dll` is a thin proxy on Windows).
- OpenCppCoverage emits Cobertura XML, which the script converts to LCOV internally for format parity with Linux/macOS.
- No `coverage-html-slangc/` directory on Windows — the slangc-only filter runs post-hoc on the LCOV; generating an HTML view would require a second test pass (deferred).
- Add `-WithSynthesis` to run the `-only-synthesized` pass. On runners without GPU drivers this provides a large backend-emit coverage boost; on machines with drivers it's a near no-op because Pass 1 already exercises the same paths.

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

## Record-Replay Coverage

The coverage script automatically includes record-replay subsystem coverage by running focused smoke tests with `SLANG_RECORD_LAYER=1` enabled after the main test suite. This captures coverage for:

- `source/slang-record-replay/record/` - API recording wrappers
- `source/slang-record-replay/util/` - Recording utilities
- Record manager and output stream infrastructure

The record-replay smoke tests (`RecordReplaySmokeCreateSession`, `RecordReplaySmokeCompileModule`, `RecordReplaySmokeEntryPoint`) exercise basic Slang API calls with recording enabled, providing ~8-10% coverage of the record-replay subsystem. All coverage data is merged into a single unified report.

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
