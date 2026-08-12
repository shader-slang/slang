#!/usr/bin/env python3
"""Slang compile-time perf-suite runner.

Drives a given slangc over the workloads in manifest.py, parses the per-phase
timers emitted by -report-perf-benchmark, and writes per-run JSON: a summary
(median/min/max/mean/stdev/n) AND the raw samples per timer; merge-on-write.

Stdlib only (no prettytable / numpy) so it runs unchanged against any release's
slangc.

Examples:
    # Run the whole suite at default sizes with the local build:
    python3 bench.py --slangc ../../../build/RelWithDebInfo/bin/slangc --label dev

    # One workload, more samples:
    python3 bench.py --slangc /path/slangc --label v2026.9 \\
        --only autodiff --samples 7
"""
import argparse
import json
import os
import re
import shutil
import statistics
import subprocess
import sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import tempfile
import time

from lib import analyze, corpus, manifest


def parse_timers(text):
    """Extract {phase: total_ms} from slangc -report*-perf-benchmark output.

    Lines look like:  [*]   compileInner \t 1 \t   316.07ms [\t 0.0040ms/op]
    The total-ms token is the first (from the right) bare '...ms' token; the
    per-op token ends in 'ms/op' and is skipped.
    """
    out = {}
    for line in text.splitlines():
        line = line.strip()
        if not line.startswith("[*]"):
            continue
        toks = line.split()
        if len(toks) < 4:
            continue
        name = toks[1]
        for tok in reversed(toks):
            if tok.endswith("ms") and not tok.endswith("ms/op"):
                try:
                    out[name] = float(tok[:-2])
                except ValueError:
                    sys.stderr.write(f"compile-perf: failed to parse timer value in: {line!r}\n")
                break
    return out


def stats(values):
    """Summarize repeated measurements, keeping the raw samples alongside.

    The samples are retained, not only the summary, because any summary is
    lossy in a way that cannot be undone: a bimodal five-sample run and a
    tight one can share a median and a stdev, and only the samples tell them
    apart. results.json is the archive, so a question that needs them later
    cannot be answered by re-deriving them. They also let a consumer compute
    statistics under its own definition instead of trusting ours — the
    BenchView submission format, for one, computes its own summary from
    samples and treats that as authoritative.

    `max` is reported for symmetry with `min`: without it the spread cannot
    be bounded from the summary alone, and consumers that accept a summary in
    place of samples generally require both extrema.
    """
    values = [v for v in values if v is not None]
    if not values:
        return None
    return {
        "median": round(statistics.median(values), 4),
        "min": round(min(values), 4),
        "max": round(max(values), 4),
        "mean": round(statistics.mean(values), 4),
        "stdev": round(statistics.stdev(values), 4) if len(values) > 1 else 0.0,
        "n": len(values),
        # NOT rounded, unlike the summary fields above. Rounding the samples
        # would defeat their purpose twice over: a consumer recomputing
        # statistics would be working from altered measurements, and where a
        # consumer checks our summary against its own (BenchView does, within
        # 1e-9) a summary derived from RAW values will not match one derived
        # from rounded samples — measured at ~80% of five-sample sets.
        "samples": list(values),
    }


# Base flag (not -report-detailed-perf-benchmark): the base flag already emits
# every phase timer the suite uses and is supported across the whole release
# window; the detailed flag was added mid-window and only adds finer sub-timers.
PERF_FLAG = "-report-perf-benchmark"

HERE = os.path.dirname(os.path.abspath(__file__))


# slangc infers the requested artifact from the -o extension for some targets
# (and per-entry-point output binding for the text targets), so the output name
# must match the -target rather than always being out.spv.
_TARGET_EXT = {"spirv": "spv", "dxil": "dxil", "ptx": "ptx", "metal": "metal",
               "wgsl": "wgsl", "hlsl": "hlsl", "glsl": "glsl", "cuda": "cu"}


def _target_ext(extra_flags):
    for i, flag in enumerate(extra_flags):
        if flag == "-target" and i + 1 < len(extra_flags):
            return _TARGET_EXT.get(extra_flags[i + 1], "out")
    return "spv"


