#!/usr/bin/env bash
#
# Benchmark several builds of Slang against every compile-perf workload, over and over, in a
# different random order each time.
#
# The unit of work is one workload measured against one build: the smallest piece that still
# carries its own warmup, and so the smallest that is worth shuffling.
#
# That matters because the differences being looked for are small, a few percent, and the machine
# does not hold still for the hours a comparison takes: it warms up, other work starts and stops,
# the page cache fills. Anything that drifts lands on whatever was being measured at the time.
# Shuffling the whole (build x workload) list, and reshuffling on every pass, spreads that across
# all of them rather than letting it settle on whichever build sits at a fixed position in a
# fixed order. Two measurements of the same build usually end up far apart in time.
#
# The loop does not end on its own. Interrupt it whenever: each measurement is recorded as it
# finishes, so the results are complete up to that point, and restarting skips what is already
# recorded rather than repeating the pass. Each pass is one more sample of everything.
#
# Usage:
#   extras/bench-loop.sh                              # every build-*/ with a Release slangc
#   extras/bench-loop.sh build-a build-b              # just these two
#   extras/bench-loop.sh --name maps-after-merge ...  # name the run
#   extras/bench-loop.sh --config Debug build-a       # measure the Debug binaries instead
#
# Options, all of which have a sensible default:
#
#   --name NAME                  what to call this run. Everything it writes lives under
#                                ~/work/tmp/bench-loop/NAME. Restarting under the same name
#                                resumes; a new name starts a fresh set of results. Defaults to
#                                the time the run started.
#   --config CONFIG              which build configuration's binaries to measure. Release by
#                                default, because Debug timings say nothing useful.
#   SLANG_BENCH_PASSES=N         stop after N passes instead of running until interrupted. A
#                                pass is one measurement of every (build, workload) pair, so
#                                this is "take N samples of everything, then stop". 0, the
#                                default, means keep going.
#   SLANG_BENCH_SAMPLES=N        timed runs per measurement. 1 by default; see below.
#   SLANG_BENCH_WARMUP=N         untimed runs before each measurement. 1 by default.
#   SLANG_BENCH_RUN=NAME         same as --name.
#   SLANG_BENCH_WORK=DIR         where to put everything. ~/work/tmp by default.
#
# Nothing is written to /tmp: results, logs, the scratch directory bench.py generates sources
# into, and TMPDIR for everything it spawns all point under ~/work/tmp, which survives a reboot.
#
# Note this measures each workload at its own size -- the `n=` in bench.py's output -- exactly as
# the suite defines it. The workload is never subdivided; the only thing this script splits up is
# the repeats.

set -u

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
perf_dir="$repo_root/tools/compile-perf"

# Each run is named, and everything it writes lives under that name. Two runs measuring
# different things -- a different set of builds, a rebuilt tree, a different sample count -- then
# cannot be mistaken for repeats of each other, and one can be left running while another is
# started. The default names the run for when it began.
run_name=${SLANG_BENCH_RUN:-run-$(date +%Y%m%d-%H%M%S)}

# bench.py's defaults: one warmup then five timed runs. The warmup is what stops the first run of
# a measurement, which pays for a cold page cache, from being counted; five timed runs amortise
# it, where one sample per measurement would spend a third of the run warming up.
#
# The consequence for reading the results: those five runs happen within seconds of each other,
# so they see the same machine and their spread understates the real one. Treat a measurement's
# median as a single observation and take the spread across passes instead. Measured here, that
# difference is large -- one binary measured twice looked repeatable to 0.3%, while the same
# comparison across five passes moved by up to 7%.
samples=${SLANG_BENCH_SAMPLES:-5}
warmup=${SLANG_BENCH_WARMUP:-1}
max_passes=${SLANG_BENCH_PASSES:-0} # 0 means keep going
config=Release

args=()
while [ "$#" -gt 0 ]; do
  case "$1" in
  --name)
    run_name=$2
    shift 2
    ;;
  --name=*)
    run_name=${1#--name=}
    shift
    ;;
  --config)
    config=$2
    shift 2
    ;;
  --config=*)
    config=${1#--config=}
    shift
    ;;
  -h | --help)
    # The comment block at the top of this file is the documentation, so print that rather than
    # keeping a second copy here for the two to drift apart. Skips the shebang and stops at the
    # first line that is not a comment.
    awk 'NR == 1 { next } /^#/ { sub(/^# ?/, ""); print; next } { exit }' "${BASH_SOURCE[0]}"
    exit 0
    ;;
  *)
    args+=("$1")
    shift
    ;;
  esac
done

work="${SLANG_BENCH_WORK:-$HOME/work/tmp}/bench-loop/$run_name"
results="$work/results"
logs="$work/logs"
scratch="$work/scratch"
state="$work/state"
progress="$work/progress.log"
mkdir -p "$results" "$logs" "$scratch" "$state" || exit 1

# The log file is written first and on its own, then the same line is echoed for anyone
# watching. Writing both through `tee` ties the log to standard output: if that goes away -- a
# terminal closing, or the run being piped into something that stops reading -- `tee` goes with
# it and the log stops, while the run itself carries on invisibly. The file is the record that
# matters, so nothing that happens to stdout is allowed to interrupt it.
say() {
  local line
  line="$(date +%Y-%m-%dT%H:%M:%S)  $*"
  printf '%s\n' "$line" >>"$progress"
  printf '%s\n' "$line" 2>/dev/null || true
}

