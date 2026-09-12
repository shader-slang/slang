#!/usr/bin/env bash
#
# Configure and build Slang once per (hash function, hash map) combination so
# that the alternatives selected by SLANG_HASH and SLANG_HASHMAP can be checked
# for compilability and benchmarked against each other. See the "Hash map and
# hash function selection" section of docs/building.md.
#
# Every combination is configured in parallel and then built in parallel, with
# the available cores divided between the builds, so that a combination which
# fails to compile shows up early rather than after every preceding build has
# finished.
#
# Each combination gets its own build directory, build-<hash>-<map>, in the
# repository root, so that runs are incremental and the ordinary ./build
# directory is left alone. Directories are never deleted; remove them yourself
# when you are done, they are large.
#
# Usage:
#   extras/hashmap-matrix.sh                 # one build per map and per hash
#   extras/hashmap-matrix.sh --all           # the full cross product
#   extras/hashmap-matrix.sh ABSL_FLAT/BOOST TSL_ROBIN/WYHASH
#
# A combination is written MAP/HASH. Each line of output is one result.
#
# By default every combination is built at once, with the machine's cores divided
# evenly between them. Two environment variables override that:
#
#   SLANG_HASHMAP_MATRIX_JOBS=N    build at most N combinations at a time
#   SLANG_HASHMAP_MATRIX_CORES=N   give each build N cores
#
# Setting a third runs the test suite against each combination that built, one
# combination at a time, after all the builds have finished. Its value is passed
# to sti as filter regexes, so a subset can be picked:
#
#   SLANG_HASHMAP_MATRIX_TEST='^tests/compute' extras/hashmap-matrix.sh
#
# A fourth caps how many tests sti runs at once:
#
#   SLANG_HASHMAP_MATRIX_TEST_JOBS=N  run at most N tests concurrently

set -u

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$repo_root" || exit 1

maps=(
  UNORDERED_DENSE
  BOOST_FLAT
  BOOST_NODE
  BOOST_UNORDERED
  ABSL_FLAT
  ABSL_NODE
  TSL_ROBIN
  STD
)
hashes=(WYHASH BOOST ABSL STD)
default_map=UNORDERED_DENSE
default_hash=WYHASH

# Build the default target rather than a hand-picked list of targets.
#
# Naming targets explicitly is tempting, because only `slang` and `slang-rt`
# actually contain `Slang::Dictionary` and so are all that is needed to prove a
# combination *compiles*. But the test suite needs more than the executables: a
# number of tests `import` standard modules that are compiled during the build
# by `ALL`-only custom targets, such as `slang-numerics-modules`, which produces
# the `slang.numerics` artifacts that `tests/numerics/` imports. A named-target
# build never runs those, and the tests then fail with "cannot open file
# 'slang/numerics/differentiable.slang'" -- a failure that looks like a hash map
# bug but is not one.
#
# Rather than enumerate those targets and get it wrong again the next time one
# is added, build everything the configuration asks for. The configure step
# below already turns off the expensive parts we do not need.

# Tests to run against each combination, as sti filter regexes. Empty means the
# combinations are only built, not run.
read -ra test_filters <<<"${SLANG_HASHMAP_MATRIX_TEST:-}"

# How many tests sti runs at once. Above roughly ten, runs start reporting
# failures and segfaults that do not reproduce when the same tests are rerun on
# their own. That noise is particularly unwelcome here, where the whole point is
# to compare one combination's failures against another's.
test_jobs=${SLANG_HASHMAP_MATRIX_TEST_JOBS:-10}

# A nix devShell exports the flags this checkout is normally configured with
# (system LLVM, system DXC, ...). Honour them so these builds match the
# developer's ordinary build, and let them override the defaults below.
nix_flags=()
if [ -n "${cmakeFlags:-}" ]; then
  read -ra nix_flags <<<"$cmakeFlags"
fi

