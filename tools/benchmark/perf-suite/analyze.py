#!/usr/bin/env python3
"""Stack per-release perf results into time-series and flag regressions.

Loads results/<tag>/results.json for every release in releases/index.json
(chronological order), then for each (workload, timer):
  - builds a release-ordered series of the chosen metric (min by default —
    most noise-robust for cross-version comparison),
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
            runs = json.load(fh)
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
    ap.add_argument("--metric", default="min", choices=["min", "median", "mean"])
    ap.add_argument("--rel", type=float, default=1.15, help="min ratio to flag (1.15 = +15%%)")
    ap.add_argument("--abs", type=float, default=2.0, help="min absolute ms jump to flag")
    ap.add_argument("--primary-only", action="store_true",
                    help="only consider each workload's primary timers")
    args = ap.parse_args()

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
    for (wl, timer), vals in sorted(series.items()):
        if timer != "compileInner" or len(vals) < 2:
            continue
        f, l = vals[0][2], vals[-1][2]
        ratio = l / f if f else 0
        mark = "  <== REGRESSED" if ratio >= args.rel else ""
        print(f"  {wl:18s} {f:9.1f} -> {l:9.1f} ms  ({ratio:4.2f}x){mark}")

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
