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
import math
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


def open_output(path, mode="w"):
    """Open a suite output file for writing under the suite-wide policy:
    UTF-8, LF-only line endings. Every artifact the suite writes — generated
    .slang sources, results/tracking/meta json, rendered HTML/SVG — must be
    byte-identical regardless of the platform that wrote it: Windows' default
    text mode would otherwise write legacy-codepage bytes and CRLF, making the
    corpus platform-dependent and churning the results repo. One helper so the
    policy lives here instead of per-call keyword arguments."""
    return open(path, mode, encoding="utf-8", newline="\n")


def daily_labels(results_dir):
    """One record per daily/<label>/ directory that has a results.json:
    [{label, date, commit, commit_time, path}], label-sorted. The single
    place that knows the daily storage layout and its meta.json fields —
    report.py's combined index, daily_movers' point loader, and any future
    consumer enumerate through here so layout knowledge cannot fork.
    `date` falls back to the label prefix and `commit` to the label suffix
    for points registered before meta carried them.

    `commit` has NO guaranteed length: track.py register stores whatever
    --commit was passed, which the nightly workflow sets from
    `git rev-parse HEAD` (full SHA), while the legacy fallback takes the
    label suffix, which came from `git rev-parse --short HEAD`. So the field
    is a full SHA for points registered with meta and a short one for older
    points. Consumers must treat it as an opaque prefix-comparable string and
    shorten it themselves for display (daily_movers uses `[:9]`, which is
    correct for both shapes); never compare two commits for equality by
    length or slice a fixed width expecting a complete SHA."""
    out = []
    ddir = os.path.join(results_dir, "daily")
    for label in sorted(os.listdir(ddir)) if os.path.isdir(ddir) else []:
        rpath = os.path.join(ddir, label, "results.json")
        if not os.path.exists(rpath):
            continue
        mpath = os.path.join(ddir, label, "meta.json")
        meta = read_json(mpath) if os.path.exists(mpath) else {}
        out.append({"label": label,
                    "date": meta.get("date", label[:10]),
                    "commit": (meta.get("commit") or label.split("-")[-1]),
                    "commit_time": meta.get("commit_time", ""),
                    "path": rpath})
    return out


def read_json(path):
    """json.load with explicit UTF-8, the read-side twin of open_output.

    Suite files are WRITTEN as UTF-8 (open_output), but a bare open() READS
    with the platform default — cp1252 on the Windows runner — so any
    non-ASCII byte (an em dash in a rendered SVG, a smart quote in a compiler
    diagnostic captured into results.json) raises UnicodeDecodeError there
    while passing everywhere else. Every suite read goes through here or
    read_text so the pair cannot drift."""
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def read_text(path):
    """Read a suite-owned text file (SVG, HTML fragment) as UTF-8. See
    read_json for why the encoding must be explicit."""
    with open(path, encoding="utf-8") as fh:
        return fh.read()


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


def unit_of(counter):
    """Unit of a per-workload counter series: "kb" for the memory counters
    (their names end in Kb by convention — peakRssKb and the api-driver's
    [MEM] deltas), "ms" for everything else. One classifier so display code
    and filters cannot disagree about what a value means."""
    return "kb" if counter.endswith("Kb") else "ms"


def fmt_qty(counter, value, signed=False):
    """Human form of a counter value: milliseconds stay ms, kb renders MiB."""
    sign = "+" if signed else ""
    if unit_of(counter) == "kb":
        return f"{value / 1024:{sign}.1f} MiB"
    return f"{value:{sign}.1f} ms"


