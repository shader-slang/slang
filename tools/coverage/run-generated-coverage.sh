#!/usr/bin/env bash
# Measure compiler coverage of the *generated* test suite ONLY
# (docs/generated/tests/), excluding the hand-written tests/ suite and
# the record-replay pass. Designed for a BEFORE/AFTER snapshot around a
# regeneration pass:
#
#   tools/coverage/run-generated-coverage.sh --label baseline
#   # ... regenerate bundles ...
#   tools/coverage/run-generated-coverage.sh --label after
#   # reports land under docs/generated/tests/_meta/coverage-snapshot/<label>-*
#
# Why per-bundle (not one big slang-test run): under coverage
# instrumentation some tests SIGSEGV slang-test (in-process state
# accumulation). A single in-process run writes its LLVM profile only at
# clean exit, so ONE crash loses ALL coverage. We therefore run each
# bundle in its own slang-test process (crash isolated to that bundle),
# and if a bundle still crashes we fall back to running its tests one
# process per file (a crash then costs only that one file). profraws are
# merged incrementally into the running profdata so peak disk stays
# bounded.
#
# Prereqs: coverage-instrumented build (cmake --preset coverage, clang)
# at build/RelWithDebInfo/, plus llvm-cov/llvm-profdata.

set -uo pipefail # NOTE: not -e; per-bundle crashes are expected and handled

LABEL="baseline"
REPORT_ONLY=false
while [[ $# -gt 0 ]]; do
  case "$1" in
  --label)
    LABEL="$2"
    shift 2
    ;;
  --report-only)
    REPORT_ONLY=true
    shift
    ;;
  -h | --help)
    sed -n '2,33p' "$0"
    exit 0
    ;;
  *)
    echo "Unknown option: $1" >&2
    exit 2
    ;;
  esac
done

SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
REPO_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
cd "$REPO_ROOT"

BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build}"
CONFIG="${CONFIG:-RelWithDebInfo}"
if [[ "$OSTYPE" == "darwin"* ]]; then
  LLVM_PROFDATA="${LLVM_PROFDATA:-xcrun llvm-profdata}"
  LLVM_COV="${LLVM_COV:-xcrun llvm-cov}"
  LIB_EXT="dylib"
else
  LLVM_PROFDATA="${LLVM_PROFDATA:-llvm-profdata}"
  LLVM_COV="${LLVM_COV:-llvm-cov}"
  LIB_EXT="so"
fi

SLANG_TEST="$BUILD_DIR/$CONFIG/bin/slang-test"
BINDIR="$BUILD_DIR/$CONFIG/bin"
LIBSLANG="$BUILD_DIR/$CONFIG/lib/libslang.$LIB_EXT"
TEST_ROOT="docs/generated/tests"

COVERAGE_DIR="${COVERAGE_DIR:-$BUILD_DIR/coverage-generated-$LABEL}"
PROFDATA="$COVERAGE_DIR/slang-test.profdata"
RAW_DIR="$COVERAGE_DIR/raw"

SNAP_DIR="$REPO_ROOT/$TEST_ROOT/_meta/coverage-snapshot"
mkdir -p "$SNAP_DIR" "$COVERAGE_DIR" "$RAW_DIR"
SLANGC_REPORT="$SNAP_DIR/$LABEL-slangc-report.txt"
FULL_REPORT="$SNAP_DIR/$LABEL-full-report.txt"
JSON_EXPORT="$SNAP_DIR/$LABEL-coverage.json"
RUN_LOG="$COVERAGE_DIR/run.log"

[[ -f "$LIBSLANG" ]] || {
  echo "Error: libslang not found at $LIBSLANG" >&2
  exit 1
}

# Common slang-test flags (expected-failures keeps known fails quiet; the
# crash isolation below is what actually guards coverage).
COMMON_ARGS=(
  -bindir "$BINDIR"
  -server-count 1
  -test-dir "$TEST_ROOT"
  -expected-failure-list "$TEST_ROOT/_meta/expected-failures.txt"
)

