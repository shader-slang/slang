#!/usr/bin/env python3
"""Drive bench.py across every cached release.

Reads releases/index.json (produced by fetch_releases.py) and runs the perf
suite against each release's slangc, writing results/<tag>/. Idempotent: skips a
release that already has results unless --force.

Typical:
    python3 fetch_releases.py            # populate releases/
    python3 sweep.py --samples 5         # run suite on every release
    python3 analyze.py                   # find regressions
"""
import argparse
import json
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)  # allow running from any directory

from lib import analyze, manifest


def workload_complete(measured_sizes, spec, sweep_requested):
    """Whether a workload's successful measurements satisfy this sweep.

    The current manifest default is always required. When scaling data was
    requested, every current ladder size is required as well. `measured_sizes`
    deliberately retains the JSON value types: a string such as "128" is not
    evidence that the integer size 128 was measured.
    """
    if not measured_sizes:
        return False
    if spec and spec.default_size not in measured_sizes:
        return False
    if sweep_requested and spec and spec.sweep_sizes:
        return set(spec.sweep_sizes) <= measured_sizes
    return True


# Pin the completeness predicate that decides whether a release is re-swept.
_SPEC = manifest.BY_NAME["interface_depth"]
assert workload_complete({_SPEC.default_size}, _SPEC, False)
assert not workload_complete({64}, _SPEC, False), \
    "a stale pre-resize default must trigger a re-sweep"
assert not workload_complete({str(_SPEC.default_size)}, _SPEC, False), \
    "size values with the wrong JSON type must not compare equal"
assert workload_complete(set(_SPEC.sweep_sizes), _SPEC, True)
assert not workload_complete(set(_SPEC.sweep_sizes[:-1]), _SPEC, True), \
    "a missing ladder rung must trigger a scaling re-sweep"
del _SPEC


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument("--index", default=os.path.join(HERE, "releases", "index.json"))
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--samples", type=int, default=5)
    ap.add_argument("--warmup", type=int, default=1)
    ap.add_argument("--only", default=None, help="comma-separated workloads")
    ap.add_argument("--sweep", action="store_true", help="pass --sweep (scaling sizes)")
    ap.add_argument("--api", action="store_true",
                    help="include the api-path workloads (the driver dlopens each "
                         "release's libslang, so history backfill works)")
    ap.add_argument("--force", action="store_true", help="re-run releases already done")
    args = ap.parse_args()

    if not os.path.exists(args.index):
        sys.exit(f"missing {args.index}; run fetch_releases.py first")
    with open(args.index) as fh:
        index = json.load(fh)
    ready = [r for r in index if "slangc" in r]
    print(f"{len(ready)} releases to sweep "
          f"({args.samples} samples + {args.warmup} warmup each)\n")

    want = set(args.only.split(",")) if args.only else None
    # Match bench.py's default-set platform gate: platform-bound workloads
    # (codegen_dxil/ptx need dxc/nvrtc) never run here off-platform, so counting
    # them as "needed" would make need <= present false forever and re-sweep
    # every release on such hosts.
    # ... and its api gate: without --api, bench.py excludes mode="api"
    # workloads from the default set, so counting them as "needed" here would
    # mark every release permanently incomplete and re-bench it on every run.
    # downstream_required workloads are excluded outright: release packages do
    # not bundle the downstream compilers (dxcompiler.dll, nvrtc), so those
    # workloads can never succeed against prebuilt release binaries — their
    # history lives in the daily (tip-of-tree) series only.
    all_wls = {w.name for w in manifest.WORKLOADS
               if (not w.platforms or sys.platform in w.platforms)
               and (args.api or w.mode != "api")
               and not w.downstream_required}
    failures = []
    for i, rec in enumerate(ready, 1):
        tag = rec["tag"]
        done = analyze.results_path(args.results, tag)
        if os.path.exists(done) and not args.force:
            # Skip only if the requested workloads are already present AND — when
            # --sweep is requested — already have multi-size scaling data; a plain
            # (non-swept) results.json from a prior run must not mask a pending
            # sweep. Tolerate an unreadable/partial file by re-running.
            try:
                prev = analyze.read_json(done)
            except (json.JSONDecodeError, OSError) as e:
                prev = []
                print(f"  (note: unreadable {done} ({e}); re-running)")
            sizes = {}
            for r in prev if isinstance(prev, list) else []:
                # A record counts toward completeness only if it MEASURED:
                # bench's failure isolation writes ok=False records for sizes
                # whose generator/run failed, and counting those as present
                # would leave a permanent hole in the scaling curve that only
                # --force could refill. (ok defaults True for legacy records
                # predating the field, so old baselines are not re-swept.)
                if isinstance(r, dict) and "workload" in r and r.get("ok", True):
                    sizes.setdefault(r["workload"], set()).add(r.get("size"))
            need = want or all_wls
            if all(workload_complete(sizes.get(wl), manifest.BY_NAME.get(wl), args.sweep)
                   for wl in need):
                print(f"[{i}/{len(ready)}] {tag}: already has {sorted(need)}, skipping")
                continue
        print(f"[{i}/{len(ready)}] {tag} ({rec.get('date','?')})")
        cmd = [sys.executable, os.path.join(HERE, "bench.py"),
               "--slangc", rec["slangc"], "--label", tag,
               "--out", os.path.join(args.results, "releases"),
               "--samples", str(args.samples), "--warmup", str(args.warmup)]
        if args.only:
            cmd += ["--only", args.only]
        else:
            # Explicit list rather than bench's default set, so the exclusions
            # above (downstream_required, the api gate) actually govern what
            # runs — otherwise bench would still attempt e.g. codegen_dxil
            # against a release that cannot load dxc, and fail every release.
            cmd += ["--only", ",".join(sorted(all_wls))]
        if args.sweep:
            cmd.append("--sweep")
        if args.api:
            cmd.append("--api")
        try:
            rc = subprocess.run(cmd, timeout=3600).returncode
        except subprocess.TimeoutExpired:
            rc = 1
            print(f"  (note: {tag} timed out after 1h)")
        if rc != 0:
            # bench.py exits 1 if any workload failed; results are still written
            failures.append(tag)
            print(f"  (note: {tag} had >=1 workload issue; partial results kept)")

    print(f"\nsweep complete. results in {args.results}/")
    if failures:
        # Non-zero so CI (release-sweep) won't stamp/push a partial baseline as success.
        print(f"releases with partial/failed results: {', '.join(failures)}")
        sys.exit(1)


if __name__ == "__main__":
    main()
