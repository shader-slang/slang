"""Stack per-release perf results into time-series and flag regressions.

Loads releases/<tag>/results.json for every release in the index
(chronological order), then for each (workload, timer):
  - builds a release-ordered series of the chosen metric (median by default —
    reflects the typical run; --metric min/mean also available),
  - flags release-over-release step-changes that exceed both a relative and a
    fixed absolute threshold (`--abs`; the median metric already rejects most
    run-to-run noise, so the floor is a flat constant rather than per-timer),
  - for a flagged compileInner jump, attributes it to the child stage timer with
    the largest concurrent delta.
Also derives the diagnostics path-cost series (errors - clean).
"""
import json
import os
import re

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


def results_dir_for(results_dir, label):
    """Return the directory that holds `label`'s results.json.

    Searches three layout conventions in order:
      1. ``<results>/releases/<label>/`` — canonical home for release-tag sweeps.
      2. ``<results>/daily/<label>/``    — nightly ToT sweeps.
      3. ``<results>/<label>/``          — ad-hoc / dev builds at the top level.

    If none of those directories contains a results.json, returns the
    ``releases/<label>/`` path so callers can construct a not-yet-created
    path without a special case.
    """
    for sub in ("releases", "daily", ""):
        d = os.path.join(results_dir, sub, label) if sub else os.path.join(results_dir, label)
        if os.path.exists(os.path.join(d, "results.json")):
            return d
    return os.path.join(results_dir, "releases", label)


def results_path(results_dir, label):
    """Path to a label's results.json — see results_dir_for()."""
    return os.path.join(results_dir_for(results_dir, label), "results.json")


def leaf_deltas(lookup, ptag, tag, wl):
    """{leaf: delta_ms} across a release boundary.

    Includes a synthetic 'emit_overhead' key (generateOutput − linkAndOptimizeIR)
    representing target-code emission plus any bundled downstream tool (spirv-opt).
    Named 'emit_overhead' rather than 'emit' to avoid confusion with the real
    emitEntryPointsSourceFromIR leaf timer visible in breakdown.py's TREE.
    """
    out = {}
    for lt in LEAF_TIMERS:
        a, b = lookup.get((ptag, wl, lt)), lookup.get((tag, wl, lt))
        if a is not None and b is not None:
            out[lt] = b - a
    # emit_overhead = generateOutput − linkAndOptimizeIR (emission + downstream tool)
    def _emit(t):
        g, l = lookup.get((t, wl, "generateOutput")), lookup.get((t, wl, "linkAndOptimizeIR"))
        return (g - l) if (g is not None and l is not None) else None
    ea, eb = _emit(ptag), _emit(tag)
    if ea is not None and eb is not None:
        out["emit_overhead"] = eb - ea
    return out


def canonical_runs(runs):
    """One row per workload for per-release/trend views.

    results.json may contain multiple size rows per workload; collapse to each
    workload's default_size so history and daily points compare like-with-like.
    Falls back to the first row seen for workloads not in the manifest.
    """
    from . import manifest
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
        path = results_path(results_dir, tag)
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




def short_tag(tag):
    """Compact label: release 'v2026.10' -> '2026.10'; daily label '2026-06-08-<sha>' -> '06-08'."""
    if re.match(r"v\d", tag):
        return tag.replace("v20", "")
    m = re.match(r"\d{4}-(\d{2}-\d{2})-", tag)
    return m.group(1) if m else tag


def is_daily(tag):
    """True for a daily ToT label '<YYYY-MM-DD>-<sha>' (vs a release 'vX.Y')."""
    return bool(re.match(r"\d{4}-\d{2}-\d{2}-", tag))
