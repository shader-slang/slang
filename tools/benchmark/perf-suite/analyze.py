#!/usr/bin/env python3
"""Stack per-release perf results into time-series and flag regressions.

Loads results/<tag>/results.json for every release in releases/index.json
(chronological order), then for each (workload, timer):
  - builds a release-ordered series of the chosen metric (median by default —
    reflects the typical run; --metric min/mean also available),
  - flags release-over-release step-changes that exceed both a relative and an
    absolute threshold (the latter scaled by sample noise),
  - for a flagged compileInner jump, attributes it to the child stage timer with
    the largest concurrent delta.
Also derives the diagnostics path-cost series (errors - clean).

Outputs a ranked console report plus results/_analysis/{series.csv,flags.csv}.
"""
import argparse
import csv
import json
import math
import os

HERE = os.path.dirname(os.path.abspath(__file__))

# The profiler timers are NESTED:
#   compileInner
#     frontEndExecute        -> parseTranslationUnit, SemanticChecking, generateIR
#     generateOutput         -> linkAndOptimizeIR -> {specializeModule, simplifyIR,
#                                                     linkIR, unrollLoopsInModule}
# Attributing a compileInner jump to an *outer* timer (generateOutput,
# linkAndOptimizeIR) double-counts its children. So attribution uses LEAF timers
# only. "emit" is the synthetic leaf generateOutput - linkAndOptimizeIR (target
# emission + any bundled downstream tool such as spirv-opt).
LEAF_TIMERS = ["parseTranslationUnit", "SemanticChecking", "generateIR",
               "specializeModule", "simplifyIR", "linkIR", "unrollLoopsInModule"]


def leaf_deltas(lookup, ptag, tag, wl):
    """{leaf: delta_ms} across a release boundary, incl. synthetic 'emit'."""
    out = {}
    for lt in LEAF_TIMERS:
        a, b = lookup.get((ptag, wl, lt)), lookup.get((tag, wl, lt))
        if a is not None and b is not None:
            out[lt] = b - a
    # emit = generateOutput - linkAndOptimizeIR, on each side
    def emit(t):
        g, l = lookup.get((t, wl, "generateOutput")), lookup.get((t, wl, "linkAndOptimizeIR"))
        return (g - l) if (g is not None and l is not None) else None
    ea, eb = emit(ptag), emit(tag)
    if ea is not None and eb is not None:
        out["emit"] = eb - ea
    return out


def canonical_runs(runs):
    """One row per workload for per-release/trend views. A `--sweep` enriches a
    results.json with several sizes for a workload (e.g. complexity_ladder); the
    trend charts compare like-with-like across releases, so collapse to each
    workload's default_size (falling back to the first row seen). Scaling
    analysis reads the full multi-size data separately (ladder_scaling.py)."""
    import manifest
    best = {}
    for r in runs:
        wl = r["workload"]
        spec = manifest.BY_NAME.get(wl)
        default = spec.default_size if spec else None
        if wl not in best or (r["size"] == default and best[wl]["size"] != default):
            best[wl] = r
    return list(best.values())


def load_series(index, results_dir, metric):
    """{(workload,timer): [(tag,date,value), ...]} in release order, plus a
    {(tag,workload): {timer: value}} lookup for attribution."""
    series = {}
    lookup = {}
    order = []
    for rec in index:
        if "slangc" not in rec:
            continue
        tag, date = rec["tag"], rec.get("date", "?")
        path = os.path.join(results_dir, tag, "results.json")
        if not os.path.exists(path):
            continue
        order.append((tag, date))
        with open(path) as fh:
            runs = canonical_runs(json.load(fh))
        for run in runs:
            wl = run["workload"]
            for timer, st in run["timers"].items():
                if not st:
                    continue
                val = st.get(metric)
                if val is None:
                    continue
                series.setdefault((wl, timer), []).append((tag, date, val))
                lookup[(tag, wl, timer)] = val
    return series, lookup, order


