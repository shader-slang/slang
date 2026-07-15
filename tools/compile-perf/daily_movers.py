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

from lib import analyze, buckets, manifest

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
        meta = analyze.read_json(mpath) if os.path.exists(mpath) else {}
        vals = {}
        for r in analyze.canonical_runs(analyze.read_json(rpath)):
            for t, st in (r.get("timers") or {}).items():
                if st:
                    vals[(r["workload"], t)] = st[metric]
        if vals:
            out.append((meta.get("date", label[:10]),
                        (meta.get("commit") or label.split("-")[-1])[:9], vals))
    return out


def _partition(workload):
    """(bucket_fn, tree) for the workload's family — breakdown's mutually-
    exclusive decomposition, whose buckets tile the headline exactly (named
    leaves + `(self)` residuals)."""
    spec = manifest.BY_NAME.get(workload)
    if spec is not None and spec.mode == "api":
        return buckets.api_buckets, buckets.API_TREE
    return buckets.buckets, buckets.TREE


def _tree_names(tree):
    names = {tree[0]}
    for child in tree[1]:
        names |= _tree_names(child)
    return names


# Raw counters that duplicate another counter one-to-one (same measured span
# under a second name), so listing both would be noise: endToEndActions ==
# compileInner, checkAllTranslationUnits == SemanticChecking, and
# generateIRForTranslationUnit == generateIR.
_ALIASES = {"endToEndActions", "checkAllTranslationUnits",
            "generateIRForTranslationUnit"}

# A bucket earns a row when it moved the workload total by at least this many
# percentage points; below it, the movement is folded into one remainder row.
# ~Measurement noise for a small bucket lands under this too (median-of-5
# noise is 1-3% of a timer, which is < 0.2pp unless the bucket dominates).
CONTRIB_MIN_PP = 0.2

# Below this many ms a timer is effectively zero: an own-% change from such a
# start would be undefined or explode (0.01 -> 0.06 ms reads as +500%), so
# those rows render a dash instead. One constant keeps the contributor,
# extras, and step-mover sites agreeing on what "starts at ~0" means.
NEAR_ZERO_MS = 0.05


