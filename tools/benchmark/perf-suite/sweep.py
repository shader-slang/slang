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

import analyze
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))


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
    failures = []
    for i, rec in enumerate(ready, 1):
        tag = rec["tag"]
        done = analyze.results_path(args.results, tag)
        if os.path.exists(done) and not args.force:
            # skip only if the requested workloads are already present (a bare
            # results.json from a different --only run must NOT mask new work)
            have = {r["workload"] for r in json.load(open(done))}
            need = want or {w.name for w in __import__("manifest").WORKLOADS}
            if need <= have:
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
        rc = subprocess.run(cmd).returncode
        if rc != 0:
            # bench.py exits 1 if any workload failed; results are still written
            failures.append(tag)
            print(f"  (note: {tag} had >=1 workload issue; partial results kept)")

    print(f"\nsweep complete. results in {args.results}/")
    if failures:
        print(f"releases with partial results: {', '.join(failures)}")


if __name__ == "__main__":
    main()
