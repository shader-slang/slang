#!/usr/bin/env bash
# Script to run tests with coverage and generate reports

# Check Bash version (requires 4.0+ for modern syntax like &>)
if [[ -z "${BASH_VERSION}" ]] || [[ "${BASH_VERSION%%.*}" -lt 4 ]]; then
  echo "Error: This script requires Bash 4.0 or later (current: ${BASH_VERSION:-unknown})" >&2
  if [[ "$OSTYPE" == "darwin" ]]; then
    echo "On macOS, you can install a newer Bash via Homebrew: brew install bash" >&2
  fi
  exit 1
fi

set -e

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
CONFIG="${CONFIG:-Debug}"

# Coverage binary and library paths
SLANG_TEST="$BUILD_DIR/$CONFIG/bin/slang-test"
LIBSLANG="$BUILD_DIR/$CONFIG/lib/libslang.$LIB_EXT"

# Coverage output directory (use build dir to keep repo clean)
COVERAGE_DIR="${COVERAGE_DIR:-$BUILD_DIR/coverage-data}"
mkdir -p "$COVERAGE_DIR"

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

# Run tests
echo
echo "Running tests with coverage instrumentation..."
echo "Coverage data directory: $COVERAGE_DIR"
cd "$REPO_ROOT"
"$SLANG_TEST" "$@"

# Check if any profraw files were generated
if ! ls "$COVERAGE_DIR"/slang-test-*.profraw 1>/dev/null 2>&1; then
  echo
  echo "Warning: No coverage data was generated."
  echo "Make sure the binaries were built with SLANG_ENABLE_COVERAGE=ON"
  exit 1
fi

# Merge coverage data
echo
echo "Merging coverage data..."
$LLVM_PROFDATA merge -sparse "$COVERAGE_DIR"/slang-test-*.profraw -o "$COVERAGE_DIR"/slang-test.profdata

# Generate summary report
echo
echo "Coverage Summary:"
echo "================"
$LLVM_COV report "$LIBSLANG" -instr-profile="$COVERAGE_DIR"/slang-test.profdata

# Generate HTML report (optional)
if [[ "$COVERAGE_HTML" = "1" ]]; then
  HTML_DIR="${COVERAGE_HTML_DIR:-$REPO_ROOT/coverage-html}"
  echo
  echo "Generating HTML coverage report..."
  $LLVM_COV show "$LIBSLANG" \
    -instr-profile="$COVERAGE_DIR"/slang-test.profdata \
    -format=html \
    -output-dir="$HTML_DIR"
  echo
  echo "HTML report generated in $HTML_DIR/index.html"

  # Try to open the report on macOS
  if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Opening report in browser..."
    open "$HTML_DIR/index.html"
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
  echo "  - ${COVERAGE_HTML_DIR:-$REPO_ROOT/coverage-html}/ (HTML report)"
fi
if [[ "$COVERAGE_LCOV" = "1" ]]; then
  echo "  - ${COVERAGE_LCOV_FILE:-$REPO_ROOT/coverage.lcov} (LCOV format for CI tools)"
fi

# Clean up raw profraw files to save space
echo
echo "Cleaning up raw profile data..."
rm -f "$COVERAGE_DIR"/*.profraw
echo "Kept merged profile data at: $COVERAGE_DIR/slang-test.profdata"
