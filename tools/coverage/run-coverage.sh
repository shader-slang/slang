#!/usr/bin/env bash
# Script to run tests with coverage and generate reports

set -e

# Parse arguments
REPORT_ONLY=false
WITH_SYNTHESIS=false
WITH_AGENTIC_TESTS=false
TEST_ARGS=()
for arg in "$@"; do
  if [[ "$arg" == "--report-only" ]]; then
    REPORT_ONLY=true
  elif [[ "$arg" == "--with-synthesis" ]]; then
    WITH_SYNTHESIS=true
  elif [[ "$arg" == "--with-agentic-tests" ]]; then
    WITH_AGENTIC_TESTS=true
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

  # Run tests
  echo
  echo "Running tests with coverage instrumentation..."
  echo "Coverage data directory: $COVERAGE_DIR"
  cd "$REPO_ROOT"
  "$SLANG_TEST" "${TEST_ARGS[@]}"

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
    SYNTH_EXIT=0
    "$SLANG_TEST" "${TEST_ARGS[@]}" -only-synthesized || SYNTH_EXIT=$?
    if [ "$SYNTH_EXIT" -gt 128 ]; then
      echo "Warning: synthesis pass crashed (signal $((SYNTH_EXIT - 128)))"
    elif [ "$SYNTH_EXIT" -ne 0 ]; then
      echo "Note: synthesis pass had test failures (exit code $SYNTH_EXIT). Coverage data still collected."
    fi
  fi

  # Optional: run the agentic test suite under docs/generated/tests/ for
  # additional coverage. This is the LLM-generated, doc-anchored bundle
  # set (see docs/generated/tests/README.md). It exercises text-emit
  # paths and diagnostics that the hand-written tests/ suite doesn't
  # always reach. profraw files from these runs accumulate into the
  # same merged profdata as the main pass.
  #
  # Failures are tolerated — the suite is still being de-bugged (see
  # docs/generated/tests/_meta/findings/) and ci-agentic-tests-nightly.yml
  # is the authoritative pass/fail gate. We're here for the coverage
  # numbers, not the verdict.
  if [[ "$WITH_AGENTIC_TESTS" == "true" ]]; then
    echo
    echo "Running agentic test suite (docs/generated/tests/) for coverage..."
    # Use the agentic suite's own expected-failure list.
    # `slang-test -expected-failure-list` does EXACT-STRING matching:
    # listed paths get reclassified to `failed(expected)` (slang-test's
    # TestResult::ExpectedFail), the run exits 0 when only those
    # occur, and any entry that has started passing is reported under
    # "passing tests that are expected to fail" so it can be removed.
    # Caveat: multi-//TEST tests are named per sub-test
    # (`foo.slang`, `foo.slang.1`, …), so if a specific sub-test
    # needs suppression, the entry must include the `.N` suffix —
    # there is no canonicalization on this path.
    # Build the agentic invocation: start with the agentic-specific
    # overrides (test-dir and expected-failure-list — those are
    # what differ from the main pass), then inherit every other
    # flag from the main pass's TEST_ARGS so settings like
    # -use-test-server, -synthesizedTestApi,
    # -skip-reference-image-generation, -enable-debug-layers, etc.
    # apply uniformly. -server-count is also overridden, but in a
    # dedicated block further down (the value gets forced to 1 for
    # sequential execution under coverage instrumentation; see the
    # comment at the `AGENTIC_TEST_ARGS+=("-server-count" "1")`
    # line for the rationale). This keeps the agentic pass running
    # in the same slang-test mode as the main pass; the only
    # differences are which tests run, which list of failures to
    # expect, and forced sequential execution.
    AGENTIC_TEST_ARGS=(
      "-test-dir" "docs/generated/tests"
      "-expected-failure-list" "docs/generated/tests/_meta/expected-failures.txt"
    )
    # Walk TEST_ARGS one token at a time and classify each:
    #   - Agentic-overridden value-taking flags (-test-dir,
    #     -expected-failure-list, -server-count): skip flag + value.
    #   - Known no-value option flags (used in the coverage workflow's
    #     test_args): inherit as a lone token.
    #   - Any other "-*" flag: assume it takes a value and inherit
    #     flag + value. Default-to-two-arg is the safer policy: if a
    #     future maintainer adds e.g. `-capability spirv_1_5` to the
    #     workflow's test_args, the value is preserved instead of
    #     being silently dropped as a "bare positional." If a new
    #     no-value flag is ever added to test_args, add it to the
    #     explicit no-value case above so the next token isn't
    #     swallowed as a phantom value.
    #   - Bare positional tokens (test selectors): drop. The agentic
    #     pass runs over its own -test-dir; inheriting positionals
    #     from the main pass would silently narrow the agentic run.
    #     CI doesn't pass bare positionals here today; this only
    #     guards local-dev `run-coverage.sh --with-agentic-tests foo`.
    i=0
    while [ "$i" -lt "${#TEST_ARGS[@]}" ]; do
      arg="${TEST_ARGS[i]}"
      case "$arg" in
      -test-dir | -expected-failure-list | -server-count)
        i=$((i + 2))
        ;;
      -use-test-server | -skip-reference-image-generation | -show-adapter-info | -only-synthesized)
        AGENTIC_TEST_ARGS+=("$arg")
        i=$((i + 1))
        ;;
      -*)
        if [ "$((i + 1))" -lt "${#TEST_ARGS[@]}" ]; then
          AGENTIC_TEST_ARGS+=("$arg" "${TEST_ARGS[i + 1]}")
          i=$((i + 2))
        else
          AGENTIC_TEST_ARGS+=("$arg")
          i=$((i + 1))
        fi
        ;;
      *)
        i=$((i + 1))
        ;;
      esac
    done
    # Force sequential execution for the agentic pass under coverage.
    # On Linux coverage we hit a deterministic SIGSEGV in slang-test
    # (runs 26810699621, 26813684019) that kills the orchestrator and
    # loses the tail of the suite. With -server-count 1 the crashing
    # test is unambiguously the last one printed before the segfault,
    # so we can identify it and add it to AGENTIC_COVERAGE_EXCLUDES
    # (below). Once the list is stable we may flip back to parallel.
    # Sequential adds wall time on Linux coverage but the workflow
    # runs once a day; data quality > runner-minute savings.
    AGENTIC_TEST_ARGS+=("-server-count" "1")

    # Read coverage-only exclude list from
    # docs/generated/tests/_meta/agentic-coverage-excludes.txt and add
    # one -exclude-prefix flag per entry. These are tests that crash
    # slang-test under coverage instrumentation (the orchestrator dies
    # SIGSEGV and loses the tail of the suite); they still run in
    # ci-agentic-tests-nightly against the uninstrumented binary.
    AGENTIC_COVERAGE_EXCLUDES_FILE="$REPO_ROOT/docs/generated/tests/_meta/agentic-coverage-excludes.txt"
    if [ -f "$AGENTIC_COVERAGE_EXCLUDES_FILE" ]; then
      while IFS= read -r line || [ -n "$line" ]; do
        # Strip trailing comment, then trim whitespace.
        line="${line%%#*}"
        line="${line#"${line%%[![:space:]]*}"}"
        line="${line%"${line##*[![:space:]]}"}"
        if [ -n "$line" ]; then
          AGENTIC_TEST_ARGS+=("-exclude-prefix" "$line")
        fi
      done <"$AGENTIC_COVERAGE_EXCLUDES_FILE"
    fi
    # Retry once on crash (exit > 128 = killed by signal). The suite
    # is long (~2680 bundles); a single SIGSEGV in slang-test mid-run
    # loses the rest of the pass and produces minimal coverage uplift.
    # Profraws from the first attempt are kept on disk and accumulate
    # with the retry's so coverage from both runs merges into the
    # report.
    #
    # Retries only fire on crashes, not on unexpected test failures
    # (those repeat deterministically and would just waste runner
    # time). The retry helps *flaky* crashes — for a reproducible
    # SIGSEGV the second attempt is pure overhead and will crash at
    # the same point. The 2-attempt cap bounds that cost.
    AGENTIC_MAX_ATTEMPTS=2
    AGENTIC_EXIT=0
    for attempt in $(seq 1 $AGENTIC_MAX_ATTEMPTS); do
      if [ "$attempt" -gt 1 ]; then
        echo "Retrying agentic-test pass (attempt $attempt of $AGENTIC_MAX_ATTEMPTS)..."
      fi
      AGENTIC_EXIT=0
      "$SLANG_TEST" "${AGENTIC_TEST_ARGS[@]}" || AGENTIC_EXIT=$?
      if [ "$AGENTIC_EXIT" -le 128 ]; then
        # Not a crash — passed, or had expected/unexpected failures.
        # Don't retry; those don't change on a re-run.
        break
      fi
      echo "Warning: agentic-test pass crashed (signal $((AGENTIC_EXIT - 128))) on attempt $attempt of $AGENTIC_MAX_ATTEMPTS"
    done

    if [ "$AGENTIC_EXIT" -gt 128 ]; then
      echo "Warning: agentic-test pass crashed on all $AGENTIC_MAX_ATTEMPTS attempts (signal $((AGENTIC_EXIT - 128))). Partial coverage data still collected."
    elif [ "$AGENTIC_EXIT" -ne 0 ]; then
      echo "Note: agentic-test pass had unexpected test failures (exit code $AGENTIC_EXIT). Coverage data still collected; see ci-agentic-tests-nightly.yml for the authoritative pass/fail gate."
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
  echo "Merging coverage data..."
  $LLVM_PROFDATA merge -sparse "$COVERAGE_DIR"/slang-test-*.profraw -o "$COVERAGE_DIR"/slang-test.profdata
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

# Generate llvm-cov JSON export (optional). The JSON form is the
# preferred input to slang-coverage-html for the LLVM-coverage path
# because it carries regions natively and matches `llvm-cov report`
# without the LCOV+--auth-summary patching dance.
if [[ "$COVERAGE_JSON" = "1" ]]; then
  JSON_FILE="${COVERAGE_JSON_FILE:-$REPO_ROOT/coverage.json}"
  echo
  echo "Generating llvm-cov JSON export..."
  $LLVM_COV export "$LIBSLANG" \
    -instr-profile="$COVERAGE_DIR"/slang-test.profdata >"$JSON_FILE"
  echo "JSON report generated: $JSON_FILE"
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
if [[ "$COVERAGE_JSON" = "1" ]]; then
  echo "  - ${COVERAGE_JSON_FILE:-$REPO_ROOT/coverage.json} (llvm-cov JSON export)"
fi

# Clean up raw profraw files to save space (only in normal mode)
if [[ "$REPORT_ONLY" != "true" ]]; then
  echo
  echo "Cleaning up raw profile data..."
  rm -f "$COVERAGE_DIR"/*.profraw
  echo "Kept merged profile data at: $COVERAGE_DIR/slang-test.profdata"
fi
