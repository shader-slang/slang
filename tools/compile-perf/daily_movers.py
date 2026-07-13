#!/usr/bin/env python3
"""Summarize where the daily (tip-of-tree) series moved and which timers moved.

Two views over the daily points in a results checkout:

  --workload <name>   per-timer progress for ONE workload: net change over the
                      window, plus each timer's biggest single day-over-day
                      step with its date and commit pair — "which timers
                      improved or regressed, when, and between which commits".

  (default)           suite-wide day boundaries ranked by total headline
                      movement (compileInner, apiTotal for api workloads),
                      with the top boundaries decomposed into the leaf timers
                      that moved and each timer's largest workload
                      contributor. The commit pair bounds the git range to
                      bisect: `git log <c0>..<c1> -- source/`.

Both views read daily/<label>/{results,meta}.json only — release points carry
a different build provenance and would masquerade as steps.

    python3 daily_movers.py --results <slang-compile-perf checkout>
    python3 daily_movers.py --results <checkout> --workload emit_cuda
"""
import argparse
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from lib import analyze, manifest

# Mutually-exclusive-enough leaves for boundary attribution (compiler leaves
# as in analyze.LEAF_TIMERS plus the backend emit timer, the legalize pair,
# and the api-path leaves).
BOUNDARY_TIMERS = analyze.LEAF_TIMERS + [
    "legalizeResourceTypes", "legalizeExistentialTypeLayout",
    "emitEntryPointsSourceFromIR",
    "apiCreateGlobalSession", "apiCreateSession", "apiLoadModule",
    "apiLink", "apiGetCode", "apiSpecialize", "apiReflection",
]


def headline(wl):
    spec = manifest.BY_NAME.get(wl)
    return "apiTotal" if (spec and spec.mode == "api") else "compileInner"


def daily_points(results_dir, metric):
    """[(date, commit9, {(workload, timer): value})], one per daily label."""
    out = []
    ddir = os.path.join(results_dir, "daily")
    for label in sorted(os.listdir(ddir)) if os.path.isdir(ddir) else []:
        rpath = os.path.join(ddir, label, "results.json")
        if not os.path.exists(rpath):
            continue
        mpath = os.path.join(ddir, label, "meta.json")
        meta = json.load(open(mpath)) if os.path.exists(mpath) else {}
        vals = {}
        for r in analyze.canonical_runs(json.load(open(rpath))):
            for t, st in (r.get("timers") or {}).items():
                if st:
                    vals[(r["workload"], t)] = st[metric]
        if vals:
            out.append((meta.get("date", label[:10]),
                        (meta.get("commit") or label.split("-")[-1])[:9], vals))
    return out


def workload_progress(points, workload, step_rel=0.05):
    """Percent-based progress summary for one workload's daily series:

      overall       (d0, c0, v0, d1, c1, v1, pct) for the headline timer —
                    the 30-day-window "+/-%" answer.
      contributors  [(timer, d_ms, pct_own, share)] — leaf timers ranked by
                    |delta ms| over the window; pct_own is the timer's own
                    change, share its fraction of the headline delta.
      steps         [(d_prev, d, c_prev, c, pct, top_timers)] — day boundaries
                    where the headline moved >= step_rel vs the PREVIOUS day
                    (both directions), with the top leaf timers of that step
                    as (timer, own pct) pairs. Percent of previous day, not
                    net ms: a 5% step means the same thing at every scale.

    Returns (None, [], []) with fewer than 2 daily points.
    """
    pts = [(d, c, {t: v for (wl, t), v in vals.items() if wl == workload})
           for d, c, vals in points]
    pts = [p for p in pts if p[2]]
    if len(pts) < 2:
        return None, [], []
    head = headline(workload)
    hs = [(d, c, tm[head]) for d, c, tm in pts if head in tm]
    if len(hs) < 2:
        return None, [], []
    (d0, c0, v0), (d1, c1, v1) = hs[0], hs[-1]
    overall = (d0, c0, v0, d1, c1, v1, (v1 / v0 - 1) * 100 if v0 else 0.0)

    # ALL leaf timers present at both endpoints — no top-N cap and no
    # minimum-delta filter: a flat counter is information too (it rules the
    # pass out as a contributor).
    contributors = []
    hdelta = v1 - v0
    first, last = pts[0][2], pts[-1][2]
    for t in BOUNDARY_TIMERS:
        if t not in first or t not in last or first[t] < 0.5:
            continue
        d_ms = last[t] - first[t]
        contributors.append((t, d_ms, (last[t] / first[t] - 1) * 100,
                             d_ms / hdelta if hdelta else 0.0))
    contributors.sort(key=lambda r: -abs(r[1]))

    steps = []
    for i in range(1, len(hs)):
        (dp, cp, vp), (d, c, v) = hs[i - 1], hs[i]
        if vp <= 0:
            continue
        pct = (v / vp - 1) * 100
        if abs(pct) < step_rel * 100:
            continue
        p_prev = next(tm for dd, _c, tm in pts if dd == dp)
        p_cur = next(tm for dd, _c, tm in pts if dd == d)
        movers = []
        for t in BOUNDARY_TIMERS:
            if t in p_prev and t in p_cur and p_prev[t] >= 0.5:
                movers.append((t, p_cur[t] - p_prev[t],
                               (p_cur[t] / p_prev[t] - 1) * 100))
        movers.sort(key=lambda m: -abs(m[1]))
        steps.append((dp, d, cp, c, pct,
                      [(t, own) for t, _dm, own in movers[:3] if abs(_dm) >= 1.0]))
    steps.sort(key=lambda st: -abs(st[4]))
    return overall, contributors, steps


