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
import ctypes  # for the Win32 peak-RSS struct; import is safe on every platform
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

from lib import analyze, manifest


def parse_mem(text):
    """Extract {name: kb} from the api-driver's "[MEM] name\tNNNkb" lines —
    point-in-time RSS deltas recorded around selected API phases (see
    native/api-driver.cpp reportMemDeltas, the producing printf), kept
    separate from the ms timers so nothing downstream mistakes kilobytes for
    milliseconds. Fields are TAB-delimited, matching the producer exactly.
    Counter names must end in "Kb" — that suffix is what analyze.unit_of
    keys the kb-vs-ms display classification on."""
    out = {}
    for line in text.splitlines():
        line = line.strip()
        if not line.startswith("[MEM]"):
            continue
        toks = line[len("[MEM]"):].strip().split("\t")
        # Two DIFFERENT suffix contracts meet here, and the case matters:
        # the VALUE token ends in lowercase "kb" (the api-driver's %.0fkb
        # print format), while the counter NAME must end in capitalized
        # "Kb" (analyze.unit_of's display-classification convention).
        if len(toks) == 2 and toks[1].endswith("kb"):
            try:
                val = float(toks[1][:-2])
            except ValueError:
                continue
            assert toks[0].endswith("Kb"), \
                f"memory counter '{toks[0]}' must end in Kb (analyze.unit_of contract)"
            out[toks[0]] = val
    return out


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
        timed += spec.api_flags
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
    out = os.path.join(gen_dir, "out." + _target_ext(spec.extra_flags))
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


# Peak RSS collection: per-child ru_maxrss via os.wait4 on POSIX,
# PeakWorkingSetSize via GetProcessMemoryInfo on Windows (below).


class PROCESS_MEMORY_COUNTERS(ctypes.Structure):
    """The Win32 PROCESS_MEMORY_COUNTERS struct that GetProcessMemoryInfo
    fills in. Defined at module scope, and with FIXED-WIDTH field types, so
    its binary layout can be pinned by a self-check on any platform.

    The DWORD fields are ctypes.c_uint32 rather than ctypes.wintypes.DWORD
    deliberately. On Windows the two are identical (DWORD is c_ulong, which
    is 4 bytes under LLP64), but ctypes.wintypes is importable on POSIX too,
    where c_ulong is 8 bytes — so a wintypes.DWORD version of this struct
    silently has a DIFFERENT layout off Windows and cannot be checked
    anywhere but the platform it is used on. Spelling the width explicitly
    makes the layout identical everywhere, which is what lets the assertion
    at the bottom of this module catch a mis-ordered or mis-typed field.
    A wrong layout would not raise: it would read PeakWorkingSetSize from
    the wrong offset and return a believable but incorrect number."""

    _fields_ = [("cb", ctypes.c_uint32),
                ("PageFaultCount", ctypes.c_uint32),
                ("PeakWorkingSetSize", ctypes.c_size_t),
                ("WorkingSetSize", ctypes.c_size_t),
                ("QuotaPeakPagedPoolUsage", ctypes.c_size_t),
                ("QuotaPagedPoolUsage", ctypes.c_size_t),
                ("QuotaPeakNonPagedPoolUsage", ctypes.c_size_t),
                ("QuotaNonPagedPoolUsage", ctypes.c_size_t),
                ("PagefileUsage", ctypes.c_size_t),
                ("PeakPagefileUsage", ctypes.c_size_t)]


def _windows_peak_rss_kb(popen):
    """PeakWorkingSetSize of a finished subprocess in KB via
    GetProcessMemoryInfo — the Windows equivalent of POSIX ru_maxrss. Reads
    through the still-open Popen handle, so it must run before the Popen is
    garbage-collected. Depends on the CPython-internal `popen._handle`
    attribute (no public accessor exists); if a CPython release renames it,
    the AttributeError lands in the except below and Windows memory
    collection degrades to None — check here first if rss_kb goes null on
    the runner after a Python upgrade. Returns None if the query fails."""
    try:
        # wintypes stays function-local: it is only needed for the call
        # signature, and only Windows ever reaches here. The struct itself is
        # at module scope so its layout can be self-checked (see above).
        from ctypes import wintypes

        pmc = PROCESS_MEMORY_COUNTERS()
        pmc.cb = ctypes.sizeof(PROCESS_MEMORY_COUNTERS)
        psapi = ctypes.WinDLL("psapi")
        fn = psapi.GetProcessMemoryInfo
        # Declare the signature: without argtypes ctypes coerces the handle
        # through a C int, which can truncate 64-bit HANDLE values.
        fn.argtypes = [wintypes.HANDLE,
                       ctypes.POINTER(PROCESS_MEMORY_COUNTERS), wintypes.DWORD]
        fn.restype = wintypes.BOOL
        if fn(wintypes.HANDLE(popen._handle), ctypes.byref(pmc), pmc.cb):
            return pmc.PeakWorkingSetSize / 1024.0
    except Exception:  # noqa: BLE001
        pass
    return None


