#!/usr/bin/env python3
"""Reproducibility check: compare a fresh sweep against a saved baseline.

For every (release, workload) present in both runs, compare the `compileInner`
timer (min and median) and report the percent drift. Same binaries + same
workloads run twice should reproduce within timing noise; large drift points to
runner contention or a non-deterministic workload, not a real perf change.

    python3 compare_repro.py --baseline results_baseline_orig --new results

Only workloads present in BOTH runs are compared (new workloads added after the
baseline are listed separately, not diffed).
"""
import argparse
import json
import os
import statistics


def load(run_dir):
    """run_dir/<tag>/results.json -> {(tag, workload, size): record}."""
    out = {}
    for tag in sorted(os.listdir(run_dir)):
        jp = os.path.join(run_dir, tag, "results.json")
        if not os.path.isfile(jp):
            continue
        for r in json.load(open(jp)):
            out[(tag, r["workload"], r["size"])] = r
    return out


def ci(rec, stat):
    t = rec.get("timers", {}).get("compileInner")
    return t.get(stat) if t else None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--baseline", default="results_baseline_orig")
    ap.add_argument("--new", default="results")
    ap.add_argument("--stat", default="min", choices=["min", "median"],
                    help="compileInner statistic to compare (min is most stable)")
    ap.add_argument("--warn", type=float, default=10.0,
                    help="flag drift above this percent")
    args = ap.parse_args()

    base = load(args.baseline)
    new = load(args.new)
    common = sorted(set(base) & set(new))
    only_new = sorted(set(new) - set(base))
    only_base = sorted(set(base) - set(new))

    print(f"baseline rows: {len(base)}   new rows: {len(new)}   "
          f"common: {len(common)}\n")

    rows = []
    for key in common:
        b, n = ci(base[key], args.stat), ci(new[key], args.stat)
        if b is None or n is None or b == 0:
            continue
        drift = (n / b - 1.0) * 100.0
        rows.append((abs(drift), drift, b, n, key))

    print(f"compileInner {args.stat}: new vs baseline drift, "
          f"worst first (flag > {args.warn}%)\n")
    print(f"{'release':16s}{'workload':22s}{'base(ms)':>10}{'new(ms)':>10}"
          f"{'drift%':>9}")
    print("-" * 67)
    flagged = 0
    for adrift, drift, b, n, (tag, wl, sz) in sorted(rows, reverse=True):
        mark = "  <-- " if adrift > args.warn else ""
        if adrift > args.warn:
            flagged += 1
        print(f"{tag:16s}{wl:22s}{b:>10.2f}{n:>10.2f}{drift:>+8.1f}%{mark}")

    drifts = [d for _, d, *_ in rows]
    if drifts:
        print(f"\nsummary over {len(drifts)} common runs:")
        print(f"  median |drift|: {statistics.median(abs(d) for d in drifts):.1f}%")
        print(f"  mean   |drift|: {statistics.mean(abs(d) for d in drifts):.1f}%")
        print(f"  max    |drift|: {max(abs(d) for d in drifts):.1f}%")
        print(f"  runs over {args.warn}%: {flagged}/{len(drifts)}")

    if only_new:
        wls = sorted(set(wl for _, wl, _ in only_new))
        print(f"\nworkloads only in new run (no baseline to compare): {wls}")
    if only_base:
        wls = sorted(set(wl for _, wl, _ in only_base))
        print(f"workloads only in baseline (failed/absent in new run): {wls}")


if __name__ == "__main__":
    main()