def classify(values, step_thr=1.4, drift_thr=1.25):
    """Classify a release-ordered [(tag,date,val)] series as 'step', 'drift',
    'faster', or 'flat', separating a single dominant jump from gradual creep.

    Returns dict with total ratio, the largest single-release step (+where), and
    the fraction of release-to-release moves that were increases (a high value on
    a 'drift' series = steady upward creep rather than noise)."""
    vals = [v for _, _, v in values]
    if len(vals) < 2 or vals[0] <= 0:
        return None
    steps = [(values[i - 1][0], values[i][0], vals[i] / vals[i - 1])
             for i in range(1, len(vals)) if vals[i - 1] > 0]
    total = vals[-1] / vals[0]
    max_step = max(steps, key=lambda s: s[2]) if steps else (None, None, 1.0)
    ups = sum(1 for *_, r in steps if r > 1.01)
    up_frac = ups / len(steps) if steps else 0.0
    if max_step[2] >= step_thr:
        kind = "step"
    elif total >= drift_thr:
        kind = "drift"
    elif total <= 0.9:
        kind = "faster"
    else:
        kind = "flat"
    return {"kind": kind, "total": total, "max_step": max_step[2],
            "max_step_at": f"{max_step[0]}->{max_step[1]}" if max_step[0] else "",
            "up_frac": up_frac, "n_steps": len(steps)}


def _linfit(xs, ys):
    """Ordinary least squares y = a + b*x. Returns (a, b, r2)."""
    n = len(xs)
    sx, sy = sum(xs), sum(ys)
    sxx = sum(x * x for x in xs)
    sxy = sum(x * y for x, y in zip(xs, ys))
    denom = n * sxx - sx * sx
    if denom == 0:
        return ys[0], 0.0, 0.0
    b = (n * sxy - sx * sy) / denom
    a = (sy - b * sx) / n
    ybar = sy / n
    ss_tot = sum((y - ybar) ** 2 for y in ys) or 1.0
    ss_res = sum((y - (a + b * x)) ** 2 for x, y in zip(xs, ys))
    return a, b, 1 - ss_res / ss_tot


def _powfit(xs, ys):
    """Power-law fit t = a * N^k via OLS on (log N, log t). Returns (a, k, r2),
    with r2 measured in log space. k is the honest super-linearity exponent —
    k≈1 linear, k>1 super-linear, k<1 sub-linear — and unlike the linear floor it
    never goes negative on a convex curve. Needs positive xs/ys; falls back to
    (0, 0, 0) otherwise."""
    pts = [(x, y) for x, y in zip(xs, ys) if x > 0 and y > 0]
    if len(pts) < 2:
        return 0.0, 0.0, 0.0
    lx = [math.log(x) for x, _ in pts]
    ly = [math.log(y) for _, y in pts]
    loga, k, r2 = _linfit(lx, ly)
    return math.exp(loga), k, r2


def slope_report(results_dir, label, metric):
    """Decompose compile time into fixed floor + per-element slope from a
    --sweep run (multiple sizes per workload). A regression in `floor` (heavier
    stdlib, e.g. PR #9808) is a different bug from a regression in `slope`
    (a pass got per-element slower) or in scaling (slope rising super-linearly)."""
    path = os.path.join(results_dir, label, "results.json")
    if not os.path.exists(path):
        raise SystemExit(f"no results at {path} (run bench.py --sweep --label {label})")
    by_wl = {}
    for r in json.load(open(path)):
        st = r["timers"].get("compileInner")
        if st and r["size"] > 0:
            by_wl.setdefault(r["workload"], []).append((r["size"], st[metric]))
    print(f"Floor + slope fit (compileInner, {metric}) for label '{label}'")
    print(f"{'workload':18s}{'floor(ms)':>11}{'slope(ms/unit)':>16}{'R^2':>7}   sizes")
    print("-" * 72)
    for wl, pts in sorted(by_wl.items()):
        pts = sorted(set(pts))
        if len(pts) < 2:
            continue
        xs, ys = [p[0] for p in pts], [p[1] for p in pts]
        a, b, r2 = _linfit(xs, ys)
        print(f"{wl:18s}{a:11.1f}{b:16.4f}{r2:7.3f}   {[x for x in xs]}")
    print("\nfloor = fixed per-compile cost (core-module load/link); "
          "slope = marginal cost per generated unit.")