def workload_progress(points, workload, step_rel=0.05):
    """Percent-based progress summary for one workload's daily series:

      overall       (d0, c0, v0, d1, c1, v1, pct) for the headline timer —
                    the 30-day-window "+/-%" answer.
      contributors  [(name, d_ms, pct_own_or_None, contrib_pp)] over the
                    breakdown BUCKET partition (named leaves + `(self)`
                    residuals): buckets tile the headline exactly, so the pp
                    contributions (d_ms / starting headline) sum to the
                    overall %-change. Only buckets with |contrib| >= 0.2pp are
                    listed (they moved the workload total by at least 0.2%);
                    the filtered tail folds into one final
                    ("(remaining N buckets)", d_ms, None, pp) row so the sum
                    property stays visible. pct_own is None when the bucket
                    starts at ~0, where an own-% is undefined.
      extras        [(name, d_ms, pct_own)] — every OTHER reported counter
                    (e.g. readSerializedModuleIR, loadBuiltinModule): they
                    nest inside or extend beyond the partition, so they carry
                    no pp column, but their own movement is still the signal
                    for passes without a dedicated bucket.
      steps         [(d_prev, d, c_prev, c, pct, top_buckets)] — day
                    boundaries where the headline moved >= step_rel vs the
                    PREVIOUS day (both directions), with the step's top
                    buckets as (name, own pct or None) pairs.

    Returns (None, [], [], []) with fewer than 2 daily points.
    """
    head = headline(workload)
    pts = [(d, c, {t: v for (wl, t), v in vals.items() if wl == workload})
           for d, c, vals in points]
    # Keep only points that report the headline timer: every series below
    # (headline, buckets, extras) is built from this one list, so the overall
    # %-change and the contributor pp column share the same first/last points
    # by construction — a partial point (sub-timers but no headline) at an
    # endpoint would otherwise silently break the pp-sums-to-overall
    # property.
    pts = [p for p in pts if head in p[2]]
    if len(pts) < 2:
        return None, [], [], []
    hs = [(d, c, tm[head]) for d, c, tm in pts]
    (d0, c0, v0), (d1, c1, v1) = hs[0], hs[-1]
    overall = (d0, c0, v0, d1, c1, v1, (v1 / v0 - 1) * 100 if v0 else 0.0)

    bucket_fn, tree = _partition(workload)
    bks = [(d, c, bucket_fn(tm)) for d, c, tm in pts]
    first_b, last_b = bks[0][2], bks[-1][2]
    kept, rest_ms, rest_pp, rest_n = [], 0.0, 0.0, 0
    for t in sorted(set(first_b) | set(last_b)):
        a, b = first_b.get(t, 0.0), last_b.get(t, 0.0)
        d_ms = b - a
        pp = d_ms / v0 * 100 if v0 else 0.0
        if abs(pp) >= CONTRIB_MIN_PP:
            own = (b / a - 1) * 100 if a >= NEAR_ZERO_MS else None
            kept.append((t, d_ms, own, pp))
        else:
            rest_ms += d_ms
            rest_pp += pp
            rest_n += 1
    kept.sort(key=lambda r: -abs(r[1]))
    contributors = kept
    if rest_n:
        contributors = kept + [(f"(remaining {rest_n} buckets)",
                                rest_ms, None, rest_pp)]
    # The buckets tile the headline (breakdown.py's contract), so the pp
    # column must reconstruct the overall %-change; fail loudly if a future
    # breakdown change stops tiling rather than publishing wrong attribution.
    pp_sum = sum(c[3] for c in contributors)
    assert abs(pp_sum - overall[6]) < 0.5, \
        f"{workload}: bucket pp sum {pp_sum:.2f} != overall {overall[6]:.2f}"

    covered = _tree_names(tree) | {f"{n} (self)" for n in _tree_names(tree)}
    first_t, last_t = pts[0][2], pts[-1][2]
    extras = []
    # Intersection, not union: a counter reported at only one endpoint has
    # no meaningful net for this informational view (unlike buckets, where a
    # missing endpoint IS a 0 ms phase), so it is dropped rather than
    # defaulted — which is also why the direct indexing below cannot KeyError.
    for t in sorted(set(first_t) & set(last_t)):
        if t in covered or t in _ALIASES:
            continue
        a, b = first_t[t], last_t[t]
        own = (b / a - 1) * 100 if a >= NEAR_ZERO_MS else None
        # informational counters: shown only when they moved noticeably
        if abs(b - a) >= 1.0 or (own is not None and abs(own) >= 5.0):
            extras.append((t, b - a, own))
    extras.sort(key=lambda r: -abs(r[1]))

    # hs and bks are both built from the same filtered `pts` list above, so
    # they are index-aligned by construction — index the buckets directly.
    # (Neither dates nor commits are safely unique across daily labels: dates
    # collide per trend.py, and a commit can repeat under two labels.)
    steps = []
    for i in range(1, len(hs)):
        (dp, cp, vp), (d, c, v) = hs[i - 1], hs[i]
        if vp <= 0:
            continue
        pct = (v / vp - 1) * 100
        # step_rel is a fraction (0.05 = 5%); pct is in percent, so compare
        # against step_rel * 100.
        if abs(pct) < step_rel * 100:
            continue
        b_prev, b_cur = bks[i - 1][2], bks[i][2]
        movers = []
        for t in set(b_prev) | set(b_cur):
            a, b = b_prev.get(t, 0.0), b_cur.get(t, 0.0)
            movers.append((t, b - a, (b / a - 1) * 100 if a >= NEAR_ZERO_MS else None))
        movers.sort(key=lambda m: -abs(m[1]))
        steps.append((dp, d, cp, c, pct,
                      [(t, own) for t, dm, own in movers[:3] if abs(dm) >= 1.0]))
    steps.sort(key=lambda st: -abs(st[4]))
    return overall, contributors, extras, steps


def workload_view(points, workload, step_rel):
    overall, contributors, extras, steps = workload_progress(points, workload, step_rel)
    if overall is None:
        raise SystemExit(f"fewer than 2 daily points for {workload}")
    d0, c0, v0, d1, c1, v1, pct = overall
    print(f"== {workload} — {d0} ({c0}) -> {d1} ({c1}) ==")
    print(f"overall {headline(workload)}: {v0:.1f} -> {v1:.1f} ms  ({pct:+.1f}%)")
    print("contributors (mutually-exclusive buckets; pp sums to the overall %):")
    for t, d_ms, own, contrib in contributors:
        o = f"{own:+6.1f}%" if own is not None else "     -"
        print(f"   {t:32s}{d_ms:+9.1f} ms  ({o} own, {contrib:+5.1f}pp of total)")
    if extras:
        print("other reported counters (nested/overlapping; no pp):")
        for t, d_ms, own in extras:
            o = f"{own:+6.1f}%" if own is not None else "     -"
            print(f"   {t:32s}{d_ms:+9.1f} ms  ({o} own)")
    print(f"day steps >= {step_rel * 100:.0f}% vs previous day:")
    if not steps:
        print("   none")
    for dp, d, cp, c, spct, movers in steps:
        ms = ", ".join(f"{t} {own:+.0f}%" if own is not None else t
                       for t, own in movers)
        print(f"   {dp} -> {d}  {spct:+6.1f}%  ({cp}..{c})  {ms}")


