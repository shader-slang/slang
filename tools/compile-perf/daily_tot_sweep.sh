#!/usr/bin/env bash
# Build + bench the post-v2026.10 daily ToT commits against the GitHub ToT repo,
# writing perf-results/daily/<date>-<sha7>/ in the slang-compile-perf repo format.
# Configures once with release-matching flags (LTO on — see CMAKE_FLAGS), then checks
# out oldest->newest. Idempotent: skips a day that already has results.json.
set -euo pipefail

SUITE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO=$(cd "$SUITE/../.." && pwd)
OUT=$SUITE/perf-results
SAMPLES=5

# Release-matching configure flags (mirrors .github/workflows/release.yml).
# LTO is the critical one: a plain Release build runs sema ~1.5x slower than the
# official LTO binaries, producing a build-method step at the release->daily boundary
# rather than a real regression. SLANG_STANDARD_MODULE_DEVELOP_BUILD=OFF matches
# release.yml. Unneeded subsystems (DXIL, gfx, tests, …) are disabled to keep
# configure and build times reasonable; they don't affect the measured timers.
# NOTE: LTO means every build is a full whole-program link — builds are NOT cheaply
# incremental across commits.
CMAKE_FLAGS=(
  -DSLANG_ENABLE_RELEASE_LTO=ON
  -DSLANG_STANDARD_MODULE_DEVELOP_BUILD=OFF
  -DSLANG_ENABLE_DXIL=OFF
  -DSLANG_SLANG_LLVM_FLAVOR=DISABLE
  -DSLANG_ENABLE_GFX=OFF
  -DSLANG_ENABLE_SLANG_RHI=OFF
  -DSLANG_ENABLE_SLANGD=OFF
  -DSLANG_ENABLE_REPLAYER=OFF
  -DSLANG_ENABLE_TESTS=OFF
  -DSLANG_ENABLE_EXAMPLES=OFF
)

# "YYYY-MM-DD full-sha" — EOD first-parent tip per active day after v2026.10,
# oldest first. Full SHAs are used so track.py register --commit stores a
# collision-safe identifier regardless of future repo growth.
COMMITS=(
  "2026-05-29 8aee8c93225292cef638117d9e4061e4810df0c5"
  "2026-05-30 fb51dcf9253eb20b6d98173fb96a6ee5bdb5ed47"
  "2026-06-01 aaa5f89dd1a8e9ba0ced7c27a7d343ebbc55117c"
  "2026-06-02 adc996670ec281aa8a4ee131f30b324648cbbe60"
  "2026-06-03 eb0dba8214c31c54e8d37f440b143ffde1c167b5"
  "2026-06-04 564ac9f050d6569efd773e2f74e7d067a4e54baa"
  "2026-06-05 5230a81f2fe68afe5cb8d04a1b09d56476f6b960"
  "2026-06-08 6b9f98ff90facc35306a0ba643dfecb59a870156"
  "2026-06-09 29e69b0bf626f87500be73a7fb3764db25658c66"
  "2026-06-10 b2ec9a4f764b08839fc73350b953eeb2a3d2107c"
  "2026-06-11 45c04170f21cd797f1b01b010c2f44a2b91896b4"
  "2026-06-12 736e3a2429223f4173f9afdc03cbd7bd78d7741f"
)

cd "$REPO" || exit 1

# Configure once with release-matching flags. cmake re-runs configure automatically
# on later builds if CMakeLists.txt changes, so a single configure suffices.
echo "=== configure (release-matching: LTO on) ==="
cmake --preset default "${CMAKE_FLAGS[@]}" >/tmp/daily_configure.log 2>&1 || {
  echo "CONFIGURE FAILED (see /tmp/daily_configure.log):"
  tail -15 /tmp/daily_configure.log
  exit 1
}

for entry in "${COMMITS[@]}"; do
  date=${entry%% *}
  sha=${entry##* }
  short=${sha:0:7}
  label="${date}-${short}"
  ddir="$OUT/daily/$label"
  if [[ -f "$ddir/results.json" ]]; then
    echo "=== [$label] already done, skipping ==="
    continue
  fi
  echo "=== [$label] checkout $sha ==="
  git -C "$REPO" checkout -q "$sha" || {
    echo "checkout failed"
    continue
  }
  git -C "$REPO" submodule update --init --recursive >/dev/null 2>&1
  # Pin the build system's git-version file to a fixed string so incremental
  # builds across commits don't regenerate the core module every time the real
  # git version changes (which would defeat the "build only changed TUs" goal).
  # This file is not committed; the measured binary is correct — only its
  # self-reported version tag is fixed.
  printf 'v2026.10\n' >"$REPO/cmake/slang_git_version"
  echo "--- build slangc + slang-glslang (LTO; full optimize per commit) ---"
  if ! cmake --build "$REPO/build" --config Release --target slangc slang-glslang >/tmp/build_$label.log 2>&1; then
    echo "BUILD FAILED for $label (see /tmp/build_$label.log):"
    tail -15 /tmp/build_$label.log
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