def flag_steps(values, rel_thr, abs_floor):
    """values: [(tag,date,val)]. Yield (prev_tag,tag,prev,cur,rel,abs)."""
    flags = []
    for i in range(1, len(values)):
        ptag, _, pv = values[i - 1]
        tag, _, cv = values[i]
        if pv <= 0:
            continue
        rel = cv / pv
        delta = cv - pv
        if rel >= rel_thr and delta >= abs_floor:
            flags.append((ptag, tag, pv, cv, rel, delta))
    return flags


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--index", default=os.path.join(HERE, "releases", "index.json"))
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    ap.add_argument("--metric", default="median", choices=["min", "median", "mean"])
    ap.add_argument("--rel", type=float, default=1.15, help="min ratio to flag (1.15 = +15%%)")
    ap.add_argument("--abs", type=float, default=2.0, help="min absolute ms jump to flag")
    ap.add_argument("--primary-only", action="store_true",
                    help="only consider each workload's primary timers")
    ap.add_argument("--slope-label", default=None,
                    help="fit floor+slope (time = a + b*N) from a --sweep run's "
                         "multi-size results under results/<label>/ and exit")
    args = ap.parse_args()

    if args.slope_label:
        slope_report(args.results, args.slope_label, args.metric)
        return

    with open(args.index) as fh:
        index = json.load(fh)
    series, lookup, order = load_series(index, args.results, args.metric)
    if not order:
        raise SystemExit("no results found; run sweep.py first")

    # which timers are 'primary' per workload (read from any result file)
    primary = {}
    for rec in index:
        if "slangc" not in rec:
            continue
        p = os.path.join(args.results, rec["tag"], "results.json")
        if os.path.exists(p):
            for run in json.load(open(p)):
                primary[run["workload"]] = set(run.get("primary_timers", []))
            break

    outdir = os.path.join(args.results, "_analysis")
    os.makedirs(outdir, exist_ok=True)

    # full series csv
    with open(os.path.join(outdir, "series.csv"), "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["workload", "timer", "tag", "date", f"{args.metric}_ms"])
        for (wl, timer), vals in sorted(series.items()):
            for tag, date, val in vals:
                w.writerow([wl, timer, tag, date, val])

    # these timers are ~aliases of another and just duplicate rows in the report
    ALIAS = {"endToEndActions", "checkAllTranslationUnits", "generateIRForTranslationUnit"}

    # collect flags
    all_flags = []
    for (wl, timer), vals in series.items():
        if timer in ALIAS:
            continue
        if args.primary_only and timer not in primary.get(wl, {timer}):
            continue
        if len(vals) < 2:
            continue
        # noise floor: use a small constant; min metric already rejects most noise
        for ptag, tag, pv, cv, rel, delta in flag_steps(vals, args.rel, args.abs):
            is_primary = timer in primary.get(wl, set())
            attribution = ""
            if timer == "compileInner":
                # attribute to the LEAF pass with the biggest concurrent delta
                deltas = leaf_deltas(lookup, ptag, tag, wl)
                if deltas:
                    best = max(deltas, key=lambda k: deltas[k])
                    if deltas[best] > 0:
                        attribution = f"{best} (+{deltas[best]:.1f}ms)"
            all_flags.append({
                "workload": wl, "timer": timer, "from": ptag, "to": tag,
                "prev_ms": round(pv, 2), "cur_ms": round(cv, 2),
                "ratio": round(rel, 3), "delta_ms": round(delta, 2),
                "primary": is_primary, "attribution": attribution,
            })

    all_flags.sort(key=lambda f: (-f["delta_ms"]))
    with open(os.path.join(outdir, "flags.csv"), "w", newline="") as fh:
        cols = ["workload", "timer", "from", "to", "prev_ms", "cur_ms",
                "ratio", "delta_ms", "primary", "attribution"]
        w = csv.DictWriter(fh, fieldnames=cols)
        w.writeheader()
        for f in all_flags:
            w.writerow(f)

    # diagnostics path-cost (errors - clean), per release
    diag = []
    for tag, date in order:
        e = lookup.get((tag, "diagnostics_errors", "SemanticChecking"))
        c = lookup.get((tag, "diagnostics_clean", "SemanticChecking"))
        if e is not None and c is not None:
            diag.append((tag, date, round(e - c, 2)))

    # ---- console report ----
    first_tag, _ = order[0]
    last_tag, _ = order[-1]
    print(f"Releases analyzed ({len(order)}): {first_tag} .. {last_tag}")
    print(f"Metric: {args.metric}; flag if ratio>={args.rel} and jump>={args.abs}ms\n")

    print("== End-to-end drift (compileInner, first -> last) ==")
    classes = {}
    for (wl, timer), vals in sorted(series.items()):
        if timer != "compileInner" or len(vals) < 2:
            continue
        f, l = vals[0][2], vals[-1][2]
        ratio = l / f if f else 0
        c = classify(vals)
        classes[wl] = c
        kind = c["kind"].upper() if c else "?"
        print(f"  {wl:18s} {f:9.1f} -> {l:9.1f} ms  ({ratio:4.2f}x)  [{kind}]")

    # Gradual drift: meaningful total rise with NO single dominant step — the
    # creep that step-change detection structurally misses (e.g. parse, sema).
    drifters = [(wl, c) for wl, c in classes.items() if c and c["kind"] == "drift"]
    drifters.sort(key=lambda x: -x[1]["total"])
    print("\n== Gradual drift (no single step >= 1.4x; ranked by total rise) ==")
    if not drifters:
        print("  none")
    for wl, c in drifters:
        print(f"  {wl:18s} total {c['total']:.2f}x over {c['n_steps']} steps  "
              f"(biggest single {c['max_step']:.2f}x; {c['up_frac']*100:.0f}% of steps rose)")

    print("\n== Flagged step-changes (ranked by absolute jump) ==")
    if not all_flags:
        print("  none")
    for f in all_flags[:40]:
        star = "*" if f["primary"] else " "
        attr = f"  [{f['attribution']}]" if f["attribution"] else ""
        print(f" {star}{f['workload']:17s} {f['timer']:18s} "
              f"{f['from']}->{f['to']}  {f['prev_ms']:8.1f} -> {f['cur_ms']:8.1f} ms "
              f"({f['ratio']:.2f}x, +{f['delta_ms']:.1f}){attr}")

    print("\n== Per-release series (primary timer of each workload) ==")
    print("   (each cell = min ms at that release; >=1.4x jump vs prev marked '<')")
    short = [t for t, _ in order]
    print("   " + " ".join(f"{t.replace('v20','').replace('.', '_'):>7s}" for t in short))
    for (wl, timer), vals in sorted(series.items()):
        if timer not in primary.get(wl, set()):
            continue
        byt = {t: v for t, _, v in vals}
        cells, prev = [], None
        for t in short:
            v = byt.get(t)
            if v is None:
                cells.append(f"{'-':>7s}")
            else:
                mark = "<" if (prev and v / prev >= 1.4) else " "
                cells.append(f"{v:6.0f}{mark}")
            if v is not None:
                prev = v
        print(f"  {wl}.{timer}")
        print("   " + " ".join(cells))

    if diag:
        print("\n== Diagnostics path cost (SemanticChecking: errors - clean) ==")
        base = diag[0][2]
        for tag, date, v in diag:
            ratio = v / base if base else 0
            print(f"  {tag:12s} {v:8.1f} ms  ({ratio:4.2f}x)")

    print(f"\nwrote {outdir}/series.csv and {outdir}/flags.csv")


if __name__ == "__main__":
    main()