def find_libslang(slangc):
    """Locate the slang shared library belonging to a slangc binary, trying the
    layouts of release packages (bin/ + ../lib/) and build trees (same dir).
    The renamed slang-compiler library is preferred: on Windows the legacy
    slang.dll is only a forwarding proxy, and resolving a forwarded export
    requires the loader to find slang-compiler.dll through the normal DLL
    search order — which does not include the proxy's own directory when the
    driver loads it by absolute path from elsewhere. Loading the real library
    directly avoids that; the legacy names remain as fallback for pre-rename
    releases. Returns None when not found — api
    workloads then fail with a clear error while slangc workloads run
    normally."""
    d = os.path.dirname(slangc)
    for cand in (
        os.path.join(d, "slang-compiler.dll"),
        os.path.join(d, "slang.dll"),
        os.path.join(d, "libslang-compiler.dylib"),
        os.path.join(d, "libslang.dylib"),
        os.path.join(d, "libslang-compiler.so"),
        os.path.join(d, "libslang.so"),
        os.path.join(d, "..", "lib", "libslang-compiler.dylib"),
        os.path.join(d, "..", "lib", "libslang.dylib"),
        os.path.join(d, "..", "lib", "libslang-compiler.so"),
        os.path.join(d, "..", "lib", "libslang.so"),
    ):
        if os.path.exists(cand):
            return os.path.abspath(cand)
    return None


def build_api_driver(out_dir):
    """Compile native/api-driver.cpp once per bench invocation with the host
    compiler. The driver dlopens whatever libslang it is pointed at, so one
    host build measures every release in a sweep. Returns the binary path, or
    None (with a message) when no host compiler is available."""
    src = os.path.join(HERE, "native", "api-driver.cpp")
    inc = os.path.join(HERE, "..", "..", "include")
    if not os.path.exists(os.path.join(inc, "slang.h")):
        sys.stderr.write(f"compile-perf: include/slang.h not found near {inc}\n")
        return None
    is_win = sys.platform == "win32"
    out = os.path.join(out_dir, "api-driver.exe" if is_win else "api-driver")
    if is_win:
        # Dash-style flags: identical to /flags for cl, but immune to MSYS/Git-
        # Bash path mangling if this command ever runs through a POSIX shell.
        cmd = ["cl.exe", "-nologo", "-O2", "-std:c++17", "-EHsc", f"-I{inc}", src,
               f"-Fe:{out}"]
    else:
        cmd = ["c++", "-O2", "-std=c++17", "-I", inc, src, "-o", out]
        # dlopen/dlsym live in libdl on pre-2.34 glibc; harmless elsewhere.
        if sys.platform.startswith("linux"):
            cmd.append("-ldl")
    try:
        r = subprocess.run(cmd, cwd=out_dir, stdout=subprocess.PIPE,
                           stderr=subprocess.STDOUT, timeout=300)
    except (FileNotFoundError, subprocess.TimeoutExpired) as e:
        sys.stderr.write(f"compile-perf: cannot build api-driver: {e}\n")
        return None
    if r.returncode != 0:
        sys.stderr.write("compile-perf: api-driver build failed:\n"
                         + r.stdout.decode("utf-8", "replace") + "\n")
        return None
    return out


def api_driver_supports_out_dir(driver):
    """Return whether `driver` understands --out-dir, by reading its own usage.

    Only an externally supplied --api-driver can be too old for the flag; the
    one bench.py builds comes from native/api-driver.cpp in this checkout. The
    usage banner is the capability signal precisely because it lives in that
    same file: a binary old enough to lack --out-dir prints a banner old enough
    to lack the line, so the two cannot drift apart. Run with no arguments the
    driver prints usage and exits 2, which is what this reads.
    """
    try:
        r = subprocess.run([driver], stdout=subprocess.PIPE,
                           stderr=subprocess.STDOUT, timeout=60)
    except (OSError, subprocess.TimeoutExpired):
        return False
    return b"--out-dir" in r.stdout