def _maxrss_to_kb(ru_maxrss, platform):
    """Convert a getrusage ru_maxrss field to kilobytes for the given
    sys.platform string. The field's UNIT is platform-specific: kilobytes on
    Linux, but BYTES on macOS, which inherited the BSD definition. Isolated
    into its own function because getting it wrong scales every memory number
    on one platform by 1024 — which renders as a perfectly plausible chart
    rather than an error — so the rule is pinned by a self-check below."""
    return ru_maxrss / (1024.0 if platform == "darwin" else 1.0)


def _reap_posix(proc, wait4=None):
    """Reap a finished child with os.wait4, set proc.returncode from its wait
    status, and return its peak RSS in KB.

    wait4 rather than Popen.wait so ru_maxrss is per-child: a
    getrusage(RUSAGE_CHILDREN) high-water mark would smear one workload's
    peak onto every later one in the same sweep.

    `wait4` is a parameter so the status decode and unit conversion can be
    exercised against a stub instead of a live process — this is the sole
    source of BOTH the return code and the memory number on Linux, so a
    regression here would null out every rss series at once.

    It defaults to None and resolves os.wait4 in the BODY rather than in the
    signature, because a default-argument expression is evaluated once when
    the def statement runs at import. os.wait4 is POSIX-only, so binding it
    in the signature would raise AttributeError while merely importing this
    module on Windows — a platform that never calls this function, and whose
    own path (_windows_peak_rss_kb) is right below. Do not "simplify" this
    back into the signature.

    Raises RuntimeError if the child cannot be reaped, rather than returning a
    number it cannot stand behind. main() catches that per workload (its
    try/except around run_spec is the isolation contract), so the workload is
    recorded with ok=False and the sweep continues to the next one — the run
    still ends non-zero through the ok-count. The translation lives HERE
    rather than in the caller so the stub-driven self-check can reach it
    without spawning a process — see the ECHILD case at the bottom of this
    module."""
    if wait4 is None:
        wait4 = os.wait4
    try:
        _pid, status, ru = wait4(proc.pid, 0)
    except ChildProcessError as e:
        # Nothing in this tool reaps the child, so ECHILD means the
        # ENVIRONMENT auto-reaped it — SIGCHLD set to SIG_IGN — and the exit
        # status is gone. Falling back to Popen.wait() does not recover it
        # and is actively harmful: CPython's Popen._try_wait catches
        # ChildProcessError and substitutes returncode = 0, so a compile that
        # FAILED would be recorded as a clean run with no memory number.
        # Raising is what keeps a fabricated success out of results.json;
        # main() then books this workload as failed and moves on.
        raise RuntimeError(
            "os.wait4 could not reap the compile child (ECHILD): the "
            "environment reaped it first, which happens when SIGCHLD is set "
            "to SIG_IGN. Its exit status and peak RSS are unrecoverable, and "
            "Popen.wait() would report success for a failed compile, so this "
            "workload is recorded as failed rather than measured. If every "
            "workload fails this way, re-run with default SIGCHLD handling."
        ) from e
    proc.returncode = os.waitstatus_to_exitcode(status)
    return _maxrss_to_kb(ru.ru_maxrss, sys.platform)


def run_once(cmd):
    """Run one compile; return (rc, wall_ms, combined_text, rss_kb_or_None).

    rss is the child's peak resident set (peak working set on Windows) in KB.
    Platform constants differ slightly (working set vs RSS), but the tracked
    series compares within one runner fingerprint, never across platforms.

    Raises RuntimeError on POSIX if the child cannot be reaped (see
    _reap_posix): the exit code is unrecoverable there, so it propagates
    instead of returning a tuple whose rc would be a guess. main()'s
    per-workload try/except turns that into one failed workload."""
    t0 = time.perf_counter()
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
    out = proc.stdout.read()
    rss = None
    if os.name == "nt":
        proc.wait()
        rss = _windows_peak_rss_kb(proc)
    else:
        # No try/except: _reap_posix already translates ECHILD into a
        # RuntimeError explaining why no trustworthy rc exists, and letting it
        # reach main()'s per-workload handler is the point.
        rss = _reap_posix(proc)
    wall = (time.perf_counter() - t0) * 1000.0
    text = out.decode("utf-8", "replace")
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