# Which builds to measure: those named, or every build directory that has a binary to measure.
# A directory without one is skipped rather than being an error, so a half-built matrix can be
# measured for whatever part of it is ready.
candidates=()
if [ "${#args[@]}" -gt 0 ]; then
  candidates=("${args[@]}")
else
  for dir in "$repo_root"/build-*/; do
    candidates+=("${dir%/}")
  done
fi

builds=()
labels=()
for dir in "${candidates[@]}"; do
  # Resolve to an absolute path, accepting one relative to the working directory or to the
  # repository. It has to be absolute: bench.py is run from tools/compile-perf, so a relative
  # path would be resolved against that instead, and the build would appear not to exist.
  if [ -d "$dir" ]; then
    dir=$(cd "$dir" && pwd)
  elif [ -d "$repo_root/$dir" ]; then
    dir=$(cd "$repo_root/$dir" && pwd)
  else
    continue
  fi
  slangc="$dir/$config/bin/slangc"
  if [ ! -x "$slangc" ]; then
    continue
  fi
  builds+=("$slangc")
  # The label names the build, so results from different builds sit side by side and a later
  # comparison can tell them apart: `build-wyhash-boost_flat` becomes `wyhash-boost_flat`.
  label=$(basename "$dir")
  labels+=("${label#build-}")
done

if [ "${#builds[@]}" -eq 0 ]; then
  say "no build has a $config slangc to measure"
  exit 1
fi

# The workloads come from the suite's own manifest rather than a list kept here, so one added to
# the suite is picked up without this script being touched.
mapfile -t workloads < <(cd "$perf_dir" && python3 -c "
import sys
sys.path.insert(0, '.')
from lib.manifest import WORKLOADS
for w in WORKLOADS:
    print(w.name)
" 2>/dev/null)

if [ "${#workloads[@]}" -eq 0 ]; then
  say "could not read the workload manifest from $perf_dir"
  exit 1
fi

say "builds: ${#builds[@]} ($config), workloads: ${#workloads[@]}, measurements per pass: $((${#builds[@]} * ${#workloads[@]}))"
say "results under $results"

pass=0
while true; do
  pass=$((pass + 1))
  if [ "$max_passes" -gt 0 ] && [ "$pass" -gt "$max_passes" ]; then
    break
  fi

  # The order for this pass: the workloads shuffled, and within each workload every build run
  # back to back, in its own shuffled order.
  #
  # Grouping by workload is the important part. What is being compared is the ratio between two
  # builds on the same workload, so those two measurements want to be as close together in time
  # as they can be: then whatever the machine is doing applies to both and cancels out of the
  # ratio. Shuffling the (build, workload) pairs globally does the opposite, putting the two
  # halves of a comparison as far apart as the pass is long and charging whatever drifted in
  # between to the difference between the builds. tools/compile-perf/ab.py makes the same point
  # about comparing two bench.py runs ninety seconds apart: it measured a binary against itself
  # that way and found an 0.8% difference at p < 0.005.
  #
  # Reshuffling the build order for every workload and every pass keeps any one build from
  # always being measured first, when the file cache is coldest, or always last.
  mapfile -t plan < <(
    while IFS= read -r workload; do
      while IFS= read -r i; do
        printf '%s\t%s\t%s\n' "$i" "${labels[$i]}" "$workload"
      done < <(printf '%s\n' "${!builds[@]}" | shuf)
    done < <(printf '%s\n' "${workloads[@]}" | shuf)
  )

  say "=== pass $pass: ${#plan[@]} measurements"
  measured=0
  skipped=0

  for entry in "${plan[@]}"; do
    IFS=$'\t' read -r index label workload <<<"$entry"

    # The marker is written only once bench.py has returned, so a measurement interrupted part
    # way through is taken again rather than being counted as done.
    marker="$state/p$pass-$label-$workload.done"
    if [ -f "$marker" ]; then
      skipped=$((skipped + 1))
      continue
    fi

    gen="$scratch/p$pass-$label-$workload"
    log="$logs/p$pass-$label-$workload.log"

    # One label per (pass, build): bench.py merges into an existing results.json, so a pass's
    # workloads accumulate in one file while each pass keeps its own.
    #
    # TMPDIR is pointed at the scratch directory as well as --gen-dir, so that the compilers
    # bench.py spawns keep their temporaries here too rather than in /tmp, which this machine
    # clears on reboot.
    mkdir -p "$gen"
    (cd "$perf_dir" && TMPDIR="$gen" timeout --kill-after=60 1800 python3 bench.py \
      --slangc "${builds[$index]}" \
      --label "p$pass-$label" \
      --only "$workload" \
      --samples "$samples" \
      --warmup "$warmup" \
      --out "$results" \
      --gen-dir "$gen" \
      >"$log" 2>&1)
    local_status=$?

    # bench.py exits non-zero if any workload in the run failed, which includes one whose corpus
    # is absent. That is a fact about the workload, not about the build, so record it and carry
    # on rather than stopping the loop.
    if [ "$local_status" -ne 0 ]; then
      printf '%s\t%s\t%s\t%s\n' "$(date +%H:%M:%S)" "p$pass-$label" "$workload" \
        "$(grep -aoE '(FAIL|error)[^\"]*' "$log" | head -1)" >>"$work/skipped.tsv"
    fi

    : >"$marker"
    rm -rf "${gen:?}"
    measured=$((measured + 1))

    # Often enough to see progress, seldom enough not to bury the log.
    if [ $((measured % 25)) -eq 0 ]; then
      say "  pass $pass: $measured measured, $skipped already recorded"
    fi
  done

  say "=== pass $pass complete: $measured measured, $skipped skipped"
done