# merge_raws: fold every *.profraw in $RAW_DIR into the running profdata,
# then clear them (bounded peak disk).
merge_raws() {
  local raws
  raws=("$RAW_DIR"/*.profraw)
  [[ -e "${raws[0]}" ]] || return 0
  if [[ -f "$PROFDATA" ]]; then
    $LLVM_PROFDATA merge -sparse "$PROFDATA" "${raws[@]}" -o "$PROFDATA.tmp" 2>/dev/null &&
      mv "$PROFDATA.tmp" "$PROFDATA"
  else
    $LLVM_PROFDATA merge -sparse "${raws[@]}" -o "$PROFDATA" 2>/dev/null
  fi
  rm -f "$RAW_DIR"/*.profraw
}

# run_scope <profraw-tag> <path-prefix...> : run slang-test scoped to the
# given path prefix(es). Returns slang-test's exit code. Writes a profraw
# only if the process exits cleanly (a SIGSEGV leaves none / an empty one).
run_scope() {
  local tag="$1"
  shift
  LLVM_PROFILE_FILE="$RAW_DIR/$tag-%p.profraw" \
    "$SLANG_TEST" "${COMMON_ARGS[@]}" "$@" >>"$RUN_LOG" 2>&1
  return $?
}

if [[ "$REPORT_ONLY" != "true" ]]; then
  [[ -f "$SLANG_TEST" ]] || {
    echo "Error: slang-test not found at $SLANG_TEST" >&2
    exit 1
  }
  echo "Cleaning old coverage data in $COVERAGE_DIR ..."
  rm -f "$PROFDATA" "$RAW_DIR"/*.profraw
  : >"$RUN_LOG"

  mapfile -t BUNDLES < <(python3 "$TEST_ROOT/_meta/regenerate.py" list 2>/dev/null)
  echo "Running ${#BUNDLES[@]} bundles under coverage (label=$LABEL), per-bundle isolated..."

  n=0
  crashed_bundles=()
  for bundle in "${BUNDLES[@]}"; do
    [[ -n "$bundle" ]] || continue
    n=$((n + 1))
    dir="$TEST_ROOT/$bundle"
    [[ -d "$dir" ]] || {
      echo "  [$n/${#BUNDLES[@]}] skip (no dir): $bundle"
      continue
    }
    tag="$(echo "$bundle" | tr '/' '_')"
    run_scope "$tag" "$dir" || true
    # Detect a lost bundle: slang-test crashed => no non-empty profraw.
    if ! find "$RAW_DIR" -name "$tag-*.profraw" -size +0c | grep -q .; then
      echo "  [$n/${#BUNDLES[@]}] CRASH in $bundle -> per-file fallback"
      crashed_bundles+=("$bundle")
      rm -f "$RAW_DIR/$tag-"*.profraw # drop the empty/partial one
      # Per-file: a crash now costs only the single offending file.
      while IFS= read -r f; do
        ftag="$tag-$(basename "$f" .slang)"
        run_scope "$ftag" "$f" || true
      done < <(find "$dir" -name '*.slang' | sort)
    else
      echo "  [$n/${#BUNDLES[@]}] ok: $bundle"
    fi
    merge_raws # incremental fold -> bounded disk
  done

  merge_raws
  if [[ ! -f "$PROFDATA" ]]; then
    echo "Error: no coverage data produced (every bundle crashed?). See $RUN_LOG" >&2
    exit 1
  fi
  if [[ ${#crashed_bundles[@]} -gt 0 ]]; then
    echo "Note: ${#crashed_bundles[@]} bundle(s) needed per-file fallback: ${crashed_bundles[*]}"
  fi
else
  [[ -f "$PROFDATA" ]] || {
    echo "Error: --report-only but no profdata at $PROFDATA" >&2
    exit 1
  }
fi

# shellcheck source=tools/coverage/slangc-ignore-patterns.sh
source "$SCRIPT_DIR/slangc-ignore-patterns.sh"

echo
echo "Full library coverage:"
$LLVM_COV report "$LIBSLANG" -instr-profile="$PROFDATA" | tee "$FULL_REPORT" | grep -E "^TOTAL" || true
echo
echo "slangc compiler-only coverage (label=$LABEL):"
$LLVM_COV report "$LIBSLANG" -instr-profile="$PROFDATA" "${SLANGC_IGNORE_ARGS[@]}" | tee "$SLANGC_REPORT" | grep -E "^TOTAL" || true
$LLVM_COV export "$LIBSLANG" -instr-profile="$PROFDATA" "${SLANGC_IGNORE_ARGS[@]}" >"$JSON_EXPORT"

echo
echo "Wrote:"
echo "  $SLANGC_REPORT"
echo "  $FULL_REPORT"
echo "  $JSON_EXPORT"
echo "  $PROFDATA"
