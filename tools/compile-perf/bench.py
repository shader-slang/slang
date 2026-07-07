#!/usr/bin/env python3
"""Slang compile-time perf-suite runner.

Drives a given slangc over the workloads in manifest.py, parses the per-phase
timers emitted by -report-perf-benchmark, and writes tidy per-run JSON
(median/min/mean/stdev per timer; merge-on-write).

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

from lib import manifest


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
    values = [v for v in values if v is not None]
    if not values:
        return None
    return {
        "median": round(statistics.median(values), 4),
        "min": round(min(values), 4),
        "mean": round(statistics.mean(values), 4),
        "stdev": round(statistics.stdev(values), 4) if len(values) > 1 else 0.0,
        "n": len(values),
    }


# Base flag (not -report-detailed-perf-benchmark): the base flag already emits
# every phase timer the suite uses and is supported across the whole release
# window; the detailed flag was added mid-window and only adds finer sub-timers.
PERF_FLAG = "-report-perf-benchmark"

HERE = os.path.dirname(os.path.abspath(__file__))


def find_libslang(slangc):
    """Locate the slang shared library belonging to a slangc binary, trying the
    layouts of release packages (bin/ + ../lib/) and build trees (same dir).
    Returns None when not found — api workloads then fail with a clear error
    while slangc workloads run normally."""
    d = os.path.dirname(slangc)
    for cand in (
        os.path.join(d, "slang.dll"),
        os.path.join(d, "libslang.dylib"),
        os.path.join(d, "libslang.so"),
        os.path.join(d, "..", "lib", "libslang.dylib"),
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
        cmd = ["cl", "-nologo", "-O2", "-std:c++17", "-EHsc", f"-I{inc}", src,
               f"-Fe:{out}"]
    else:
        cmd = ["c++", "-O2", "-std=c++17", "-I", inc, src, "-o", out]
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


def build_commands(slangc, spec, gen_dir, files, size=None, api=None):
    """Return (commands, primary_outfile_for_parsing_index).

    For "link" mode the timed command is the final main compile (last element);
    module precompiles are setup and run once (not timed).
    For "api" mode the timed command is the api-driver (api = {"driver", "libslang"});
    the driver emits [*] timer lines in the slangc report format."""
    if spec.mode == "api":
        timed = [api["driver"], api["libslang"], spec.api_cmd]
        if spec.api_cmd == "session-create":
            timed += ["--iters", str(size)]
        else:
            timed += ["--dir", gen_dir]
        if spec.api_root:
            timed += ["--root", spec.api_root]
        return {"setup": [], "timed": timed}
    main = next((f for f in files if "main" in f), None)
    if spec.mode == "module":
        f = list(files)[0]
        out = os.path.join(gen_dir, "out.slang-module")
        return {
            "setup": [],
            "timed": [slangc, PERF_FLAG, os.path.join(gen_dir, f),
                      *spec.extra_flags, "-o", out],
        }
    if spec.mode == "link":
        setup = []
        for f in files:
            if f == main:
                continue
            setup.append([slangc, os.path.join(gen_dir, f), "-o",
                          os.path.join(gen_dir, f.replace(".slang", ".slang-module"))])
        out = os.path.join(gen_dir, "out.spv")
        timed = [slangc, PERF_FLAG, "-I", gen_dir,
                 os.path.join(gen_dir, main), *spec.extra_flags, "-o", out]
        return {"setup": setup, "timed": timed}
    # "target" mode: single or multi-file compile to a GPU target. For single-file
    # workloads spec.main_file (or the first file) is the entry point. For corpus
    # workloads (e.g. mdl_dxr), spec.main_file names the root; -I gen_dir lets
    # sibling imports resolve without explicit paths. reflection_json attaches a
    # per-run output path so the layout/reflection serializer is exercised without
    # polluting the results directory.
    f = spec.main_file or main or list(files)[0]
    out = os.path.join(gen_dir, "out.spv")
    extra = list(spec.extra_flags)
    # reflection JSON needs a writable path; gen_dir is per-run and writable.
    if getattr(spec, "reflection_json", False):
        extra += ["-reflection-json", os.path.join(gen_dir, "reflect.json")]
    # -I gen_dir lets multi-file corpora resolve imports; harmless for single files
    return {
        "setup": [],
        "timed": [slangc, PERF_FLAG, "-I", gen_dir, os.path.join(gen_dir, f),
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
            with open(memfile) as fh:
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
# Matches the modern "error[E30015]:" and legacy "error 30015:" slangc formats,
# plus the api-driver's bare "error: ..." lines.
_ERR_RE = re.compile(r"error\[|: error:|\berror \d+:|^error: ")


def real_error(text):
    """A genuine compile error in either the modern or legacy slangc format,
    ignoring benign missing-downstream-tool diagnostics."""
    for line in text.splitlines():
        if _ERR_RE.search(line) and not any(b in line for b in _BENIGN):
            return line.strip()
    return None


def run_spec(slangc, spec, size, samples, warmup, gen_root, api=None):
    gen_dir = os.path.join(gen_root, spec.name + f"_n{size}")
    if os.path.exists(gen_dir):
        shutil.rmtree(gen_dir)
    os.makedirs(gen_dir, exist_ok=True)
    files = spec.gen(size)
    for fn, src in files.items():
        with open(os.path.join(gen_dir, fn), "w") as fh:
            fh.write(src)

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

    cmds = build_commands(slangc, spec, gen_dir, files, size=size, api=api)
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
        err = real_error(text)
        sample_ok.append(err is None)  # ok when no compile error
        for name, ms in parse_timers(text).items():
            per_timer.setdefault(name, []).append(ms)

    err = real_error(last_text)
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
    ap.add_argument("--slangc", required=True, help="path to slangc to benchmark")
    ap.add_argument("--label", required=True, help="version/run label, e.g. v2026.9")
    ap.add_argument("--out", default="results", help="output directory")
    ap.add_argument("--samples", type=int, default=5)
    ap.add_argument("--warmup", type=int, default=1)
    ap.add_argument("--only", default=None,
                    help="comma-separated workload names to run (default all)")
    ap.add_argument("--gen-dir", default=None,
                    help="scratch dir for generated sources + compiled outputs "
                         "(default: a tempdir, auto-removed — keeps the results dir, which "
                         "is committed to the perf-results repo, free of build scratch). "
                         "Pass a path to keep them for inspection.")
    ap.add_argument("--api-driver", default=None,
                    help="prebuilt api-driver binary (default: build it from "
                         "native/api-driver.cpp with the host compiler)")
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

    slangc = os.path.abspath(args.slangc)
    if not os.path.exists(slangc):
        sys.exit(f"slangc not found: {slangc}")

    specs = manifest.WORKLOADS
    if not args.api and not args.only:
        specs = [s for s in specs if s.mode != "api"]
    if args.only:
        want = set(args.only.split(","))
        specs = [s for s in specs if s.name in want]
        missing = want - {s.name for s in specs}
        if missing:
            sys.exit(f"unknown workloads: {sorted(missing)}")

    root = os.path.join(os.path.abspath(args.out), args.label)
    os.makedirs(root, exist_ok=True)
    # Generated sources + compiled outputs are large, transient build scratch; keep
    # them OUT of the results dir so it stores only results.json. Default to a
    # tempdir that is removed at the end (overridable with --gen-dir to keep them).
    gen_root = os.path.abspath(args.gen_dir) if args.gen_dir else tempfile.mkdtemp(prefix="perfsuite_gen_")
    os.makedirs(gen_root, exist_ok=True)

    # Resolve the api-driver + libslang once when any api workload is selected.
    api = None
    if any(s.mode == "api" for s in specs):
        libslang = os.path.abspath(args.libslang) if args.libslang else find_libslang(slangc)
        driver = os.path.abspath(args.api_driver) if args.api_driver else build_api_driver(gen_root)
        if libslang and driver:
            api = {"driver": driver, "libslang": libslang}
        else:
            sys.stderr.write("compile-perf: api workloads will FAIL "
                             f"(libslang={libslang}, driver={driver})\n")

    records = []
    for spec in specs:
        sizes = [spec.default_size]
        for size in sizes:
            print(f"[run] {spec.name:18s} n={size:<5d} ", end="", flush=True)
            rec = run_spec(slangc, spec, size, args.samples, args.warmup, gen_root, api=api)
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
        with open(jpath) as fh:
            for r in json.load(fh):
                merged[(r["workload"], r["size"])] = r
    for r in records:
        merged[(r["workload"], r["size"])] = r
    records = list(merged.values())
    with open(jpath, "w") as fh:
        json.dump(records, fh, indent=2)

    # results.json is the single source of truth (all of median/min/mean/stdev per
    # timer); the analysis/report tools read it directly. No CSV is emitted.
    if not args.gen_dir:
        shutil.rmtree(gen_root, ignore_errors=True)

    n_ok = sum(1 for r in this_run if r["ok"])
    print(f"\n{n_ok}/{len(this_run)} runs ok")
    print(f"wrote {jpath}")
    if n_ok != len(this_run):
        sys.exit(1)


if __name__ == "__main__":
    main()