def build_commands(slangc, spec, src_dir, files, out_dir, size=None, api=None):
    """Return (commands, primary_outfile_for_parsing_index).

    `src_dir` holds the workload's .slang sources and is treated as READ-ONLY;
    every artifact the compiler produces — the -o output, precompiled
    .slang-module files, reflection JSON — goes to `out_dir`. The two are the
    same directory on the default path, and differ under `bench.py --corpus`,
    where src_dir is a corpus prepared by another job or another machine and is
    not ours to write into (it may be read-only, or shared by several runs).
    Separating them here rather than at the call site keeps every artifact path
    in one function, so a new one cannot quietly default to the corpus.

    For "link" mode the timed command is the final main compile (last element);
    module precompiles are setup and run once (not timed).
    For "api" mode the timed command is the api-driver (api = {"driver", "libslang"});
    the driver emits [*] timer lines in the slangc report format."""
    if spec.mode == "api":
        timed = [api["driver"], api["libslang"], spec.api_cmd]
        if spec.api_cmd == "session-create":
            timed += ["--iters", str(size)]
        else:
            # --out-dir for the same reason as -o below: module-graph-bin
            # serializes .slang-module binaries, and the driver writes them
            # there instead of beside the sources it read.
            timed += ["--dir", src_dir, "--out-dir", out_dir]
        if spec.api_root:
            timed += ["--root", spec.api_root]
        timed += spec.api_flags
        return {"setup": [], "timed": timed}
    main = next((f for f in files if "main" in f), None)
    if spec.mode == "module":
        f = list(files)[0]
        out = os.path.join(out_dir, "out.slang-module")
        return {
            "setup": [],
            "timed": [slangc, PERF_FLAG, os.path.join(src_dir, f),
                      *spec.extra_flags, "-o", out],
        }
    if spec.mode == "link":
        setup = []
        for f in files:
            if f == main:
                continue
            setup.append([slangc, os.path.join(src_dir, f), "-o",
                          os.path.join(out_dir, f.replace(".slang", ".slang-module"))])
        out = os.path.join(out_dir, "out.spv")
        # BOTH roots on the include path, out_dir first: the precompiled
        # modules live there while their sources live in src_dir, and the link
        # must resolve an import to the .slang-module rather than recompile the
        # .slang next to it — which is what this workload measures. Deduped so
        # the default path, where the two roots ARE one directory, emits the
        # single -I it always did and its recorded cmd stays comparable with
        # every result already in the series.
        includes = []
        for d in (out_dir, src_dir):
            if d not in includes:
                includes += ["-I", d]
        timed = [slangc, PERF_FLAG, *includes,
                 os.path.join(src_dir, main), *spec.extra_flags, "-o", out]
        return {"setup": setup, "timed": timed}
    # "target" mode: single or multi-file compile to a GPU target. For single-file
    # workloads spec.main_file (or the first file) is the entry point. For corpus
    # workloads (e.g. mdl_dxr), spec.main_file names the root; -I src_dir lets
    # sibling imports resolve without explicit paths. reflection_json attaches a
    # per-run output path so the layout/reflection serializer is exercised without
    # polluting the results directory.
    f = spec.main_file or main or list(files)[0]
    out = os.path.join(out_dir, "out." + _target_ext(spec.extra_flags))
    extra = list(spec.extra_flags)
    # reflection JSON needs a writable path; out_dir is per-run and writable.
    if getattr(spec, "reflection_json", False):
        extra += ["-reflection-json", os.path.join(out_dir, "reflect.json")]
    # -I src_dir lets multi-file corpora resolve imports; harmless for single files
    return {
        "setup": [],
        "timed": [slangc, PERF_FLAG, "-I", src_dir, os.path.join(src_dir, f),
                  *extra, "-o", out],
    }


