#!/usr/bin/env bash
# Wrapper script for running coverage locally on macOS and Linux
# This script configures, builds, and runs tests with coverage reporting

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Parse command line options
SKIP_BUILD=false
SKIP_TEST=false
SKIP_REPORT=false
SERVER_COUNT=8
SHOW_HELP=false

while [[ $# -gt 0 ]]; do
  case $1 in
    --skip-build)
      SKIP_BUILD=true
      shift
      ;;
    --skip-test)
      SKIP_TEST=true
      shift
      ;;
    --skip-report)
      SKIP_REPORT=true
      shift
      ;;
    --server-count)
      SERVER_COUNT="$2"
      shift 2
      ;;
    -h|--help)
      SHOW_HELP=true
      shift
      ;;
    *)
      echo -e "${RED}Unknown option: $1${NC}"
      SHOW_HELP=true
      shift
      ;;
  esac
done

if [[ "$SHOW_HELP" == "true" ]]; then
  cat <<EOF
Usage: $0 [OPTIONS]

Options:
  --skip-build          Skip the CMake configuration and build steps
  --skip-test           Skip running tests (use existing coverage data)
  --skip-report         Skip generating coverage report
  --server-count N      Number of test servers to use (default: 8)
  -h, --help            Show this help message

Examples:
  # Full run: configure, build, test, and generate report
  $0

  # Quick test run without rebuilding
  $0 --skip-build

  # Only regenerate report from existing coverage data
  $0 --skip-build --skip-test

  # Run tests without generating report (for incremental coverage data)
  $0 --skip-report

  # Run with single test server (slower but uses less memory)
  $0 --server-count 1

Environment Variables:
  CONFIG             Build configuration (default: RelWithDebInfo)
  BUILD_DIR          Build directory (default: ./build)

Note: The coverage preset uses default LLVM configuration from CMakePresets.json.
      For custom LLVM setup, configure manually with cmake --preset coverage.

EOF
  exit 0
fi

# Get repository root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$REPO_ROOT"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Slang Coverage Report - Local Run${NC}"
echo -e "${BLUE}========================================${NC}"
echo

# Step 1: Configure with coverage preset
if [[ "$SKIP_BUILD" == "false" ]]; then
  echo -e "${GREEN}Step 1: Configuring with coverage preset...${NC}"
  cmake --preset coverage --fresh

  # Step 2: Build
  echo
  echo -e "${GREEN}Step 2: Building with coverage instrumentation...${NC}"
  echo -e "${YELLOW}Note: This may take 5-20 minutes depending on your machine${NC}"
  cmake --build --preset coverage

  echo
  echo -e "${GREEN}Build completed successfully!${NC}"
fi

# Step 3: Run tests with coverage
if [[ "$SKIP_TEST" == "false" ]]; then
  echo
  echo -e "${GREEN}Step 3: Running tests with coverage...${NC}"
  echo -e "${YELLOW}Note: This may take 10-30 minutes depending on server count${NC}"

  # Build test arguments
  test_args=(
    "-expected-failure-list" "tests/expected-failure-github.txt"
    "-expected-failure-list" "tests/expected-failure-no-gpu.txt"
    "-skip-reference-image-generation"
    "-show-adapter-info"
    "-ignore-abort-msg"
    "-enable-debug-layers" "true"
  )

  # Add test server arguments if server count > 1
  if [ "$SERVER_COUNT" -gt 1 ]; then
    test_args+=("-use-test-server")
    test_args+=("-server-count" "$SERVER_COUNT")
  fi

  # Run coverage (tests only, skip report generation)
  ./tools/coverage/run-coverage.sh "${test_args[@]}"

  echo
  echo -e "${GREEN}Tests completed successfully!${NC}"
fi

# Step 4: Generate report (unless skipped)
if [[ "$SKIP_REPORT" == "false" ]]; then
  echo
  echo -e "${GREEN}Step 4: Generating coverage report...${NC}"

  export COVERAGE_HTML=1
  export COVERAGE_HTML_DIR="$REPO_ROOT/coverage-html"

  # Run coverage script in report-only mode
  ./tools/coverage/run-coverage.sh --report-only
fi

# Summary
echo
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}Coverage run completed!${NC}"
echo -e "${BLUE}========================================${NC}"
echo
echo "Coverage reports generated:"
echo "  📊 HTML Report: $REPO_ROOT/coverage-html/index.html"
echo "  📁 Coverage Data: $REPO_ROOT/build/coverage-data/"
echo

# Try to open HTML report (skip if on remote/SSH session)
if [[ -f "$REPO_ROOT/coverage-html/index.html" ]]; then
  # Check if we're in an SSH session
  if [[ -n "${SSH_CONNECTION:-}" || -n "${SSH_CLIENT:-}" || -n "${SSH_TTY:-}" ]]; then
    echo "Note: Detected SSH session - skipping automatic browser open"
    echo "      You can view the report by copying coverage-html/ to your local machine"
  else
    # Local session - try to open in browser
    if [[ "$OSTYPE" == "darwin"* ]]; then
      echo "Opening coverage report in your browser..."
      open "$REPO_ROOT/coverage-html/index.html"
    elif command -v xdg-open &> /dev/null; then
      echo "Opening coverage report in your browser..."
      xdg-open "$REPO_ROOT/coverage-html/index.html" 2>/dev/null || true
    fi
  fi
fi

echo
echo "Useful commands:"
echo "  Regenerate report only:  $0 --skip-build --skip-test"
echo "  Run tests without report: $0 --skip-report"
echo "  Quick rerun after changes: $0 --skip-build"
echo