def workload_view(points, workload, step_rel):
    overall, contributors, steps = workload_progress(points, workload, step_rel)
    if overall is None:
        raise SystemExit(f"fewer than 2 daily points for {workload}")
    d0, c0, v0, d1, c1, v1, pct = overall
    print(f"== {workload} — {d0} ({c0}) -> {d1} ({c1}) ==")
    print(f"overall {headline(workload)}: {v0:.1f} -> {v1:.1f} ms  ({pct:+.1f}%)")
    print("main contributors (leaf timers, window delta):")
    for t, d_ms, own, share in contributors:
        print(f"   {t:32s}{d_ms:+9.1f} ms  ({own:+6.1f}% own, {share * 100:4.0f}% of change)")
    print(f"day steps >= {step_rel * 100:.0f}% vs previous day:")
    if not steps:
        print("   none")
    for dp, d, cp, c, spct, movers in steps:
        ms = ", ".join(f"{t} {own:+.0f}%" for t, own in movers)
        print(f"   {dp} -> {d}  {spct:+6.1f}%  ({cp}..{c})  {ms}")


def boundaries(points):
    """[(total_headline_delta_ms, d0, d1, c0, c1, v0, v1)] per consecutive
    daily pair — the data behind both the CLI boundary view and the landing
    page's recent-movers strip."""
    wls = sorted({wl for *_x, vals in points for (wl, _t) in vals})
    out = []
    for i in range(1, len(points)):
        d0, c0, v0 = points[i - 1]
        d1, c1, v1 = points[i]
        total = sum(v1[(wl, headline(wl))] - v0[(wl, headline(wl))]
                    for wl in wls
                    if (wl, headline(wl)) in v0 and (wl, headline(wl)) in v1)
        out.append((total, d0, d1, c0, c1, v0, v1))
    return out


def timer_deltas(v0, v1, limit=6, min_ms=2.0):
    """Top BOUNDARY_TIMERS movers between two points: [(timer, delta_ms)]."""
    per = {}
    for (wl, t), val in v1.items():
        if t in BOUNDARY_TIMERS and (wl, t) in v0:
            per[t] = per.get(t, 0.0) + val - v0[(wl, t)]
    out = [(t, d) for t, d in sorted(per.items(), key=lambda kv: -abs(kv[1]))
           if abs(d) >= min_ms]
    return out[:limit]


def boundary_view(points, top, min_ms):
    bounds = boundaries(points)

    print(f"{'boundary':26s}{'commits':22s}{'net suite change':>18}")
    for total, d0, d1, c0, c1, *_x in sorted(bounds, key=lambda b: -abs(b[0])):
        print(f"{d0} -> {d1:12s}{c0}..{c1}   {total:+12.0f} ms")

    print("\n=== timer movers at the top boundaries ===")
    for total, d0, d1, c0, c1, v0, v1 in sorted(bounds, key=lambda b: -abs(b[0]))[:top]:
        if abs(total) < min_ms:
            continue
        print(f"\n-- {d0} -> {d1} ({c0}..{c1}): suite {total:+.0f} ms "
              f"-- bisect: git log {c0}..{c1} -- source/")
        for t, d in timer_deltas(v0, v1, min_ms=min_ms):
            wl_best = max(((wl, v1[(wl, tt)] - v0[(wl, tt)])
                           for (wl, tt) in v1 if tt == t and (wl, tt) in v0),
                          key=lambda x: abs(x[1]))
            print(f"   {t:32s}{d:+9.0f} ms   "
                  f"(largest: {wl_best[0]} {wl_best[1]:+.0f})")


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--workload", default=None,
                    help="per-timer view for one workload (default: suite-wide "
                         "boundary view)")
    ap.add_argument("--metric", default="median", choices=["min", "median", "mean"])
    ap.add_argument("--top", type=int, default=3,
                    help="boundaries to decompose in the suite view")
    ap.add_argument("--min-ms", type=float, default=2.0)
    ap.add_argument("--step-rel", type=float, default=0.05,
                    help="day-step threshold vs the previous day in the "
                         "workload view (0.05 = 5%%)")
    args = ap.parse_args()

    points = daily_points(args.results, args.metric)
    if len(points) < 2:
        raise SystemExit("fewer than 2 daily points; nothing to summarize")
    if args.workload:
        workload_view(points, args.workload, args.step_rel)
    else:
        boundary_view(points, args.top, args.min_ms)


if __name__ == "__main__":
    main()
