#!/usr/bin/env python3
"""Slang compile-time perf-suite runner.

Drives a given slangc over the workloads in manifest.py, parses the per-phase
timers emitted by -report-detailed-perf-benchmark, and writes tidy JSON + CSV.

Stdlib only (no prettytable / numpy) so it runs unchanged against any release's
slangc during the Phase 3 sweep.

Examples:
    # Run the whole suite at default sizes with the local build:
    python3 bench.py --slangc ../../../build/RelWithDebInfo/bin/slangc --label dev

    # One workload, scaling-curve sizes, more samples:
    python3 bench.py --slangc /path/slangc --label v2026.9 \\
        --only autodiff --sweep --samples 7
"""
import argparse
import json
import os
import shutil
import statistics
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import manifest  # noqa: E402


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
                    pass
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


def build_commands(slangc, spec, gen_dir, files):
    """Return (commands, primary_outfile_for_parsing_index).

    For "link" mode the timed command is the final main compile (last element);
    module precompiles are setup and run once (not timed)."""
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
    # target mode
    f = spec.main_file or main or list(files)[0]
    out = os.path.join(gen_dir, "out.spv")
    # -I gen_dir lets multi-file corpora resolve imports; harmless for single files
    return {
        "setup": [],
        "timed": [slangc, PERF_FLAG, "-I", gen_dir, os.path.join(gen_dir, f),
                  *spec.extra_flags, "-o", out],
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
# Matches both the modern "error[E30015]:" and the legacy "error 30015:" formats.
_ERR_RE = __import__("re").compile(r"error\[|: error:|\berror \d+:")


def real_error(text):
    """A genuine compile error in either the modern or legacy slangc format,
    ignoring benign missing-downstream-tool diagnostics."""
    for line in text.splitlines():
        if _ERR_RE.search(line) and not any(b in line for b in _BENIGN):
            return line.strip()
    return None


def run_spec(slangc, spec, size, samples, warmup, root):
    gen_dir = os.path.join(root, "gen", spec.name + f"_n{size}")
    if os.path.exists(gen_dir):
        shutil.rmtree(gen_dir)
    os.makedirs(gen_dir, exist_ok=True)
    files = spec.gen(size)
    for fn, src in files.items():
        with open(os.path.join(gen_dir, fn), "w") as fh:
            fh.write(src)

    cmds = build_commands(slangc, spec, gen_dir, files)
    for c in cmds["setup"]:
        subprocess.run(c, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    timed = cmds["timed"]
    for _ in range(warmup):
        run_once(timed)

    per_timer = {}
    walls = []
    rsses = []
    last_text = ""
    rc = 0
    for _ in range(samples):
        rc, wall, text, rss = run_once(timed)
        last_text = text
        walls.append(wall)
        if rss is not None:
            rsses.append(rss)
        for name, ms in parse_timers(text).items():
            per_timer.setdefault(name, []).append(ms)

    err = real_error(last_text)
    if spec.expect_fail:
        ok = err is not None  # we *want* compile errors
    else:
        ok = err is None
    got_timers = bool(per_timer)

    return {
        "workload": spec.name,
        "bucket": spec.bucket,
        "size": size,
        "mode": spec.mode,
        "expected_fail": spec.expect_fail,
        "ok": ok and got_timers,
        "got_timers": got_timers,
        "samples": samples,
        "warmup": warmup,
        "wall_ms": stats(walls),
        "rss_kb": stats(rsses) if rsses else None,
        "timers": {k: stats(v) for k, v in sorted(per_timer.items())},
        "primary_timers": spec.primary_timers,
        "cmd": " ".join(timed),
        "error": err,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--slangc", required=True, help="path to slangc to benchmark")
    ap.add_argument("--label", required=True, help="version/run label, e.g. v2026.9")
    ap.add_argument("--out", default="results", help="output directory")
    ap.add_argument("--samples", type=int, default=5)
    ap.add_argument("--warmup", type=int, default=1)
    ap.add_argument("--only", default=None,
                    help="comma-separated workload names to run (default all)")
    ap.add_argument("--sweep", action="store_true",
                    help="run each workload's sweep_sizes instead of default_size")
    args = ap.parse_args()

    slangc = os.path.abspath(args.slangc)
    if not os.path.exists(slangc):
        sys.exit(f"slangc not found: {slangc}")

    specs = manifest.WORKLOADS
    if args.only:
        want = set(args.only.split(","))
        specs = [s for s in specs if s.name in want]
        missing = want - {s.name for s in specs}
        if missing:
            sys.exit(f"unknown workloads: {sorted(missing)}")

    root = os.path.join(os.path.abspath(args.out), args.label)
    os.makedirs(root, exist_ok=True)

    records = []
    for spec in specs:
        sizes = spec.sweep_sizes if (args.sweep and spec.sweep_sizes) else [spec.default_size]
        for size in sizes:
            print(f"[run] {spec.name:18s} n={size:<5d} ", end="", flush=True)
            rec = run_spec(slangc, spec, size, args.samples, args.warmup, root)
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
        for r in json.load(open(jpath)):
            merged[(r["workload"], r["size"])] = r
    for r in records:
        merged[(r["workload"], r["size"])] = r
    records = list(merged.values())
    with open(jpath, "w") as fh:
        json.dump(records, fh, indent=2)

    # CSV (long format: one row per timer)
    cpath = os.path.join(root, "results.csv")
    with open(cpath, "w") as fh:
        fh.write("label,workload,bucket,size,mode,timer,median_ms,min_ms,stdev_ms,n,ok,primary\n")
        for r in records:
            for tname, st in r["timers"].items():
                if st is None:
                    continue
                primary = 1 if tname in r["primary_timers"] else 0
                fh.write(
                    f'{r["label"]},{r["workload"]},{r["bucket"]},{r["size"]},{r["mode"]},'
                    f'{tname},{st["median"]},{st["min"]},{st["stdev"]},{st["n"]},'
                    f'{int(r["ok"])},{primary}\n'
                )

    n_ok = sum(1 for r in this_run if r["ok"])
    print(f"\n{n_ok}/{len(this_run)} runs ok")
    print(f"wrote {jpath}")
    print(f"wrote {cpath}")
    if n_ok != len(this_run):
        sys.exit(1)


if __name__ == "__main__":
    main()
