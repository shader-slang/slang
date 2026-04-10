#!/usr/bin/env bash
# Script to run tests with coverage and generate reports

set -e

# Parse arguments
REPORT_ONLY=false
WITH_SYNTHESIS=false
TEST_ARGS=()
for arg in "$@"; do
  if [[ "$arg" == "--report-only" ]]; then
    REPORT_ONLY=true
  elif [[ "$arg" == "--with-synthesis" ]]; then
    WITH_SYNTHESIS=true
  else
    TEST_ARGS+=("$arg")
  fi
done

# Detect platform and set appropriate tools
if [[ "$OSTYPE" == "darwin"* ]]; then
  # macOS - use xcrun to find the right tools
  LLVM_PROFDATA="${LLVM_PROFDATA:-xcrun llvm-profdata}"
  LLVM_COV="${LLVM_COV:-xcrun llvm-cov}"
  LIB_EXT="dylib"
else
  # Linux/Unix - use system tools
  LLVM_PROFDATA="${LLVM_PROFDATA:-llvm-profdata}"
  LLVM_COV="${LLVM_COV:-llvm-cov}"
  LIB_EXT="so"
fi

# Determine paths
SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}" 2>/dev/null || realpath "${BASH_SOURCE[0]}" 2>/dev/null || echo "${BASH_SOURCE[0]}")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
REPO_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build}"
CONFIG="${CONFIG:-RelWithDebInfo}"

# Coverage binary and library paths
SLANG_TEST="$BUILD_DIR/$CONFIG/bin/slang-test"
LIBSLANG="$BUILD_DIR/$CONFIG/lib/libslang.$LIB_EXT"

# Coverage output directory (use build dir to keep repo clean)
COVERAGE_DIR="${COVERAGE_DIR:-$BUILD_DIR/coverage-data}"
mkdir -p "$COVERAGE_DIR"

if [[ "$REPORT_ONLY" == "true" ]]; then
  # Report-only mode: check that profdata exists
  echo "Report-only mode: using existing coverage data"

  if [[ ! -f "$COVERAGE_DIR/slang-test.profdata" ]]; then
    echo "Error: No existing coverage data found at $COVERAGE_DIR/slang-test.profdata"
    echo "Run without --report-only first to collect coverage data"
    exit 1
  fi

  if [[ ! -f "$LIBSLANG" ]]; then
    echo "Error: libslang not found at $LIBSLANG"
    exit 1
  fi