def canonical_runs(runs):
    """One row per workload for per-release/trend views.

    results.json may contain multiple size rows per workload; collapse to each
    workload's default_size so history and daily points compare like-with-like.
    Falls back to the first row seen for workloads not in the manifest.

    The returned records' `timers` dict is a MIXED-UNIT counter map: ms phase
    timers plus, for manifest track_memory workloads, the kb memory counters
    (peakRssKb and the api-driver deltas). The units are distinguished by
    unit_of()'s Kb-suffix convention — enforced below, where counters are
    synthesized — which is what lets every consumer (tracking, trend,
    pages) handle one uniform {counter: stats} shape without a second
    channel, at the cost that display code must format through fmt_qty
    rather than assuming milliseconds.

    Raises ValueError if a promoted counter's name does not end in Kb, since
    unit_of would then classify it as milliseconds.
    """
    from . import manifest
    best = {}
    for r in runs:
        wl = r["workload"]
        spec = manifest.BY_NAME.get(wl)
        default = spec.default_size if spec else None
        if wl not in best or (r["size"] == default and best[wl]["size"] != default):
            best[wl] = r
    out = []
    for r in best.values():
        # Surface the memory measurements as counter series next to the
        # timers (a shallow copy; the record itself is not mutated) — but
        # only for workloads the manifest flags with track_memory: raw
        # rss_kb is recorded everywhere, while the TRACKED memory surface is
        # deliberately small (most peaks are floor-bound and would only
        # re-draw the session floor across dozens of panels and alert
        # series). unit_of() keeps kb from masquerading as ms in display
        # code, and the bucket partition is unaffected — these names are
        # not in any TREE.
        spec = manifest.BY_NAME.get(r["workload"])
        extra = {}
        if spec is not None and getattr(spec, "track_memory", False):
            if r.get("rss_kb"):
                extra["peakRssKb"] = r["rss_kb"]
            for name, st in (r.get("memory") or {}).items():
                extra[name] = st
        if extra:
            # A `raise`, not an `assert`: this is the promotion point where a
            # memory counter enters the mixed-unit map, and it runs on the perf
            # runner and in report rendering rather than under
            # check-python-core, so `python -O` would erase an assert and let a
            # kb value through to be charted and gated as milliseconds. Same
            # contract, and same reasoning, as bench.parse_mem's guard.
            for name in extra:
                if not name.endswith("Kb"):
                    raise ValueError(f"memory counter '{name}' must end in Kb "
                                     "(unit_of contract)")
            r = dict(r, timers=dict(r.get("timers") or {}, **extra))
        out.append(r)
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

    Threshold rationale:
    - step_thr=1.4 (40%): a single release-over-release jump this large is likely
      a discrete regression introduced in one release, not cumulative drift.
      Set higher than trend.py's --rel 1.10 because single-step classification
      on release history is noisier than nightly drift detection.
    - drift_thr=1.25 (25% total): the end-to-end ratio across all releases
      exceeds this → labelled "drift" (gradual creep across many releases).
      Intentionally higher than trend.py's --rel 1.10: release-over-release
      drift is expected to be larger before it becomes actionable.
    - 1.01 below: 1% noise floor for counting a release-to-release move as
      genuinely upward (vs run-to-run jitter); distinct from the flagging
      thresholds above.

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
    ups = sum(1 for *_, r in steps if r > 1.01)  # 1% noise floor for direction
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


def linfit(xs, ys):
    """Ordinary least squares y = a + b*x. Returns (a, b, r2); (0, 0, 0) with
    fewer than 2 points, where no line is determined (mirrors powfit)."""
    n = len(xs)
    if n < 2:
        return 0.0, 0.0, 0.0
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


def powfit(xs, ys):
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
    loga, k, r2 = linfit(lx, ly)
    return math.exp(loga), k, r2


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