def run_spec(slangc, spec, size, samples, warmup, gen_root, api=None):
    gen_dir = os.path.join(gen_root, spec.name + f"_n{size}")
    if os.path.exists(gen_dir):
        shutil.rmtree(gen_dir)
    os.makedirs(gen_dir, exist_ok=True)
    files = spec.gen(size)
    for fn, src in files.items():
        # Fail-loud guard for the byte-determinism invariant (see _HEADER in
        # workloads.py): a typographic character anywhere in a GENERATED
        # source would silently make the corpus bytes platform-dependent.
        # External corpora are exempt — they are third-party input read with
        # a tolerant decode, not something our generators promise about.
        # A raise, not an assert: the contract must hold under python -O too.
        if not spec.external_corpus and not src.isascii():
            raise ValueError(
                f"generated source {fn} contains non-ASCII; generators must "
                f"emit ASCII only so the corpus is byte-identical everywhere")
        with analyze.open_output(os.path.join(gen_dir, fn)) as fh:
            fh.write(src)

    # An api workload without a driver+libslang must fail loudly (not silently
    # skip): a missing host compiler or unrecognized package layout would
    # otherwise drop the workload from the series with no visible signal.
    if spec.mode == "api" and api is None:
        return {
            "workload": spec.name, "bucket": spec.bucket, "size": size,
            "mode": spec.mode, "ok": False, "setup_ok": False,
            "got_timers": False, "samples": samples, "warmup": warmup,
            "wall_ms": None, "rss_kb": None, "memory": None, "timers": {},
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

    benign = (_BENIGN_DOWNSTREAM_REQUIRED
              if getattr(spec, "downstream_required", False) else _BENIGN)

    timed = cmds["timed"]
    for _ in range(warmup):
        run_once(timed)

    per_timer = {}
    per_mem = {}
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
        for name, kb in parse_mem(text).items():
            per_mem.setdefault(name, []).append(kb)

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
        "memory": ({k: stats(v) for k, v in sorted(per_mem.items())}
                   if per_mem else None),
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
    ap.add_argument("--sweep", action="store_true",
                    help="run each workload's sweep_sizes instead of default_size")
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
        sizes = spec.sweep_sizes if (args.sweep and spec.sweep_sizes) else [spec.default_size]
        for size in sizes:
            print(f"[run] {spec.name:18s} n={size:<5d} ", end="", flush=True)
            # ANY generator/run failure (missing corpus, a generator bug, a
            # bad manifest field) must cost ONE workload, not the whole run's
            # results: everything measured before it would be lost, since
            # results.json is written at the end. Record the failure and keep
            # going; bench still exits non-zero at the end via the ok-count.
            try:
                rec = run_spec(slangc, spec, size, args.samples, args.warmup, gen_root,
                               api=api)
            except Exception as e:  # noqa: BLE001 — isolation is the contract
                rec = {
                    "workload": spec.name, "bucket": spec.bucket, "size": size,
                    "mode": spec.mode, "ok": False, "setup_ok": False,
                    "got_timers": False, "samples": args.samples,
                    "warmup": args.warmup, "wall_ms": None, "rss_kb": None,
                    "memory": None, "timers": {}, "primary_timers": spec.primary_timers,
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

    # results.json is the single source of truth (all of median/min/mean/stdev per
    # timer); the analysis/report tools read it directly. No CSV is emitted.
    if not args.gen_dir:
        shutil.rmtree(gen_root, ignore_errors=True)

    n_ok = sum(1 for r in this_run if r["ok"])
    print(f"\n{n_ok}/{len(this_run)} runs ok")
    print(f"wrote {jpath}")
    if n_ok != len(this_run):
        sys.exit(1)


# Import-time self-check pinning the [MEM] line contract to api-driver.cpp's
# printf format (TAB-delimited, kb suffix): a format drift would otherwise
# silently degrade the memory feature to "no data" with nothing failing.
assert parse_mem("[MEM] apiCreateGlobalSessionRssDeltaKb\t20480kb\nnoise\n[*] x\t1\t2ms") == \
    {"apiCreateGlobalSessionRssDeltaKb": 20480.0}, \
    "parse_mem: [MEM] line contract drifted vs api-driver.cpp"
assert parse_mem("[MEM] malformed") == {}, "parse_mem must ignore malformed lines"


# Import-time self-check for the ru_maxrss unit rule. Pure and platform-free
# (the platform is a parameter), so both branches are exercised on every host
# rather than only the one running the check.
assert _maxrss_to_kb(2048, "linux") == 2048.0, "Linux ru_maxrss is already KB"
assert _maxrss_to_kb(2048 * 1024, "darwin") == 2048.0, "macOS ru_maxrss is BYTES"


# Import-time guard against re-introducing a POSIX-only default argument.
# os.wait4 does not exist on Windows, and a default-argument expression is
# evaluated when the def statement runs — so `def _reap_posix(proc,
# wait4=os.wait4)` raises AttributeError while merely IMPORTING this module
# on Windows, before any os.name branch can protect it. This check runs on
# every platform because the POSIX hosts are the ones that would not notice.
assert _reap_posix.__defaults__ == (None,), \
    "_reap_posix must resolve os.wait4 in its body, not bind it as a default"


# Import-time self-check for the Win32 struct layout. Runs on every platform
# — which is the point: the fields are fixed-width (see the class docstring),
# so the layout here is the layout Windows sees, and a mis-ordered or
# mis-typed field is caught on the Linux CI rather than by a believable wrong
# number on the Windows runner. The expected values are the x64 ABI's:
# two 4-byte DWORDs, then eight 8-byte SIZE_T fields at offset 8.
assert ctypes.sizeof(ctypes.c_size_t) == 8, \
    "these offsets assume a 64-bit host; revisit if a 32-bit runner appears"
assert ctypes.sizeof(PROCESS_MEMORY_COUNTERS) == 72, \
    "PROCESS_MEMORY_COUNTERS layout drifted from the Win32 x64 definition"
assert PROCESS_MEMORY_COUNTERS.PeakWorkingSetSize.offset == 8, \
    "PeakWorkingSetSize must follow the two DWORDs; a wrong offset reads garbage"
assert PROCESS_MEMORY_COUNTERS.cb.offset == 0, "cb must be the first field"


# Import-time self-check for the POSIX reaping path, driven by a stub wait4 so
# no process is spawned at import. This path is the only source of BOTH the
# return code and the peak RSS on Linux, and its failure mode is silent
# (rss_kb: null across every workload, or a fabricated returncode), so the
# status decode and the wait4 call convention are pinned here.
if os.name != "nt":  # the function, and the wait-status encoding, are POSIX-only
    class _StubProc:
        pid = 4321
        returncode = None

    class _StubRusage:
        # KB on Linux, bytes on macOS — matched to the host so the expected
        # value below is the same 2 MiB either way.
        ru_maxrss = 2048 * (1024 if sys.platform == "darwin" else 1)

    _seen = []

    def _stub_wait4(pid, flags):
        _seen.append((pid, flags))
        return pid, 0x100, _StubRusage()  # wait status for "exited with code 1"

    _p = _StubProc()
    assert _reap_posix(_p, wait4=_stub_wait4) == 2048.0, \
        "_reap_posix: ru_maxrss must reach the caller as KB"
    assert _p.returncode == 1, \
        "_reap_posix: returncode must come from the wait status, not Popen"
    assert _seen == [(4321, 0)], \
        "_reap_posix: wait4 must be called blocking on the child's own pid"

    # The ECHILD branch — the highest-stakes path here, because the fallback
    # it refuses (Popen.wait) would record a FAILED compile as a clean run.
    # Reachable from this stub only because the translation lives inside
    # _reap_posix rather than in run_once, which cannot be driven without
    # spawning a process.
    def _stub_wait4_echild(pid, flags):
        raise ChildProcessError(10, "No child processes")

    _p2 = _StubProc()
    try:
        _reap_posix(_p2, wait4=_stub_wait4_echild)
        raise AssertionError("_reap_posix must not swallow ECHILD")
    except RuntimeError as _e:
        assert isinstance(_e.__cause__, ChildProcessError), \
            "_reap_posix must chain the original ECHILD as __cause__"
        assert "SIGCHLD" in str(_e), \
            "_reap_posix: the ECHILD message must name the cause to look for"
    assert _p2.returncode is None, \
        "_reap_posix must NOT leave a fabricated returncode after ECHILD"
    # `_e` needs no del: Python unbinds an `except ... as` name at block exit.
    del _StubProc, _StubRusage, _seen, _stub_wait4, _stub_wait4_echild, _p, _p2


# Import-time self-check for the Windows collector's degradation path, which
# runs on every host: given an object without the CPython-internal `_handle`
# the function must return None rather than raise, because run_once treats a
# None rss as "no memory number this sample" and a raise would sink the sweep.
# This is the failure the docstring warns about — a CPython release renaming
# `_handle` — and on POSIX the missing WinDLL takes the same except path.
class _NoHandle:
    pass


assert _windows_peak_rss_kb(_NoHandle()) is None, \
    "_windows_peak_rss_kb must degrade to None, not raise, when the query fails"
del _NoHandle


if __name__ == "__main__":
    main()