else
  # Normal mode: run tests and collect coverage

  # Check if binaries exist
  if [[ ! -f "$SLANG_TEST" ]]; then
    echo "Error: slang-test not found at $SLANG_TEST"
    echo "Please build with coverage enabled first:"
    echo "  cmake --preset coverage"
    echo "  cmake --build --preset coverage"
    exit 1
  fi

  if [[ ! -f "$LIBSLANG" ]]; then
    echo "Error: libslang not found at $LIBSLANG"
    exit 1
  fi

  # Clean up old coverage data
  echo "Cleaning up old coverage data..."
  rm -rf "$COVERAGE_DIR"/*.profraw "$COVERAGE_DIR"/*.profdata

  # Set up coverage output in temp directory
  export LLVM_PROFILE_FILE="$COVERAGE_DIR/slang-test-%p.profraw"

  # Run tests (capture exit code but continue to generate reports even if tests fail)
  echo
  echo "Running tests with coverage instrumentation..."
  echo "Coverage data directory: $COVERAGE_DIR"
  cd "$REPO_ROOT"
  TEST_EXIT=0
  "$SLANG_TEST" "${TEST_ARGS[@]}" || TEST_EXIT=$?
  if [ "$TEST_EXIT" -ne 0 ]; then
    echo "Warning: slang-test exited with code $TEST_EXIT. Coverage data still collected."
  fi

  # Run record-replay API tests with recording enabled to capture record-replay coverage
  # This runs only the focused RecordReplayApi* tests with SLANG_RECORD_LAYER=1 to
  # exercise the record-replay code paths. The profraw files accumulate with the main run.
  echo
  echo "Running record-replay API tests with recording enabled..."
  RECORD_DIR="$COVERAGE_DIR/slang-record"
  mkdir -p "$RECORD_DIR"

  export SLANG_RECORD_LAYER=1
  export SLANG_RECORD_DIRECTORY="$RECORD_DIR"

  # Run only the RecordReplayApi tests (fast, focused coverage)
  "$SLANG_TEST" slang-unit-test-tool/RecordReplayApi || true

  unset SLANG_RECORD_LAYER
  unset SLANG_RECORD_DIRECTORY

  # Clean up recording files (only need coverage data)
  rm -rf "$RECORD_DIR"

  # Optional: run synthesized compile-target tests for backend emit coverage.
  # These exercise code generation paths (HLSL, SPIRV, Metal, etc.) without a GPU.
  # Failures are tolerated since some targets have known gaps.
  if [[ "$WITH_SYNTHESIS" == "true" ]]; then
    echo
    echo "Running synthesized compile-target tests..."
    # Use a separate profraw prefix so synthesis data doesn't interfere
    # with main pass data during merge
    export LLVM_PROFILE_FILE="$COVERAGE_DIR/synth-%p.profraw"
    SYNTH_EXIT=0
    "$SLANG_TEST" "${TEST_ARGS[@]}" -only-synthesized || SYNTH_EXIT=$?
    if [ "$SYNTH_EXIT" -gt 128 ]; then
      echo "Warning: synthesis pass crashed (signal $((SYNTH_EXIT - 128)))"
    elif [ "$SYNTH_EXIT" -ne 0 ]; then
      echo "Note: synthesis pass had test failures (exit code $SYNTH_EXIT). Coverage data still collected."
    fi
  fi

  # Check if any profraw files were generated
  if ! ls "$COVERAGE_DIR"/slang-test-*.profraw >/dev/null 2>&1; then
    echo
    echo "Warning: No coverage data was generated."
    echo "Make sure the binaries were built with SLANG_ENABLE_COVERAGE=ON"
    exit 1
  fi

  # Merge coverage data
  echo
  echo "Main pass profraw files:"
  ls -lh "$COVERAGE_DIR"/slang-test-*.profraw 2>/dev/null | head -20
  echo "Main pass profraw count: $(ls "$COVERAGE_DIR"/slang-test-*.profraw 2>/dev/null | wc -l)"

  if ls "$COVERAGE_DIR"/synth-*.profraw >/dev/null 2>&1; then
    echo
    echo "Synthesis pass profraw files:"
    ls -lh "$COVERAGE_DIR"/synth-*.profraw 2>/dev/null | head -20
    echo "Synthesis profraw count: $(ls "$COVERAGE_DIR"/synth-*.profraw 2>/dev/null | wc -l)"
  fi

  echo
  echo "Merging coverage data..."
  $LLVM_PROFDATA merge -sparse "$COVERAGE_DIR"/slang-test-*.profraw "$COVERAGE_DIR"/synth-*.profraw -o "$COVERAGE_DIR"/slang-test.profdata 2>/dev/null \
    || $LLVM_PROFDATA merge -sparse "$COVERAGE_DIR"/slang-test-*.profraw -o "$COVERAGE_DIR"/slang-test.profdata
  echo "Merged profdata size: $(ls -lh "$COVERAGE_DIR/slang-test.profdata" | awk '{print $5}')"
fi

# Load slangc compiler-only ignore patterns (shared with CI workflow)
source "$(dirname "${BASH_SOURCE[0]}")/slangc-ignore-patterns.sh"

# Generate summary report (full library)
echo
echo "Full Library Coverage Summary:"
echo "=============================="
$LLVM_COV report "$LIBSLANG" -instr-profile="$COVERAGE_DIR"/slang-test.profdata

# Generate slangc compiler-only summary report
echo
echo "slangc Compiler Coverage (excludes generated/non-compiler code):"
echo "================================================================"
$LLVM_COV report "$LIBSLANG" -instr-profile="$COVERAGE_DIR"/slang-test.profdata "${SLANGC_IGNORE_ARGS[@]}"

# Generate HTML reports (optional)
if [[ "$COVERAGE_HTML" = "1" ]]; then
  HTML_DIR="${COVERAGE_HTML_DIR:-$REPO_ROOT/coverage-html}"

  echo
  echo "Generating HTML coverage report (full library)..."
  $LLVM_COV show "$LIBSLANG" \
    -instr-profile="$COVERAGE_DIR"/slang-test.profdata \
    -format=html \
    -output-dir="$HTML_DIR"
  echo "HTML report generated in $HTML_DIR/index.html"

  SLANGC_HTML_DIR="${HTML_DIR}-slangc"
  echo
  echo "Generating HTML coverage report (slangc compiler only)..."
  $LLVM_COV show "$LIBSLANG" \
    -instr-profile="$COVERAGE_DIR"/slang-test.profdata \
    -format=html \
    -output-dir="$SLANGC_HTML_DIR" \
    "${SLANGC_IGNORE_ARGS[@]}"
  echo "HTML report generated in $SLANGC_HTML_DIR/index.html"

  if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Opening reports in browser..."
    open "$HTML_DIR/index.html"
    open "$SLANGC_HTML_DIR/index.html"
  fi
fi

# Generate lcov format (optional, useful for CI integration)
if [[ "$COVERAGE_LCOV" = "1" ]]; then
  LCOV_FILE="${COVERAGE_LCOV_FILE:-$REPO_ROOT/coverage.lcov}"
  echo
  echo "Generating LCOV format report..."
  $LLVM_COV export "$LIBSLANG" \
    -instr-profile="$COVERAGE_DIR"/slang-test.profdata \
    -format=lcov >"$LCOV_FILE"
  echo "LCOV report generated: $LCOV_FILE"
fi

echo
echo "Coverage data files:"
echo "  - $COVERAGE_DIR/slang-test.profdata (merged profile data)"
echo "  - $COVERAGE_DIR/*.profraw (raw profile data - can be deleted)"
if [[ "$COVERAGE_HTML" = "1" ]]; then
  echo "  - ${COVERAGE_HTML_DIR:-$REPO_ROOT/coverage-html}/ (HTML report - full library)"
  echo "  - ${COVERAGE_HTML_DIR:-$REPO_ROOT/coverage-html}-slangc/ (HTML report - slangc compiler only)"
fi
if [[ "$COVERAGE_LCOV" = "1" ]]; then
  echo "  - ${COVERAGE_LCOV_FILE:-$REPO_ROOT/coverage.lcov} (LCOV format for CI tools)"
fi

# Clean up raw profraw files to save space (only in normal mode)
if [[ "$REPORT_ONLY" != "true" ]]; then
  echo
  echo "Cleaning up raw profile data..."
  rm -f "$COVERAGE_DIR"/*.profraw
  echo "Kept merged profile data at: $COVERAGE_DIR/slang-test.profdata"
fi

# Propagate test failure after reports are generated
if [[ "${TEST_EXIT:-0}" -ne 0 ]]; then
  echo
  echo "Exiting with test failure code $TEST_EXIT (reports were still generated)"
  exit "$TEST_EXIT"
fi
