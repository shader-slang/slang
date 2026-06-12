#!/usr/bin/env bash
# Build + bench the post-v2026.10 daily ToT commits against the GitHub ToT repo,
# writing perf-results/daily/<date>-<sha7>/ in the slang-compile-perf repo format.
# Builds incrementally (configure once, checkout oldest->newest), so only changed
# translation units recompile per commit. Idempotent: skips a day already done.
set -uo pipefail

SUITE=/home/jvepsalainen/gitlab/slang/tools/benchmark/perf-suite
REPO=/home/jvepsalainen/workspace/jvepsalainen-nv-2/slang
OUT=$SUITE/perf-results
SAMPLES=5

# date<TAB>full-sha, oldest first (EOD first-parent tip per active day after v2026.10)
COMMITS=(
  "2026-05-29 8aee8c93225292cef638117d9e4061e4810df0c5"
  "2026-05-30 fb51dcf92"
  "2026-06-01 aaa5f89dd"
  "2026-06-02 adc996670"
  "2026-06-03 eb0dba821"
  "2026-06-04 564ac9f05"
  "2026-06-05 5230a81f2"
  "2026-06-08 6b9f98ff9"
  "2026-06-09 29e69b0bf"
  "2026-06-10 b2ec9a4f7"
  "2026-06-11 45c04170f"
  "2026-06-12 736e3a242"
)

cd "$REPO" || exit 1
for entry in "${COMMITS[@]}"; do
  date=${entry%% *}; sha=${entry##* }
  short=${sha:0:7}
  label="${date}-${short}"
  ddir="$OUT/daily/$label"
  if [[ -f "$ddir/results.json" ]]; then
    echo "=== [$label] already done, skipping ==="
    continue
  fi
  echo "=== [$label] checkout $sha ==="
  git -C "$REPO" checkout -q "$sha" || { echo "checkout failed"; continue; }
  git -C "$REPO" submodule update --init --recursive >/dev/null 2>&1
  printf 'v2026.10\n' > "$REPO/cmake/slang_git_version"
  echo "--- build slangc (incremental) ---"
  if ! cmake --build "$REPO/build" --config Release --target slangc slang-glslang >/tmp/build_$label.log 2>&1; then
    echo "BUILD FAILED for $label (see /tmp/build_$label.log):"; tail -15 /tmp/build_$label.log
    continue
  fi
  slangc=$(ls "$REPO"/build/Release/bin/slangc 2>/dev/null || ls "$REPO"/build/*/bin/slangc 2>/dev/null | head -1)
  echo "--- bench with $slangc ---"
  python3 "$SUITE/bench.py" --slangc "$slangc" --label "$label" \
    --out "$OUT/daily" --samples "$SAMPLES" --warmup 1
  python3 "$SUITE/track.py" register --results "$OUT" \
    --label "$label" --commit "$sha" --date "$date"
done
echo "=== daily sweep complete ==="