# GNU /usr/bin/time -v gives per-process peak RSS; detect once.
def _detect_gnu_time():
    try:
        r = subprocess.run(["/usr/bin/time", "-v", "true"],
                           stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        return b"Maximum resident set size" in r.stderr
    except Exception:  # noqa: BLE001
        return False


_GNU_TIME = _detect_gnu_time()


def run_once(cmd):
    """Run one compile; return (rc, wall_ms, combined_text, rss_kb_or_None).

    When GNU time is available the command is wrapped so its peak RSS is written
    to a side file (keeping the compiler's own stdout/stderr clean for parsing)."""
    memfile = None
    runcmd = cmd
    if _GNU_TIME:
        memfd, memfile = tempfile.mkstemp(prefix="bench_mem_")
        os.close(memfd)
        runcmd = ["/usr/bin/time", "-v", "-o", memfile] + cmd
    t0 = time.perf_counter()
    proc = subprocess.run(runcmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    wall = (time.perf_counter() - t0) * 1000.0
    text = proc.stdout.decode("utf-8", "replace")
    rss = None
    if memfile:
        try:
            with open(memfile, encoding="utf-8", errors="replace") as fh:
                for line in fh:
                    if "Maximum resident set size" in line:
                        rss = float(line.rsplit(":", 1)[1].strip())  # kbytes
                        break
        except Exception:  # noqa: BLE001
            pass
        os.unlink(memfile)
    return proc.returncode, wall, text, rss


# Benign messages to ignore: a missing downstream tool (spirv-opt/glslang) on
# hosts that lack it. Release tarballs bundle these, so they usually don't fire.
_BENIGN = ("E00100", "E52002", "spirv-opt", "spirv-dis", "slang-glslang",
           "failed to load downstream", "pass-through compiler not found")
# For downstream_required workloads a missing downstream compiler is THE
# failure being guarded against, not noise: slangc can emit its internal
# timers before the downstream handoff, so without this the run would record
# timers and report OK with no DXIL/PTX ever produced. Only genuinely
# irrelevant tool noise stays benign.
_BENIGN_DOWNSTREAM_REQUIRED = ("E52002", "spirv-opt", "spirv-dis", "slang-glslang")
# Matches the modern "error[E30015]:" and legacy "error 30015:" slangc formats,
# plus the api-driver's bare "error: ..." lines.
_ERR_RE = re.compile(r"error\[|: error:|\berror \d+:|^error: ")


def real_error(text, benign=_BENIGN):
    """A genuine compile error in either the modern or legacy slangc format,
    ignoring the given benign diagnostics (by default, missing-downstream-tool
    noise; downstream_required workloads pass a stricter set)."""
    for line in text.splitlines():
        if _ERR_RE.search(line) and not any(b in line for b in benign):
            return line.strip()
    return None


def run_spec(slangc, spec, size, samples, warmup, src_root, out_root, api=None,
             prepared=False):
    src_dir = os.path.join(src_root, corpus.dir_name(spec, size))
    out_dir = os.path.join(out_root, corpus.dir_name(spec, size))
    # Where the sources come from is corpus.py's problem, not this function's:
    # generated here, or already prepared by an earlier step / another machine
    # (bench.py --corpus). Either way what follows measures a directory.
    if prepared:
        files = corpus.prepared_files(src_dir)
        # An empty prepared directory is legitimate for an api workload — the
        # driver takes --dir and reads it itself, and session-create needs no
        # sources at all — but every other mode picks an entry point out of
        # this list, so empty there means the corpus was prepared incompletely
        # or --corpus points a level too high. Named here rather than left to
        # build_commands, whose IndexError would say nothing about which
        # directory to go and look at.
        if not files and spec.mode != "api":
            raise FileNotFoundError(
                f"no .slang files in prepared corpus {src_dir}")
    else:
        files = corpus.materialize(spec, size, src_dir)
    os.makedirs(out_dir, exist_ok=True)

    # An api workload without a driver+libslang must fail loudly (not silently
    # skip): a missing host compiler or unrecognized package layout would
    # otherwise drop the workload from the series with no visible signal.
    if spec.mode == "api" and api is None:
        return {
            "workload": spec.name, "bucket": spec.bucket, "size": size,
            "mode": spec.mode, "ok": False, "setup_ok": False,
            "got_timers": False, "samples": samples, "warmup": warmup,
            "wall_ms": None, "rss_kb": None, "timers": {},
            "primary_timers": spec.primary_timers, "cmd": "",
            "error": "api-driver or libslang unavailable (see stderr)",
            "crash_codes": None,
        }

    cmds = build_commands(slangc, spec, src_dir, files, out_dir, size=size, api=api)
    # A failed setup step (e.g. a module that didn't precompile in link mode) must
    # fail the workload — otherwise the timed compile runs against missing inputs.
    setup_ok = True
    for c in cmds["setup"]:
        try:
            rc = subprocess.run(c, stdout=subprocess.DEVNULL,
                                stderr=subprocess.DEVNULL, timeout=600).returncode
        except subprocess.TimeoutExpired:
            rc = 1
        if rc != 0:
            setup_ok = False

    benign = (_BENIGN_DOWNSTREAM_REQUIRED
              if getattr(spec, "downstream_required", False) else _BENIGN)

    timed = cmds["timed"]
    for _ in range(warmup):
        run_once(timed)

    per_timer = {}
    walls = []
    rsses = []
    last_text = ""
    # Validate EVERY sample: a workload that fails on 2 of 5 runs but succeeds
    # on 3 would look valid if only the last sample were checked. When ALL samples
    # crash, sample_ok is empty (all([]) is True) AND got_timers is False — both
    # independently make ok=False; crash_codes also fires as a third guard.
    sample_ok = []
    crash_codes = []
    for _ in range(samples):
        rc, wall, text, rss = run_once(timed)
        last_text = text
        # rc == 0: success; rc == 1: slangc-reported compile error (caught by
        # real_error(), which marks the sample as failed). rc > 1 or
        # rc < 0: slangc crashed or was killed by a signal (SIGSEGV=139, SIGABRT=134
        # on Linux; large negative values on Windows — Python converts NTSTATUS codes
        # such as 0xC0000005 to signed int: -1073741819). Exit code 2+ from usage errors
        # won't occur here because the bench harness always builds valid invocations.
        # Exclude crashed samples from timing stats; their wall time is meaningless.
        if rc > 1 or rc < 0:
            crash_codes.append(rc)
            sample_ok.append(False)
            continue
        walls.append(wall)
        if rss is not None:
            rsses.append(rss)
        err = real_error(text, benign)
        sample_ok.append(err is None)  # ok when no compile error
        for name, ms in parse_timers(text).items():
            per_timer.setdefault(name, []).append(ms)

    err = real_error(last_text, benign)
    got_timers = bool(per_timer)
    # A run that produced no timers and no recognizable diagnostic would report
    # a bare "no timers" with the actual output lost — surface the first output
    # line (e.g. a loader failure or crash banner) so remote CI runs are
    # debuggable from results.json alone.
    if err is None and not got_timers:
        err = next((ln.strip()[:200] for ln in last_text.splitlines() if ln.strip()), None)
    ok = setup_ok and got_timers and all(sample_ok) and not crash_codes

    return {
        "workload": spec.name,
        "bucket": spec.bucket,
        "size": size,
        "mode": spec.mode,
        "ok": ok,
        "setup_ok": setup_ok,
        "got_timers": got_timers,
        "samples": samples,
        "warmup": warmup,
        "wall_ms": stats(walls),
        "rss_kb": stats(rsses) if rsses else None,
        "timers": {k: stats(v) for k, v in sorted(per_timer.items())},
        "primary_timers": spec.primary_timers,
        "cmd": " ".join(timed),
        "error": err,
        "crash_codes": crash_codes or None,
    }


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    # Required to MEASURE, but not to PREPARE — see the --prepare check below.
    ap.add_argument("--slangc", default=None,
                    help="path to slangc to benchmark (required unless --prepare)")
    ap.add_argument("--label", default=None,
                    help="version/run label, e.g. v2026.9 (required unless --prepare)")
    ap.add_argument("--out", default="results", help="output directory")
    ap.add_argument("--samples", type=int, default=5)
    ap.add_argument("--warmup", type=int, default=1)
    ap.add_argument("--only", default=None,
                    help="comma-separated workload names to run (default all)")
    ap.add_argument("--sweep", action="store_true",
                    help="run each workload's sweep_sizes instead of default_size")
    ap.add_argument("--gen-dir", default=None,
                    help="scratch dir for generated sources + compiled outputs "
                         "(default: a tempdir, auto-removed — keeps the results dir, which "
                         "is committed to the perf-results repo, free of build scratch). "
                         "Pass a path to keep them for inspection.")
    # Generation as a SEPARATE, OPTIONAL step. --prepare writes the corpus and
    # stops; --corpus benches a corpus somebody else wrote. Together they split
    # authoring from measurement across machines: a runner with the tree can
    # prepare, and the quiesced perf machine only measures. Apart from that,
    # the default path is unchanged — prepare-then-bench in one process.
    ap.add_argument("--prepare", metavar="DIR", default=None,
                    help="write the selected workloads' .slang sources to DIR "
                         "as one <workload>_n<size>/ directory per run, then "
                         "exit without benchmarking. Needs no --slangc/--label: "
                         "pass the same DIR to --corpus on the measuring machine")
    ap.add_argument("--corpus", metavar="DIR", default=None,
                    help="bench sources already prepared in DIR (skips "
                         "generation entirely; DIR must contain one "
                         "<workload>_n<size>/ per run)")
    ap.add_argument("--api-driver", default=None,
                    help="prebuilt api-driver binary (default: build it from "
                         "native/api-driver.cpp with the host compiler). Build "
                         "it from THIS tree: the driver is passed --out-dir, "
                         "which one predating that flag ignores, leaving it "
                         "writing module binaries into the --corpus tree")
    ap.add_argument("--libslang", default=None,
                    help="slang shared library for api workloads (default: "
                         "derived from --slangc's package layout)")
    ap.add_argument("--api", action="store_true",
                    help="include the api workloads in the default set. Until "
                         "this is passed (or api workloads are named in --only) "
                         "they are excluded, so existing CI series and published "
                         "reports don't change shape before the history is "
                         "resynced with them (see DESIGN.md 'API-path workloads').")
    args = ap.parse_args()

    # --slangc and --label are checked HERE rather than declared required=True,
    # and validated only after the --prepare return below. Preparing a corpus
    # invokes no compiler and writes no results directory, so a machine that has
    # the tree but no slangc must be able to run it — which is the entire point
    # of splitting the two steps. argparse cannot express "required unless
    # --prepare", so the condition is spelled out.
    if not args.prepare:
        absent = [flag for flag, val in (("--slangc", args.slangc),
                                         ("--label", args.label)) if not val]
        if absent:
            sys.exit(f"{', '.join(absent)} required for a benchmark run "
                     f"(only --prepare runs without them)")

    specs = manifest.WORKLOADS
    if not args.api and not args.only:
        specs = [s for s in specs if s.mode != "api"]
    # Platform-bound workloads (downstream toolchains like dxc/nvrtc) leave the
    # default set on other platforms; naming one in --only runs it regardless —
    # explicit intent fails loudly if the tool is genuinely absent.
    if not args.only:
        skipped = [s2.name for s2 in specs
                   if s2.platforms and sys.platform not in s2.platforms]
        if skipped:
            print(f"[skip] platform-bound workloads not on {sys.platform}: "
                  + ", ".join(skipped))
        specs = [s2 for s2 in specs
                 if not s2.platforms or sys.platform in s2.platforms]
    if args.only:
        want = set(args.only.split(","))
        specs = [s for s in specs if s.name in want]
        missing = want - {s.name for s in specs}
        if missing:
            sys.exit(f"unknown workloads: {sorted(missing)}")

    # Generated sources + compiled outputs are large, transient build scratch; keep
    # them OUT of the results dir so it stores only results.json. Default to a
    # tempdir that is removed at the end (overridable with --gen-dir to keep them).
    if args.corpus and args.prepare:
        sys.exit("--prepare and --corpus are opposite halves of the same split; "
                 "pass one")
    # Directories this process created and therefore must remove; tracked as we
    # go rather than re-derived from the flags at exit, so a root can never be
    # deleted because the conditions drifted apart.
    scratch_roots = []

    def scratch(prefix):
        d = tempfile.mkdtemp(prefix=prefix)
        scratch_roots.append(d)
        return d

    # --corpus reads a prepared tree; --prepare writes one; otherwise scratch.
    src_root = (os.path.abspath(args.corpus) if args.corpus
                else os.path.abspath(args.prepare) if args.prepare
                else os.path.abspath(args.gen_dir) if args.gen_dir
                else scratch("perfsuite_gen_"))

    # --prepare: materialize and stop. No slangc is invoked, so this half of
    # the split runs anywhere — including a machine that has the tree but no
    # business doing timing.
    if args.prepare:
        total = 0
        for spec in specs:
            for size in (spec.sweep_sizes if args.sweep and spec.sweep_sizes
                         else [spec.default_size]):
                dest = os.path.join(src_root, corpus.dir_name(spec, size))
                names = corpus.materialize(spec, size, dest)
                total += len(names)
                print(f"[prep] {spec.name:24s} n={size:<6} {len(names):4d} file(s)")
        print(f"\nwrote {total} file(s) to {src_root}")
        return

    # Everything below MEASURES, so slangc and the results directory are
    # required from here on — and not one line earlier: validating slangc above
    # would fail a --prepare run on a machine that has no compiler, and creating
    # the results directory above would leave an empty one behind that --prepare
    # never writes into.
    slangc = os.path.abspath(args.slangc)
    if not os.path.exists(slangc):
        sys.exit(f"slangc not found: {slangc}")
    root = os.path.join(os.path.abspath(args.out), args.label)
    os.makedirs(root, exist_ok=True)
    # Only when WE produce the sources. A --corpus tree is an input the caller
    # prepared, so creating a missing one would turn "you pointed --corpus at
    # the wrong path" into a silently-created empty directory plus a confusing
    # per-workload failure further down.
    if not args.corpus:
        os.makedirs(src_root, exist_ok=True)

    # Compiler artifacts go to their OWN root when the sources came from
    # --corpus. That tree is the caller's input — prepared by another job or
    # another machine, possibly read-only, possibly shared by several runs — so
    # writing out.spv, .slang-module files and reflect.json into it would
    # mutate somebody else's data and make a second run's inputs depend on the
    # first run's outputs. Everywhere else the two roots are the same directory,
    # which is what keeps the default path's layout (and --gen-dir's, where the
    # point is to inspect sources and outputs together) exactly as it was.
    out_root = src_root
    if args.corpus:
        out_root = (os.path.abspath(args.gen_dir) if args.gen_dir
                    else scratch("perfsuite_out_"))
        os.makedirs(out_root, exist_ok=True)

    # Resolve the api-driver + libslang once when any api workload is selected.
    api = None
    if any(s.mode == "api" for s in specs):
        libslang = os.path.abspath(args.libslang) if args.libslang else find_libslang(slangc)
        driver = os.path.abspath(args.api_driver) if args.api_driver else build_api_driver(out_root)
        # A driver too old to know --out-dir does not reject it — argValue()
        # scans for the flags it knows and ignores the rest — so module-graph-bin
        # would serialize .slang-module binaries beside the sources it read,
        # which under --corpus is the caller's prepared tree. Checked rather
        # than warned about, because the failure mode is writing into somebody
        # else's directory and then reporting success. Only --corpus splits the
        # two roots; everywhere else --out-dir equals --dir and an old driver
        # ignoring it lands in the same place anyway.
        if driver and args.corpus and not api_driver_supports_out_dir(driver):
            sys.exit(f"--api-driver {driver} predates --out-dir, so a corpus run "
                     f"would write .slang-module files into {src_root}; rebuild "
                     f"it from this checkout, or drop --api-driver and let "
                     f"bench.py build it")
        if libslang and driver:
            api = {"driver": driver, "libslang": libslang}
        else:
            sys.stderr.write("compile-perf: api workloads will FAIL "
                             f"(libslang={libslang}, driver={driver})\n")

    records = []
    for spec in specs:
        sizes = spec.sweep_sizes if (args.sweep and spec.sweep_sizes) else [spec.default_size]
        for size in sizes:
            print(f"[run] {spec.name:18s} n={size:<5d} ", end="", flush=True)
            # ANY generator/run failure (missing corpus, a generator bug, a
            # bad manifest field) must cost ONE workload, not the whole run's
            # results: everything measured before it would be lost, since
            # results.json is written at the end. Record the failure and keep
            # going; bench still exits non-zero at the end via the ok-count.
            try:
                rec = run_spec(slangc, spec, size, args.samples, args.warmup,
                               src_root, out_root, api=api,
                               prepared=bool(args.corpus))
            except Exception as e:  # noqa: BLE001 — isolation is the contract
                rec = {
                    "workload": spec.name, "bucket": spec.bucket, "size": size,
                    "mode": spec.mode, "ok": False, "setup_ok": False,
                    "got_timers": False, "samples": args.samples,
                    "warmup": args.warmup, "wall_ms": None, "rss_kb": None,
                    "timers": {}, "primary_timers": spec.primary_timers,
                    "cmd": "", "error": str(e), "crash_codes": None,
                }
            rec["label"] = args.label
            rec["slangc"] = slangc
            records.append(rec)
            ci = rec["timers"].get("compileInner")
            tag = "OK " if rec["ok"] else "FAIL"
            ms = f'{ci["median"]:.2f}ms' if ci else "no-compileInner"
            extra = "" if rec["ok"] else f"  <- {rec['error'] or 'no timers'}"
            print(f"{tag} compileInner={ms}{extra}")

    # JSON (full detail). Merge with any existing file so a partial run
    # (e.g. --only mdl_dxr) augments rather than clobbers prior workloads.
    jpath = os.path.join(root, "results.json")
    this_run = records  # workloads run this invocation (for the summary/exit)
    merged = {}
    if os.path.exists(jpath):
        with open(jpath, encoding="utf-8") as fh:
            for r in json.load(fh):
                merged[(r["workload"], r["size"])] = r
    for r in records:
        merged[(r["workload"], r["size"])] = r
    records = list(merged.values())
    with analyze.open_output(jpath) as fh:
        json.dump(records, fh, indent=2)

    # results.json is the single source of truth (summary AND raw samples per
    # timer); the analysis/report tools read it directly. No CSV is emitted.
    # Only a tree THIS process created is ours to delete, which is exactly what
    # scratch_roots records. A --corpus tree is the caller's prepared input —
    # deleting it would destroy what we were asked to measure — and a --gen-dir
    # tree was passed in precisely so its contents survive for inspection.
    for d in scratch_roots:
        shutil.rmtree(d, ignore_errors=True)

    n_ok = sum(1 for r in this_run if r["ok"])
    print(f"\n{n_ok}/{len(this_run)} runs ok")
    print(f"wrote {jpath}")
    if n_ok != len(this_run):
        sys.exit(1)


# Import-time self-checks (the directory idiom), run by check-python-core.yml
# on every PR touching these files. bench.py had none, which mattered here:
# the samples are stored RAW deliberately, and nothing else in the suite reads
# them yet, so a future edit rounding them would import cleanly, merge, and
# surface only as silently altered measurements — and as a rejected BenchView
# submission, since a summary computed from raw values does not match one
# recomputed from rounded samples.
# Both extrema carry a 5th decimal so that dropping their round() is
# observable. A value that is already exact at 4 places (3.0, say) asserts
# nothing about rounding: it compares equal either way, so the check would
# pass through the very edit it exists to catch.
_s = stats([1.23456, 2.0, 3.98769])
assert _s["samples"] == [1.23456, 2.0, 3.98769], \
    "samples must be stored RAW; rounding them alters what a consumer recomputes"
assert _s["min"] == 1.2346 and _s["max"] == 3.9877, \
    "summary fields ARE rounded, and max is reported alongside min"
assert _s["n"] == 3
assert stats([]) is None, "no measurements yields no stats, not an empty summary"
assert stats([5.0])["stdev"] == 0.0, "a single sample has zero deviation, not None"
# Pins the sample list, not just n: both are built from the same filtered
# list today, so n alone would still hold if a later edit archived the
# unfiltered argument, letting a None reach a consumer that cannot take one.
assert stats([1.0, None, 2.0])["samples"] == [1.0, 2.0], \
    "None samples are dropped from the archive, not merely uncounted"
assert stats([1.0, None, 2.0])["n"] == 2
del _s


# build_commands is pure, and two of its properties fail SILENTLY — as a wrong
# or incomparable number rather than an error — so both are pinned here.
def _check_build_commands():
    """Check the include paths build_commands emits for a link workload."""
    class LinkSpec:
        mode = "link"
        extra_flags = []

    files = ["link_main.slang", "m0.slang"]
    same = build_commands("slangc", LinkSpec, "/src", files, "/src")["timed"]
    assert same.count("-I") == 1, \
        ("on the default path sources and artifacts share a directory, and the "
         "single -I emitted there is part of the cmd string recorded in "
         "results.json; a second one makes new results incomparable with every "
         f"point already in the series: {same}")

    split = build_commands("slangc", LinkSpec, "/src", files, "/out")
    timed = split["timed"]
    assert timed.count("-I") == 2 and timed.index("/out") < timed.index("/src"), \
        ("split roots must put out_dir on the include path FIRST: the "
         "precompiled .slang-module lives there while its .slang source lives "
         "in src_dir, so the reverse order resolves the import to the source "
         "and measures a recompile — succeeding all the while, which is the "
         f"whole danger: {timed}")
    assert timed[-1].startswith("/out"), "the -o output must land under out_dir"
    assert all(c[-1].startswith("/out") for c in split["setup"]), \
        "precompiled modules must land under out_dir, never in a --corpus tree"


_check_build_commands()
del _check_build_commands


if __name__ == "__main__":
    main()