def boundaries(points):
    """[(total_headline_delta_ms, d0, d1, c0, c1, v0, v1)] per consecutive
    daily pair — the data behind the CLI boundary view and the largest-daily-
    change line on the *-tot cadence pages. report.movers_block indexes this
    tuple layout positionally (v0/v1 at 5/6, plus an 8-name unpack after
    prepending a pct) — keep the two in sync if the layout ever changes."""
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
    """Top BOUNDARY_TIMERS movers between two points: [(timer, delta_ms)].
    delta_ms is the SIGNED SUITE-NET for the timer (summed across all
    workloads), so opposite-sign moves in different workloads cancel; a timer
    that regressed +5 ms in one workload and improved -5 ms in another nets
    to ~0 and drops below min_ms by design — this view answers "where did the
    suite total move", not "which workloads moved"."""
    per = {}
    for (wl, t), val in v1.items():
        if t in BOUNDARY_TIMERS and (wl, t) in v0:
            per[t] = per.get(t, 0.0) + val - v0[(wl, t)]
    out = [(t, d) for t, d in sorted(per.items(), key=lambda kv: -abs(kv[1]))
           if abs(d) >= min_ms]
    return out[:limit]


def boundary_view(points, top, min_ms):
    """Print every day boundary ranked by |net suite change|, then decompose
    the `top` largest into their timer movers. `min_ms` gates twice: timer
    rows below it are dropped, and a whole boundary whose net is below it is
    skipped rather than decomposed."""
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
            # The largest single-workload |delta| for the timer; because the
            # printed net is a suite-wide sum, this can carry the opposite
            # sign (one workload regressed while the net improved).
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


# Import-time self-checks over a tiny synthetic fixture, matching the
# directory idiom (lib/manifest.py, new_release_check.py). Only the
# breakdown-independent pieces can run here: workload_progress's bucket
# partition imports breakdown lazily, and at module-import time that import
# is only safe in one direction — its pp-sum invariant is therefore asserted
# at runtime inside workload_progress instead.
_P0 = ("2026-01-01", "aaaaaaaaa", {("w", "compileInner"): 100.0,
                                   ("w", "SemanticChecking"): 50.0,
                                   ("w", "generateIR"): 40.0})
_P1 = ("2026-01-02", "bbbbbbbbb", {("w", "compileInner"): 90.0,
                                   ("w", "SemanticChecking"): 40.0,
                                   ("w", "generateIR"): 45.0})
assert workload_progress([_P0], "w") == (None, [], [], []), \
    "workload_progress must early-return with fewer than 2 points"
_B = boundaries([_P0, _P1])
assert len(_B) == 1 and abs(_B[0][0] - (-10.0)) < 1e-9, \
    "boundaries: one consecutive pair, headline (compileInner) net -10 ms"
# compileInner is deliberately absent from the result: it is a TOTAL, not a
# BOUNDARY_TIMERS leaf, so only the leaves are attributed.
assert timer_deltas(_P0[2], _P1[2]) == [("SemanticChecking", -10.0), ("generateIR", 5.0)], \
    "timer_deltas: signed per-leaf suite-net, sorted by |delta|"
del _P0, _P1, _B


# Import-time self-check for the pp-sum tiling contract against the real
# lib/buckets partition (this used to live in breakdown.py when the partition
# did — the former import cycle constrained fixture placement).
# The fixture includes compileInner's DIRECT children (frontEndExecute,
# generateOutput) so alloc() actually descends: named-leaf buckets, (self)
# residuals at two levels, and the pp sum are all exercised, not just a
# single degenerate compileInner (self) bucket.
_T0 = ("2026-01-01", "aaaaaaaaa",
       {("w", "compileInner"): 100.0, ("w", "frontEndExecute"): 70.0,
        ("w", "SemanticChecking"): 40.0, ("w", "generateIR"): 20.0,
        ("w", "generateOutput"): 25.0})
_T1 = ("2026-01-02", "bbbbbbbbb",
       {("w", "compileInner"): 80.0, ("w", "frontEndExecute"): 60.0,
        ("w", "SemanticChecking"): 30.0, ("w", "generateIR"): 25.0,
        ("w", "generateOutput"): 15.0})
_ov, _contrib, _ex, _st = workload_progress([_T0, _T1], "w")
assert _ov is not None and abs(_ov[6] - (-20.0)) < 1e-9, \
    "workload_progress fixture: headline 100 -> 80 ms must be -20%"
assert len(_contrib) >= 4, \
    "workload_progress fixture must produce a MULTI-bucket partition"
assert abs(sum(c[3] for c in _contrib) - _ov[6]) < 1e-9, \
    "workload_progress fixture: contributor pp must sum to the overall %"
del _T0, _T1, _ov, _contrib, _ex, _st


if __name__ == "__main__":
    main()
