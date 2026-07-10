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
    all_wls = {w.name for w in manifest.WORKLOADS
               if not w.platforms or sys.platform in w.platforms}
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
                prev = json.load(open(done))
            except (json.JSONDecodeError, OSError) as e:
                prev = []
                print(f"  (note: unreadable {done} ({e}); re-running)")
            sizes = {}
            for r in prev if isinstance(prev, list) else []:
                if isinstance(r, dict) and "workload" in r:
                    sizes.setdefault(r["workload"], set()).add(r.get("size"))
            need = want or all_wls

            # `complete` closes over `sizes`, `args.sweep`, and `manifest` — all
            # stable across loop iterations. It does NOT capture the loop variable.
            def complete(wl):
                szs = sizes.get(wl)
                if not szs:
                    return False
                spec = manifest.BY_NAME.get(wl)
                if args.sweep and spec and spec.sweep_sizes:
                    # Every configured ladder size must be present: an
                    # interrupted prior sweep (e.g. a per-run timeout) leaves a
                    # valid results.json with only the low sizes, and accepting
                    # it here would leave a permanent gap in the scaling curve
                    # that only --force could backfill. This also means a
                    # retuned ladder in the manifest re-sweeps affected
                    # releases on the next run, by design.
                    return set(spec.sweep_sizes) <= szs
                # True covers: no --sweep requested; a spec with no ladder
                # (nothing to sweep); and a workload present in results.json
                # but gone from the manifest (no ladder left to validate).
                return True

            if all(complete(wl) for wl in need):
                print(f"[{i}/{len(ready)}] {tag}: already has {sorted(need)}, skipping")
                continue
        print(f"[{i}/{len(ready)}] {tag} ({rec.get('date','?')})")
        cmd = [sys.executable, os.path.join(HERE, "bench.py"),
               "--slangc", rec["slangc"], "--label", tag,
               "--out", os.path.join(args.results, "releases"),
               "--samples", str(args.samples), "--warmup", str(args.warmup)]
        if args.only:
            cmd += ["--only", args.only]
        if args.sweep:
            cmd.append("--sweep")
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