# Pick the combinations to build.
combos=()
case "${1:-}" in
"")
  for map in "${maps[@]}"; do
    combos+=("$map/$default_hash")
  done
  for hash in "${hashes[@]}"; do
    [ "$hash" = "$default_hash" ] && continue
    combos+=("$default_map/$hash")
  done
  ;;
--all)
  for map in "${maps[@]}"; do
    for hash in "${hashes[@]}"; do
      combos+=("$map/$hash")
    done
  done
  ;;
*)
  combos=("$@")
  ;;
esac

# Reorder the combinations so that a spanning subset comes first: every map and
# every hash is exercised at least once before any pair is repeated. The two are
# chosen independently and most compile failures are caused by one of them alone,
# so this prefix -- eight or so builds out of the full thirty-two -- is where
# nearly all the information is. The remainder still runs, and confirms that no
# failure needs a *particular* pairing, but it can be cut short with much less
# lost.
#
# Both the spanning prefix and the remainder are shuffled, so that repeated runs
# pick different representatives rather than re-proving the same ones.
span_first() {
  local -a shuffled=()
  mapfile -t shuffled < <(printf '%s\n' "$@" | shuf)

  local -A seen_map=() seen_hash=()
  local -a spanning=() remainder=()
  local combo map hash
  for combo in "${shuffled[@]}"; do
    IFS=/ read -r map hash <<<"$combo"
    if [ -z "${seen_map[$map]:-}" ] || [ -z "${seen_hash[$hash]:-}" ]; then
      seen_map[$map]=1
      seen_hash[$hash]=1
      spanning+=("$combo")
    else
      remainder+=("$combo")
    fi
  done
  printf '%s\n' "${spanning[@]}" "${remainder[@]}"
}

if command -v shuf >/dev/null 2>&1; then
  mapfile -t combos < <(span_first "${combos[@]}")
fi