# Import-time self-checks (the directory idiom): the memory pivot in
# canonical_runs is the hinge every consumer relies on, and unit_of/fmt_qty
# are what keep kilobytes from rendering as milliseconds.
assert unit_of("peakRssKb") == "kb" and unit_of("SemanticChecking") == "ms"
assert fmt_qty("peakRssKb", 215040) == "210.0 MiB"
assert fmt_qty("simplifyIR", 12.34, signed=True) == "+12.3 ms"
_rec = {
    "size": 1, "rss_kb": {"median": 5.0},
    "memory": {"apiCreateGlobalSessionRssDeltaKb": {"median": 7.0}},
    "timers": {"compileInner": {"median": 1.0}},
}
_mr = canonical_runs([dict(_rec, workload="rt_renderer")])
assert _mr[0]["timers"]["peakRssKb"] == {"median": 5.0}
assert _mr[0]["timers"]["apiCreateGlobalSessionRssDeltaKb"] == {"median": 7.0}
assert _mr[0]["timers"]["compileInner"] == {"median": 1.0}, "originals preserved"
assert _mr[0]["rss_kb"] == {"median": 5.0}, "source fields not consumed"
_mu = canonical_runs([dict(_rec, workload="conformance")])
assert "peakRssKb" not in _mu[0]["timers"], \
    "memory promotion must be curated: only track_memory workloads"

# The rejection branch, which the promotions above never reach. Pinned for the
# same reason as its twin in bench.py: the guard must be a `raise` rather than
# an `assert` so `python -O` cannot erase it on the perf runner, and catching
# ValueError specifically is what makes a later revert to `assert` fail here.
_bad = dict(_rec, workload="rt_renderer",
            memory={"apiCreateGlobalSessionRssDelta": {"median": 7.0}})
try:
    canonical_runs([_bad])
    raise AssertionError("canonical_runs must reject a counter name without Kb")
except ValueError as _e:
    assert "unit_of contract" in str(_e), \
        f"the rejection must cite the unit_of contract; got {str(_e)!r}"
del _rec, _mr, _mu, _bad


# Import-time self-check for daily_labels, over a throwaway tmpdir (the same
# idiom as fetch_releases.py's zip check). This function is the single place
# that knows the daily storage layout, so a change here forks silently into
# both report.combined_index and daily_movers.daily_points; and its two
# fallbacks only fire for points registered before meta.json carried the
# fields, which no current nightly produces — so nothing else would exercise
# them. The three cases: meta present (fields win, full SHA preserved), meta
# absent (date from the label prefix, commit from the label suffix — a SHORT
# sha, which is why the docstring says the field has no guaranteed length),
# and a directory with no results.json (skipped entirely).
def _daily_labels_selfcheck():
    import shutil
    import tempfile

    d = tempfile.mkdtemp(prefix="analyze_selfcheck_")
    try:
        def point(label, meta=None, results=True):
            p = os.path.join(d, "daily", label)
            os.makedirs(p)
            if results:
                with open_output(os.path.join(p, "results.json")) as fh:
                    fh.write("[]")
            if meta is not None:
                with open_output(os.path.join(p, "meta.json")) as fh:
                    json.dump(meta, fh)

        point("2026-01-02-bbbbbbb",
              {"date": "2026-01-02", "commit": "b" * 40, "commit_time": "t"})
        point("2026-01-01-aaaaaaa")            # legacy: no meta.json
        point("2026-01-03-ccccccc", results=False)  # swept but never completed

        got = daily_labels(d)
        assert [r["label"] for r in got] == ["2026-01-01-aaaaaaa",
                                             "2026-01-02-bbbbbbb"], \
            "daily_labels must be label-sorted and skip points with no results"
        assert got[0]["date"] == "2026-01-01" and got[0]["commit"] == "aaaaaaa", \
            "without meta.json, date/commit fall back to the label's two halves"
        assert got[0]["commit_time"] == "", "missing commit_time defaults to ''"
        assert got[1]["date"] == "2026-01-02" and got[1]["commit"] == "b" * 40, \
            "with meta.json, its fields win and the full SHA is preserved"
        assert os.path.isfile(got[0]["path"]), "path must point at results.json"
        assert daily_labels(os.path.join(d, "nonexistent")) == [], \
            "a results dir with no daily/ yields no points, not an error"
    finally:
        shutil.rmtree(d, ignore_errors=True)


_daily_labels_selfcheck()
del _daily_labels_selfcheck