# How many builds to run at once, and how to share the machine between them.
# Running every combination concurrently finds a compile error in any of them
# soonest, but gives each build so few cores that none of them finishes quickly;
# a smaller SLANG_HASHMAP_MATRIX_JOBS trades that breadth for combinations that
# actually complete.
total_cores=$(nproc 2>/dev/null || echo 4)
max_jobs=${SLANG_HASHMAP_MATRIX_JOBS:-${#combos[@]}}
[ "$max_jobs" -lt 1 ] && max_jobs=1
cores_each=${SLANG_HASHMAP_MATRIX_CORES:-$((total_cores / max_jobs))}
[ "$cores_each" -lt 1 ] && cores_each=1

build_dir_for() {
  printf 'build-%s' "$(printf '%s-%s' "$2" "$1" | tr '[:upper:]' '[:lower:]')"
}

configure_one() {
  local map="$1" hash="$2"
  local dir
  dir=$(build_dir_for "$map" "$hash")
  local log="$dir/hashmap-matrix.log"

  mkdir -p "$dir" || return 1
  : >"$log"

  # slang-test is only defined when SLANG_ENABLE_TESTS is on, and CMake refuses
  # that combination unless SLANG_ENABLE_SLANG_RHI is on too, so both are set
  # together. gfx is on for the same reason: it generates the gfx.slang module,
  # without which tests that import it fail for a reason that has nothing to do
  # with the hash map under test.
  if cmake -S . -B "$dir" -G "Ninja Multi-Config" \
    -DSLANG_ENABLE_TESTS=ON \
    -DSLANG_ENABLE_SLANG_RHI=ON \
    -DSLANG_ENABLE_GFX=ON \
    -DSLANG_ENABLE_EXAMPLES=OFF \
    -DSLANG_ENABLE_REPLAYER=OFF \
    -DSLANG_ENABLE_PCH=OFF \
    "${nix_flags[@]}" \
    -DSLANG_HASHMAP="$map" \
    -DSLANG_HASH="$hash" \
    >>"$log" 2>&1; then
    return 0
  fi
  echo "$map/$hash: CONFIGURE FAILED ($log)"
  return 1
}

build_one() {
  local map="$1" hash="$2"
  local dir
  dir=$(build_dir_for "$map" "$hash")
  local log="$dir/hashmap-matrix.log"

  # Builds run in background subshells, so success is recorded on disk rather
  # than in a shell array. The test phase below reads these markers to decide
  # which combinations are worth running.
  rm -f "$dir/hashmap-matrix.built"

  if cmake --build "$dir" --config Debug --parallel "$cores_each" >>"$log" 2>&1; then
    : >"$dir/hashmap-matrix.built"
    echo "$map/$hash: OK ($dir)"
    return 0
  fi
  {
    echo "$map/$hash: BUILD FAILED ($log)"
    grep -E "error:" "$log" | sort -u | head -20
  }
  return 1
}

# Run the requested tests against one combination's own slang-test. sti is given
# that binary explicitly, because it otherwise picks the newest build directory
# on the machine, which during a matrix run is some other combination entirely.
test_one() {
  local map="$1" hash="$2"
  local dir
  dir=$(build_dir_for "$map" "$hash")
  local log="$dir/hashmap-matrix-test.log"

  # `--retry-crashes` retries a crashing test as an ordinary failure rather
  # than letting it take its whole batch down, which keeps one flaky test from
  # obscuring the rest of the run. Note the cost: a test that crashes once and
  # passes on retry is reported as passing, so an *intermittent* crash caused
  # by the hash map under test would be hidden. Re-run a suspect combination
  # without this flag before concluding it is clean.
  if sti --retry-crashes -j "$test_jobs" --slang-test "$dir/Debug/bin/slang-test" \
    "${test_filters[@]}" >"$log" 2>&1; then
    echo "$map/$hash: TESTS PASSED ($dir)"
    return 0
  fi
  {
    echo "$map/$hash: TESTS FAILED ($log)"
    grep -E "^(failed|FAILED|error)" "$log" | head -20
    tail -5 "$log"
  }
  return 1
}

# Configure everything first: a configure failure is cheap to hit and there is
# no point starting long builds for the combinations that did succeed until we
# know which ones those are.
echo "Configuring ${#combos[@]} combinations..."
configured=()
pids=()
for combo in "${combos[@]}"; do
  IFS=/ read -r map hash <<<"$combo"
  configure_one "$map" "$hash" &
  pids+=("$!")
done
for i in "${!pids[@]}"; do
  if wait "${pids[$i]}"; then
    configured+=("${combos[$i]}")
  fi
done

status=0
[ "${#configured[@]}" -ne "${#combos[@]}" ] && status=1

if [ "${#configured[@]}" -eq 0 ]; then
  exit 1
fi

echo "Building ${#configured[@]} combinations, $max_jobs at a time, $cores_each core(s) each..."
running=0
for combo in "${configured[@]}"; do
  # Wait for a slot before starting the next build. `wait -n` returns as soon
  # as any one of the background builds finishes, and yields that build's exit
  # status, so a failure anywhere is still recorded.
  while [ "$running" -ge "$max_jobs" ]; do
    wait -n || status=1
    running=$((running - 1))
  done
  IFS=/ read -r map hash <<<"$combo"
  build_one "$map" "$hash" &
  running=$((running + 1))
done
while [ "$running" -gt 0 ]; do
  wait -n || status=1
  running=$((running - 1))
done

# Run the tests one combination at a time, in the same order the builds were
# started. sti already saturates the machine by itself, so there is nothing to
# gain from overlapping two of them, and keeping them serial means a failure is
# attributable to one combination rather than to contention between several.
if [ "${#test_filters[@]}" -gt 0 ]; then
  echo "Testing with filters: ${test_filters[*]}"
  for combo in "${configured[@]}"; do
    IFS=/ read -r map hash <<<"$combo"
    dir=$(build_dir_for "$map" "$hash")
    [ -e "$dir/hashmap-matrix.built" ] || continue
    test_one "$map" "$hash" || status=1
  done
fi

exit $status
