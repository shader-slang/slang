#!/usr/bin/env python3
"""Drift alert for the nightly compile-perf tracking series.

Reads the tracking series (track.py's tracking/tracking.json) and compares the
latest point — tonight's tip-of-tree (ToT) sweep — against the trailing median of
the previous N points, per (workload, counter). "Counter" rather than "timer"
throughout: the series is mixed-unit, carrying kb memory counters alongside the
ms phase timers, and which one a value is decides both its formatting and its
absolute floor.

A metric that rises beyond both a relative and an absolute threshold is flagged:
printed, emitted as a GitHub Actions ::error:: annotation + step-summary row,
and (unless --no-fail) the process exits non-zero so the nightly job goes red.
This catches the gradual drift a per-PR step gate structurally misses.

Absolute compile times are runner-specific, so comparisons are restricted to
points sharing the current point's runner fingerprint. If the latest point ran on
a different runner than the release history was built on, the history is stale for
this machine — re-run perf-compile-release-sweep (force=true); trend.py warns and
compares only against same-runner points.

    python3 trend.py --results <perf-results>        # after track.py rebuild
    python3 trend.py --results <dir> --window 7 --rel 1.10 --abs 2.0
"""
import argparse
import json
import os
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)  # allow running from any directory

from lib import analyze, manifest
# The exit codes live in slack_status because it is the consumer that must
# stay stdlib-only, and importing them from there is what keeps the producer
# and the classifier from drifting apart.
from slack_status import EXIT_CANNOT_EVALUATE, EXIT_REGRESSION


def abort(msg):
    """Report `msg` and exit with EXIT_CANNOT_EVALUATE.

    Use this for every "cannot judge this run" exit, never `raise
    SystemExit(msg)`: the string form prints the same text but exits 1, which
    is the REGRESSION code, and the Slack step then announces a >=10%
    regression for a run that was never measured.
    """
    print(msg, file=sys.stderr)
    raise SystemExit(EXIT_CANNOT_EVALUATE)


def timers_for(workload):
    """The ms timers worth alerting on for a workload: its manifest primary
    timers, always including compileInner (the holistic signal).

    Deliberately timers only, despite the mixed-unit series — memory counters
    are declared nowhere per workload, so judged() is the single place the two
    kinds are unioned."""
    spec = manifest.BY_NAME.get(workload)
    timers = set(spec.primary_timers) if spec else set()
    timers.add("compileInner")
    return timers


def abs_floor_for(counter, ms_floor):
    """The absolute-delta gate for a counter: `ms_floor` (the --abs argument)
    for time, a fixed 1 MiB for the kb-unit memory counters — a few-KB wobble
    on a ~200 MB value must not page anyone (the ratio gate is the primary
    filter for both units)."""
    return 1024.0 if analyze.unit_of(counter) == "kb" else ms_floor


def judged(workload, counter):
    """Whether the trend check judges this (workload, counter) series: the
    workload's alert timers, plus EVERY kb-unit memory counter — memory
    counters are not in any primary_timers list (they are synthesized by
    canonical_runs, not declared per workload), and without this the memory
    alert path would be unreachable."""
    return counter in timers_for(workload) or analyze.unit_of(counter) == "kb"


def emit_gha_command(line):
    """Emit a GitHub Actions workflow command (e.g. ::error::) if running under Actions."""
    if os.environ.get("GITHUB_ACTIONS") == "true":
        print(line)


def write_step_summary(md):
    """Append markdown to the GitHub Actions step summary ($GITHUB_STEP_SUMMARY)."""
    path = os.environ.get("GITHUB_STEP_SUMMARY")
    if path:
        with analyze.open_output(path, "a") as fh:
            fh.write(md + "\n")


def classify_metric(ratio, delta, rel, warn_rel, abs_floor):
    """Classify one metric's movement as "error", "warning", or None.

    "error" fails the nightly; "warning" is reported (yellow Slack, step
    summary) but exits 0; None is silent. `abs_floor` gates BOTH tiers — a
    large ratio on a tiny absolute delta is measurement noise whichever band
    it lands in, so a 50% rise in a 3 ms timer must stay silent rather than
    becoming a warning.

    Taking `abs_floor` as a parameter rather than reading args.abs is
    deliberate: it keeps this pure (so the self-checks below need no argparse
    or filesystem), and it is the seam a per-counter floor plugs into — see
    the note at the call site.

    The bands assume warn_rel < rel, which check_threshold_order enforces at
    parse time; at ratio == rel the metric is an error, since `rel` is
    documented as the threshold at which the nightly fails."""
    if delta < abs_floor:
        return None
    if ratio >= rel:
        return "error"
    if ratio >= warn_rel:
        return "warning"
    return None


def check_threshold_order(rel, warn_rel):
    """Raise SystemExit unless the warning band sits STRICTLY below the error
    band; return None when the pair is usable.

    A function rather than an inline `if` in main() so that the guard itself —
    not a second copy of its condition — is what the self-checks exercise. A
    separate predicate returning a bool would be a second source of truth for
    the relation, and could drift from the branch that actually rejects.

    Equal thresholds are rejected as well as inverted ones, which is why the
    comparison is `>=` and not `>`: at warn_rel == rel, classify_metric's
    error branch matches every ratio the warning branch would have, so the
    warning tier does not misfire — it silently ceases to exist. Inversion is
    worse still: the error branch claims the whole warning band, so every
    5-10% move fails the nightly, which is the alert fatigue the two-tier gate
    was added to remove."""
    if warn_rel >= rel:
        abort(f"--warn-rel ({warn_rel}) must be < --rel ({rel}): "
              f"the warning band sits below the error band, and a "
              f"pair that is equal or inverted silently promotes "
              f"every warning-level change to an error")


def point_runner(point, history_runner):
    """The runner fingerprint for `point`, or "" when it is unknown.

    Release points deliberately omit a per-point runner because they all use
    the release-sweep runner recorded at series level. Daily points do not
    share that invariant: an absent runner means the legacy point's machine is
    unknown, so inheriting `history_runner` would make unrelated measurements
    look comparable.
    """
    if point.get("kind") == "release":
        return history_runner
    return point.get("runner", "")


def comparable_metric_values(current, points, key, provenance_keys):
    """Values for `key` whose complete provenance matches `current`.

    Missing provenance is never evidence of equality. In particular, two
    legacy points that both predate a marker must not match merely because
    their dictionary lookups both return None.
    """
    current_provenance = tuple(current["metrics"].get(k) for k in provenance_keys)
    if any(value is None for value in current_provenance):
        return []

    values = []
    for point in points:
        if key not in point.get("metrics", {}):
            continue
        point_provenance = tuple(point["metrics"].get(k) for k in provenance_keys)
        if (all(value is not None for value in point_provenance)
                and point_provenance == current_provenance):
            values.append(point["metrics"][key])
    return values


def main():
    # The Windows runner's Python defaults to a cp1252 console encoding, which
    # cannot encode this report's non-ASCII table headers — and the flag table
    # only prints when a regression IS found, so an encoding crash would mask
    # exactly the output that matters. Force UTF-8 (errors="replace" so a
    # future exotic character degrades instead of raising).
    for stream in (sys.stdout, sys.stderr):
        if hasattr(stream, "reconfigure"):
            stream.reconfigure(encoding="utf-8", errors="replace")

    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--results", default=os.path.join(HERE, "results"))
    # Threshold rationale:
    # --rel 1.10: flag a 10% rise vs trailing median. The runner is a dedicated
    #   quiesced machine with 5-sample medians, giving a noise floor of ~1-3%,
    #   so 10% catches real medium regressions while avoiding noise false positives.
    #   The --abs guard (2 ms) prevents alerting on tiny absolute deltas even when
    #   the relative ratio exceeds 10%.
    # --abs 2.0: ignore sub-2 ms absolute deltas regardless of ratio — a 50%
    #   rise in a 3 ms timer is within measurement noise, not a real regression.
    # --window 7: trailing-7-point median spans ~one week of nightly runs,
    #   long enough to be stable against one bad night but short enough to track
    #   genuine drift (a regression from 3 weeks ago is already known).
    # --min-baseline 3: require at least 3 prior same-runner points before judging,
    #   so the first few nights after a new runner don't produce false positives.
    ap.add_argument("--window", type=int, default=7, help="trailing points for the median")
    ap.add_argument("--rel", type=float, default=1.10,
                    help="relative threshold for an ERROR (fails the nightly)")
    ap.add_argument("--warn-rel", type=float, default=1.05,
                    help="relative threshold for a WARNING (reported, does not fail)")
    ap.add_argument("--abs", type=float, default=2.0, help="min absolute ms delta to flag")
    ap.add_argument("--min-baseline", type=int, default=3,
                    help="min trailing points required to judge a metric")
    ap.add_argument("--baseline-kind", choices=["daily", "any"], default="daily",
                    help="which points form the trailing baseline (default daily: "
                         "release points carry a build-provenance offset)")
    ap.add_argument("--no-fail", action="store_true", help="report only; always exit 0")
    # Daily labels are keyed by the SWEPT COMMIT's date, so several points can
    # share one date (e.g. master's HEAD was committed yesterday, or a manual
    # backfill re-measured a day). The series sort order then makes pts[-1]
    # ambiguous — observed 2026-07-08, when the nightly judged a same-date
    # sibling instead of the point it had just registered. The nightly passes
    # the label it registered so the right point is judged unconditionally.
    ap.add_argument("--label", default=None,
                    help="judge the point with this label instead of the last point")
    args = ap.parse_args()

    # argparse cannot express a relation between two options, so the gate's
    # central precondition is checked here. See check_threshold_order.
    check_threshold_order(args.rel, args.warn_rel)

    tpath = os.path.join(args.results, "tracking", "tracking.json")
    if not os.path.exists(tpath):
        abort(f"no tracking series at {tpath}; run track.py rebuild first")
    series = analyze.read_json(tpath)
    pts = series.get("points", [])
    if len(pts) < 2:
        print("not enough points to trend (need >= 2)")
        return

    hist_runner = series.get("runner", "")
    if args.label:
        cur_idx = next((i for i, p in enumerate(pts) if p.get("label") == args.label), None)
        if cur_idx is None:
            abort(f"--label {args.label}: no such point in the tracking series "
                  f"(was track.py register run for it?)")
    else:
        cur_idx = len(pts) - 1
    current = pts[cur_idx]
    earlier = pts[:cur_idx]
    cur_runner = point_runner(current, hist_runner)

    print(f"trend: current={current['label']} ({current['date']}, {current['kind']})  "
          f"runner={cur_runner or 'unset'}")

    # Baseline defaults to DAILY points only: release points are official
    # prebuilt binaries while dailies are runner-built with matched flags but a
    # different MSVC toolset — a build-provenance offset (uniform few-%, and
    # 30%+ on single hot loops) that is not a code regression. Judging tonight
    # against recent nights keeps the baseline provenance-consistent; use
    # --baseline-kind any for ad-hoc cross-kind comparisons.
    #
    # In that default mode the candidates come from daily/ on disk rather than
    # from the tracking series, because the series deliberately keeps only the
    # dailies dated after the last release (`assemble`). That truncation is the
    # right shape for the rendered line — release history, then the current tail
    # — but reusing it as the ALERTING baseline means every release erases the
    # comparable nights alerting depends on: with --min-baseline 3, the three
    # nights after a release judge nothing at all. v2026.18 shipped 2026-09-15
    # and the 09-16/09-17/09-18 nightlies each printed "only N comparable
    # trailing point(s) (need 3); skipping trend judgement" and passed green —
    # which is how #13008's regression went unreported on the night it landed.
    # The points are on disk the whole time; only the series had dropped them.
    #
    # --baseline-kind any still reads the series, since mixing kinds is an
    # explicit ad-hoc request and the series is the only place release points
    # carry comparable metrics.
    def _order(p):
        """Series order: date, then the commit's full timestamp for same-date
        siblings, then the label as a deterministic last resort."""
        return (p["date"], p.get("commit_time") or "", p["label"])

    if args.baseline_kind == "daily":
        # The UNION of the series' daily points and every daily sweep on disk,
        # deduped by label. The on-disk point wins a collision because it is
        # reconstructed through point_metrics() and therefore carries the
        # current provenance markers; tracking.json may have been written by an
        # older tool and lack them. The series remains the fallback when daily/
        # is absent.
        by_label = {p["label"]: p for p in earlier if p.get("kind") == "daily"}
        for p in analyze.daily_series_points(args.results):
            by_label[p["label"]] = p
        cur_order = _order(current)
        candidates = sorted((p for p in by_label.values() if _order(p) < cur_order),
                            key=_order)
    else:
        candidates = list(earlier)

    # Restrict the baseline to points on the same known runner. A legacy daily
    # point with no runner is unknown provenance, not a release point that can
    # inherit the series-level release-sweep runner.
    prior = [p for p in candidates
             if cur_runner and point_runner(p, hist_runner) == cur_runner]

    window = prior[-args.window:]

    if hist_runner and cur_runner and cur_runner != hist_runner:
        msg = (f"latest point ran on runner '{cur_runner}' but the release history "
               f"was built on '{hist_runner}'. Comparing against same-runner points "
               f"only; re-run perf-compile-release-sweep (force=true) to resync the "
               f"history to this machine.")
        print(f"WARNING: {msg}")
        emit_gha_command(f"::warning title=Perf runner mismatch::{msg}")

    if len(window) < args.min_baseline:
        msg = (f"only {len(window)} comparable trailing point(s) "
               f"(need {args.min_baseline}); skipping trend judgement.")
        print(msg)
        emit_gha_command(f"::warning title=Perf trend::{msg}")
        return

    base_labels = f"{window[0]['label']}..{window[-1]['label']}"
    regressions = []
    warnings = []
    provenance_skipped = set()
    for key, cur in sorted(current.get("metrics", {}).items()):
        wl, _, counter = key.partition("|")
        if not judged(wl, counter):
            continue
        # Two more provenance axes, alongside the runner fingerprint and the
        # point kind that the window was already filtered on. Both are checked
        # HERE rather than on the window because both vary PER WORKLOAD within a
        # single point, so dropping whole points would discard good baselines to
        # repair bad ones:
        #
        #   timer schema — decides how the compiler ATTRIBUTES time, not merely
        #     how many counters it reports. The same unchanged compile read
        #     `specializeModule` at 16.4 ms coarse and 33.1 ms detailed, with
        #     `wall_ms` unmoved and the sub-timers summing past their parent.
        #     api-mode workloads are permanently coarse (their driver takes no
        #     such flag) while target/module/link went detailed in #13009, so one
        #     point legitimately holds both.
        #
        #   workload size — canonical_runs() falls back to an off-default size
        #     when no default-size row exists, so a point swept before a resize
        #     publishes the SAME metric key measured at the old size.
        #
        # Both were live on the 2026-09-20 nightly, which flagged 25 regressions
        # against a commit identical to the night before: 7 from the schema
        # change, 12 from #13035's resizes. Neither is a code change.
        #
        # An absent marker (data predating the field) counts as NOT matching
        # rather than as a wildcard — the same refusal the runner check makes.
        # Admitting unknown provenance risks a false alert; excluding it costs a
        # few nights of reduced coverage while the window refills.
        prov_keys = (f"{wl}|{analyze.SCHEMA_MARKER}", f"{wl}|{analyze.SIZE_MARKER}")
        present = [p for p in window if key in p.get("metrics", {})]
        baseline = comparable_metric_values(current, present, key, prov_keys)
        if len(baseline) < args.min_baseline:
            # Only counts as provenance-skipped when the counter WAS present in
            # enough trailing points and the provenance filter is what removed
            # them — otherwise this is the ordinary "not enough history yet"
            # case (a new workload), which is not worth reporting.
            if len(present) >= args.min_baseline:
                provenance_skipped.add(wl)
            continue
        med = statistics.median(baseline)
        if med <= 0:
            continue
        ratio = cur / med
        delta = cur - med
        # The floor is PER COUNTER, not the flat --abs: this series carries
        # kb-unit memory counters alongside ms timers, and a few-KB wobble on
        # a ~200 MB value must not page anyone. classify_metric takes the
        # floor as a parameter precisely so it can be substituted here, which
        # is what keeps the two-tier gate and the unit-aware floor composable
        # rather than one overwriting the other.
        verdict = classify_metric(ratio, delta, args.rel, args.warn_rel,
                                  abs_floor_for(counter, args.abs))
        if verdict == "error":
            regressions.append((wl, counter, med, cur, ratio, delta))
        elif verdict == "warning":
            warnings.append((wl, counter, med, cur, ratio, delta))

    # Surfaced rather than silent: a workload dropping out of judgement looks
    # identical to a workload that passed, and the whole point of the schema
    # filter is that it trades coverage for correctness — the reader has to be
    # able to see which side of that trade a given run landed on.
    if provenance_skipped:
        shown = sorted(provenance_skipped)
        listed = ", ".join(shown[:6]) + (f", +{len(shown) - 6} more" if len(shown) > 6 else "")
        msg = (f"{len(shown)} workload(s) not judged: their trailing points were "
               f"measured under a different timer schema or at a different size "
               f"({listed}). A re-attributed or resized counter is not a "
               f"regression; judgement resumes once the window refills with "
               f"comparable points.")
        print(f"WARNING: {msg}")
        emit_gha_command(f"::warning title=Perf timer schema::{msg}")

    regressions.sort(key=lambda r: -r[4])
    warnings.sort(key=lambda r: -r[4])

    # The absolute floor is quoted per unit, and read back out of
    # abs_floor_for rather than restated: the memory floor is not --abs, so a
    # single "{args.abs} ms" would misreport the gate every memory counter is
    # actually judged against.
    mem_floor = analyze.fmt_qty("peakRssKb", abs_floor_for("peakRssKb", args.abs))
    print(f"baseline: trailing {len(window)} point(s) [{base_labels}], "
          f"median per metric; ERROR at ratio >= {args.rel}, WARNING at "
          f">= {args.warn_rel}, both gated on an absolute delta of "
          f">= {args.abs} ms for timers / {mem_floor} for memory counters\n")

    # `warnings` is the ONLY key the workflow reads, and the only one it
    # needs: the Slack step distinguishes a warnings-only night from a clean
    # one, which the exit code alone cannot do since warnings deliberately do
    # not fail the job. A regression is already carried by the exit code
    # (EXIT_REGRESSION), so an `errors` key would be a second
    # spelling of the same fact — one that no reader would notice going stale.
    #
    # This write must stay AHEAD of every path that leaves main() below — the
    # clean-night `return` and the regression `SystemExit(EXIT_REGRESSION)`.
    # Both are exits
    # the workflow still reads the output on, and an unwritten key falls back
    # to the step's `|| '0'`, which reports a warnings-only night as clean:
    # the one state this key exists to distinguish. Classify, emit, then
    # report — do not move reporting logic above this block.
    out = os.environ.get("GITHUB_OUTPUT")
    if out:
        with open(out, "a", encoding="utf-8") as fh:
            fh.write(f"warnings={len(warnings)}\n")

    if not regressions and not warnings:
        print(f"OK — no compile-perf regression in {current['label']} vs trailing median.")
        write_step_summary(f"### Compile-perf trend — {current['label']}\n\n"
                f"OK — no regression vs trailing {len(window)}-point median "
                f"(`{base_labels}`).")
        return

    # Two tables (error tier then warning tier) with the header reflecting the
    # worse of the two — from the two-tier gate; every VALUE rendered through
    # analyze.fmt_qty — from memory tracking. Both halves are needed: this
    # series now carries kb counters as well as ms timers, so the hard-coded
    # "ms" formatting the two-tier tables originally used would print a
    # 200 MB peak as "204800.0 ms". The column headers say "median"/"Δ"
    # without a unit for the same reason — fmt_qty puts the unit on each value.
    print(f"{'workload':20s}{'counter':26s}{'median':>12}{'current':>12}{'ratio':>8}{'Δ':>12}")
    rows = [f"### {'🔴' if regressions else '⚠️'} Compile-perf trend — " + current["label"],
            f"\nvs trailing {len(window)}-point median (`{base_labels}`), "
            f"runner `{cur_runner}`. ERROR ≥ {args.rel}×, WARNING ≥ {args.warn_rel}×.\n"]

    def table(items, kind, gha):
        rows.append(f"\n**{kind}** ({len(items)}):\n")
        rows.append("| workload | counter | median | current | ratio | Δ |")
        rows.append("|---|---|--:|--:|--:|--:|")
        for wl, counter, med, cur, ratio, delta in items:
            print(f"{wl:20s}{counter:26s}{analyze.fmt_qty(counter, med):>12s}"
                  f"{analyze.fmt_qty(counter, cur):>12s}{ratio:7.2f}x"
                  f"{analyze.fmt_qty(counter, delta, signed=True):>12s}")
            emit_gha_command(
                f"::{gha} title=Perf {kind.lower()} {wl}/{counter}::"
                f"{ratio:.2f}x ({analyze.fmt_qty(counter, med)} -> "
                f"{analyze.fmt_qty(counter, cur)}, "
                f"{analyze.fmt_qty(counter, delta, signed=True)}) vs trailing median")
            rows.append(f"| {wl} | {counter} | {analyze.fmt_qty(counter, med)} | "
                        f"{analyze.fmt_qty(counter, cur)} | {ratio:.2f}× | "
                        f"{analyze.fmt_qty(counter, delta, signed=True)} |")

    if regressions:
        table(regressions, "Regressions", "error")
    if warnings:
        table(warnings, "Warnings", "warning")
    write_step_summary("\n".join(rows))

    print(f"\n{len(regressions)} regression(s), {len(warnings)} warning(s) flagged.")
    if regressions and not args.no_fail:
        raise SystemExit(EXIT_REGRESSION)


# Import-time self-checks (the directory idiom): judged() and the per-unit
# absolute floor ARE the memory alert path — if either regressed, memory
# alerting would silently never fire.
assert judged("minimal", "peakRssKb"), "kb counters must always be judged"
assert judged("minimal", "compileInner"), "compileInner is always judged"
assert not judged("minimal", "emitEntryPointsSourceFromIR"), \
    "non-primary ms timers are not judged for workloads that do not list them"
assert abs_floor_for("peakRssKb", 2.0) == 1024.0, "memory floor is 1 MiB"
assert abs_floor_for("compileInner", 2.0) == 2.0, "time floor is --abs"

# Runner provenance differs by point kind: only releases inherit the runner
# recorded for the release history. A daily point with no runner is legacy data
# from an unknown machine and must stay unknown.
assert point_runner({"kind": "release"}, "r1") == "r1"
assert point_runner({"kind": "daily"}, "r1") == ""
assert point_runner({"kind": "daily", "runner": "r2"}, "r1") == "r2"

# Complete, equal per-workload provenance admits a metric. Missing either
# marker on either side admits nothing — most importantly, two missing values
# do not become a false match through None == None.
_PROV_KEYS = (f"minimal|{analyze.SCHEMA_MARKER}",
              f"minimal|{analyze.SIZE_MARKER}")
_KNOWN_METRICS = {"minimal|compileInner": 100.0,
                  _PROV_KEYS[0]: 1.0, _PROV_KEYS[1]: 64.0}
_CURRENT = {"metrics": dict(_KNOWN_METRICS)}
assert comparable_metric_values(
    _CURRENT, [{"metrics": dict(_KNOWN_METRICS)}],
    "minimal|compileInner", _PROV_KEYS) == [100.0]
_MISMATCHED_METRICS = dict(_KNOWN_METRICS)
_MISMATCHED_METRICS[_PROV_KEYS[1]] = 128.0
assert comparable_metric_values(
    _CURRENT, [{"metrics": _MISMATCHED_METRICS}],
    "minimal|compileInner", _PROV_KEYS) == [], \
    "complete but unequal provenance must not enter the baseline"
for _missing in _PROV_KEYS:
    _legacy_current = {"metrics": dict(_KNOWN_METRICS)}
    del _legacy_current["metrics"][_missing]
    assert comparable_metric_values(
        _legacy_current, [{"metrics": dict(_KNOWN_METRICS)}],
        "minimal|compileInner", _PROV_KEYS) == []
    _legacy_point = {"metrics": dict(_KNOWN_METRICS)}
    del _legacy_point["metrics"][_missing]
    assert comparable_metric_values(
        _CURRENT, [_legacy_point], "minimal|compileInner", _PROV_KEYS) == []
del _PROV_KEYS, _KNOWN_METRICS, _MISMATCHED_METRICS, _CURRENT
del _missing, _legacy_current, _legacy_point

# The two gates compose, and that composition is what the merge of the
# two-tier gate and the unit-aware floor had to get right: a kb counter must
# clear 1 MiB before EITHER tier fires, so a few-KB wobble is neither an error
# nor a warning, while the same numeric delta in ms clears the 2 ms floor.
assert classify_metric(1.20, 500.0, 1.10, 1.05,
                       abs_floor_for("peakRssKb", 2.0)) is None, \
    "a 500 KB move must not alert: it is below the 1 MiB memory floor"
assert classify_metric(1.20, 2048.0, 1.10, 1.05,
                       abs_floor_for("peakRssKb", 2.0)) == "error", \
    "a 2 MiB move at 1.20x is over both the memory floor and the error ratio"
assert classify_metric(1.07, 2048.0, 1.10, 1.05,
                       abs_floor_for("peakRssKb", 2.0)) == "warning", \
    "a 2 MiB move at 1.07x lands in the warning tier, not silence"
assert classify_metric(1.20, 500.0, 1.10, 1.05,
                       abs_floor_for("compileInner", 2.0)) == "error", \
    "the SAME delta in ms is far over the 2 ms time floor — unit picks the gate"


# Import-time self-checks over classify_metric, matching the directory idiom
# (lib/manifest.py, daily_movers.py) and run by check-python-core.yml on every
# PR that touches these files. This is the decision the two-tier gate exists
# to make, and each branch's failure mode is quiet: a mis-ordered comparison
# turns a warning into a nightly failure (alert fatigue) or an error into a
# yellow icon nobody acts on (missed regression). Written against the shipped
# defaults so the cases read as the real bands.
_REL, _WARN, _ABS = 1.10, 1.05, 2.0

# The three bands, comfortably inside each.
assert classify_metric(1.20, 50.0, _REL, _WARN, _ABS) == "error"
assert classify_metric(1.07, 50.0, _REL, _WARN, _ABS) == "warning"
assert classify_metric(1.02, 50.0, _REL, _WARN, _ABS) is None

# Both boundaries are inclusive, and `rel` belongs to the ERROR band: --rel is
# documented as the ratio at which the nightly fails, so a metric sitting
# exactly on it must fail rather than warn.
assert classify_metric(_REL, 50.0, _REL, _WARN, _ABS) == "error", \
    "ratio == rel is an error, not a warning"
assert classify_metric(_WARN, 50.0, _REL, _WARN, _ABS) == "warning", \
    "ratio == warn_rel is a warning, not silence"

# The absolute floor gates BOTH tiers. A huge ratio on a sub-floor delta is
# noise in a small timer, not a regression — and it must not leak into the
# warning band either, which is the easy mistake when adding a second tier.
assert classify_metric(1.50, _ABS - 0.1, _REL, _WARN, _ABS) is None, \
    "abs floor must gate the error tier"
assert classify_metric(1.07, _ABS - 0.1, _REL, _WARN, _ABS) is None, \
    "abs floor must gate the WARNING tier too, not just the error tier"
assert classify_metric(1.50, _ABS, _REL, _WARN, _ABS) == "error", \
    "delta == abs is above the floor (the floor is exclusive-below)"

# A drop is never flagged: delta is negative, so it fails the floor first.
assert classify_metric(0.5, -50.0, _REL, _WARN, _ABS) is None, \
    "an improvement must never be classified as a regression"

# The threshold-order guard, exercised through the real function rather than
# a restatement of its condition. The shipped defaults must pass; an EQUAL
# pair must be rejected as firmly as an inverted one, since it erases the
# warning tier just as completely — that is the >=-vs-> boundary, and it is
# the one-character regression this block exists to catch.
check_threshold_order(_REL, _WARN)
for _bad in ((_REL, _REL), (_WARN, _REL)):
    # stderr is captured: abort() reports before it raises, and letting the
    # fixture through would put its message on every run that merely imports
    # this module. The capture is also the assertion — a silent abort would
    # leave the operator with a bare exit code and no reason.
    _buf = __import__("io").StringIO()
    try:
        with __import__("contextlib").redirect_stderr(_buf):
            check_threshold_order(*_bad)
    except SystemExit as _e:
        assert _e.code == EXIT_CANNOT_EVALUATE, \
            (f"check_threshold_order{_bad} must abort with "
             f"EXIT_CANNOT_EVALUATE, not the regression code: a bad threshold "
             f"pair is an operator error, and Slack reports the regression "
             f"code as a >=10% regression")
        assert "--warn-rel" in _buf.getvalue(), \
            "an aborted run must say why, not just exit non-zero"
    else:
        raise AssertionError(f"check_threshold_order{_bad} must be rejected")
# `_e` is not in the del list: `except ... as _e` unbinds it at the end of the
# except block, so naming it here is a NameError, not tidiness.
del _REL, _WARN, _ABS, _bad, _buf


def _warnings_output_selfcheck():
    """Pin the ONE invariant that only running main() can establish: the
    `warnings=` GITHUB_OUTPUT write happens on every path out of main() that
    FOLLOWS classification.

    The qualifier is load-bearing, not hedging. Two returns leave main()
    before any judgement happens — too few points to trend at all, and too few
    same-runner trailing points to meet --min-baseline — and neither writes
    the key. That is correct: no warnings have been computed at that stage, so
    there is no count to report, and the workflow's
    `${{ steps.trend.outputs.warnings || '0' }}` fallback yields 0. Those two
    paths are deliberately outside this fixture's scope. (Separately, they do
    leave the Slack step reporting a green "No regressions detected" on a
    night where nothing was judged — a real state on a fresh runner — but that
    is a missing Slack state, not a missing write.)

    The classifier checks above are pure and cannot see this. The write is the
    sole signal separating a warnings-only night from a clean one — warnings
    deliberately do not fail the job — so if a future edit moved it below the
    clean-night `return` or the regression `SystemExit(EXIT_REGRESSION)`,
    that fallback
    would report a warnings-only night as a green "No regressions detected".
    Nothing else would notice.

    Driving main() in-process rather than as a subprocess keeps this an
    ordinary import-time check (no process spawn), but it means argv, the
    three GITHUB_* variables, and stdout all have to be borrowed and given
    back — GITHUB_STEP_SUMMARY especially, since leaving the real one set
    would append fixture output to the actual CI job summary."""
    import contextlib
    import io
    import shutil
    import tempfile

    def point(label, date, metrics):
        metrics = dict(metrics)
        workloads = {key.partition("|")[0] for key in metrics}
        for workload in workloads:
            metrics[f"{workload}|{analyze.SCHEMA_MARKER}"] = 1.0
            metrics[f"{workload}|{analyze.SIZE_MARKER}"] = 64.0
        return {"label": label, "date": date, "kind": "daily",
                "runner": "r1", "metrics": metrics}

    BASE = 100.0
    # Two metrics per point so the last case can put one in each tier at once;
    # five stable trailing points clears --window/--min-baseline comfortably.
    history = [point(f"2026-01-0{i}-aaaaaaaaa", f"2026-01-0{i}",
                     {"minimal|compileInner": BASE, "parse|compileInner": BASE})
               for i in range(1, 6)]
    # (metrics, exit code, expected GITHUB_OUTPUT line, expected summary header)
    # — one case per path out of main() after classification, plus the mixed
    # case, which is the only one that renders both tables and so is the only
    # one that can pin the header's choice between them.
    cases = [
        # clean: the write still happens, ahead of the early return
        ({"minimal|compileInner": BASE, "parse|compileInner": BASE},
         0, "warnings=0", None),
        # warning band only: reported, does not fail the job
        ({"minimal|compileInner": BASE * 1.07, "parse|compileInner": BASE},
         0, "warnings=1", "⚠️"),
        # regression only: the write precedes SystemExit(EXIT_REGRESSION)
        ({"minimal|compileInner": BASE * 1.15, "parse|compileInner": BASE},
         EXIT_REGRESSION, "warnings=0", "🔴"),
        # both tiers at once: a regression outranks a warning in the header,
        # and the warning is still counted rather than swallowed by it
        ({"minimal|compileInner": BASE * 1.15, "parse|compileInner": BASE * 1.07},
         EXIT_REGRESSION, "warnings=1", "🔴"),
    ]

    d = tempfile.mkdtemp(prefix="trend_selfcheck_")
    saved_argv = sys.argv
    saved_env = {k: os.environ.get(k)
                 for k in ("GITHUB_OUTPUT", "GITHUB_STEP_SUMMARY", "GITHUB_ACTIONS")}
    try:
        os.makedirs(os.path.join(d, "tracking"))
        tpath = os.path.join(d, "tracking", "tracking.json")
        gho = os.path.join(d, "github_output")
        summary = os.path.join(d, "step_summary")
        # Both are redirected into the fixture's tmpdir rather than left
        # pointing at the real ones: GITHUB_STEP_SUMMARY would otherwise
        # append fixture tables to the actual CI job summary, and it is read
        # back below to check which header the run chose. GITHUB_ACTIONS is
        # unset because ::warning:: annotations are noise outside a real run.
        os.environ.pop("GITHUB_ACTIONS", None)
        os.environ["GITHUB_OUTPUT"] = gho
        os.environ["GITHUB_STEP_SUMMARY"] = summary

        for metrics, want_code, want_line, want_header in cases:
            with analyze.open_output(tpath) as fh:
                json.dump({"runner": "r1",
                           "points": history + [point("2026-01-09-zzzzzzzzz",
                                                      "2026-01-09", metrics)]},
                          fh)
            for f in (gho, summary):
                with analyze.open_output(f) as fh:
                    fh.write("")
            sys.argv = ["trend.py", "--results", d, "--label", "2026-01-09-zzzzzzzzz"]
            code = 0
            with contextlib.redirect_stdout(io.StringIO()):
                try:
                    main()
                except SystemExit as e:
                    code = e.code or 0
            written = analyze.read_text(gho)
            assert code == want_code, \
                (f"trend fixture: {metrics} expected exit {want_code}, got {code}"
                 f" — the warning tier must not fail the job, and a regression must")
            assert want_line in written, \
                (f"trend fixture: {metrics} expected '{want_line}' in GITHUB_OUTPUT,"
                 f" got {written!r} — the warnings= write must precede every exit "
                 f"from main() that follows classification, or a warnings-only "
                 f"night reports as clean")
            if want_header:
                md = analyze.read_text(summary)
                assert want_header in md.splitlines()[0], \
                    (f"trend fixture: {metrics} expected the step summary to lead "
                     f"with {want_header}, got {md.splitlines()[0]!r} — a run with "
                     f"any regression must not be headed by the warning icon")

        # A real provenance transition: minimal is present throughout the
        # trailing window but changes size tonight, while newcomer has no
        # history at all. Only minimal is a provenance skip, it must not be
        # classified as a regression, and Actions must receive the warning.
        current_metrics = {
            "minimal|compileInner": BASE * 1.20,
            "parse|compileInner": BASE,
            "newcomer|compileInner": BASE * 1.20,
        }
        current = point("2026-01-09-zzzzzzzzz", "2026-01-09", current_metrics)
        current["metrics"][f"minimal|{analyze.SIZE_MARKER}"] = 128.0
        with analyze.open_output(tpath) as fh:
            json.dump({"runner": "r1", "points": history + [current]}, fh)
        for f in (gho, summary):
            with analyze.open_output(f) as fh:
                fh.write("")
        os.environ["GITHUB_ACTIONS"] = "true"
        sys.argv = ["trend.py", "--results", d, "--label", current["label"]]
        stdout = io.StringIO()
        code = 0
        with contextlib.redirect_stdout(stdout):
            try:
                main()
            except SystemExit as e:
                code = e.code or 0
        report = stdout.getvalue()
        assert code == 0 and "OK — no compile-perf regression" in report, \
            "a size transition must suppress comparison, not become a regression"
        assert "Perf timer schema" in report and "(minimal)" in report, \
            "a provenance skip must emit an Actions warning naming the workload"
        assert "newcomer" not in report, \
            "a new workload with too little history is not a provenance skip"
    finally:
        sys.argv = saved_argv
        for k, v in saved_env.items():
            if v is None:
                os.environ.pop(k, None)
            else:
                os.environ[k] = v
        shutil.rmtree(d, ignore_errors=True)


_warnings_output_selfcheck()
del _warnings_output_selfcheck


def _daily_baseline_selfcheck():
    """Exercise the on-disk daily union, collision rule, and current cutoff.

    The tracking series intentionally contains only one stale baseline point;
    daily/ contributes the two pre-release nights the rendered series dropped
    and replaces the stale shared label with a provenance-bearing point. Two
    future points ensure the current-point cutoff is also load-bearing.
    """
    import contextlib
    import io
    import shutil
    import tempfile

    d = tempfile.mkdtemp(prefix="trend_daily_baseline_selfcheck_")
    saved_argv = sys.argv
    saved_env = {k: os.environ.get(k)
                 for k in ("GITHUB_OUTPUT", "GITHUB_STEP_SUMMARY", "GITHUB_ACTIONS")}
    try:
        os.makedirs(os.path.join(d, "tracking"))
        shared_label = "2026-01-03-ccccccccc"
        current_label = "2026-01-04-ddddddddd"
        stale_shared = {
            "label": shared_label, "date": "2026-01-03", "kind": "daily",
            "runner": "r1", "metrics": {"minimal|compileInner": 100.0},
        }
        current = {
            "label": current_label, "date": "2026-01-04", "kind": "daily",
            "runner": "r1", "metrics": {
                "minimal|compileInner": 120.0,
                f"minimal|{analyze.SCHEMA_MARKER}": 1.0,
                f"minimal|{analyze.SIZE_MARKER}": 64.0,
            },
        }
        with analyze.open_output(os.path.join(d, "tracking", "tracking.json")) as fh:
            json.dump({"runner": "r1", "points": [stale_shared, current]}, fh)

        def write_daily(label, date, value):
            path = os.path.join(d, "daily", label)
            os.makedirs(path)
            with analyze.open_output(os.path.join(path, "results.json")) as fh:
                json.dump([{
                    "workload": "minimal", "size": 64,
                    "timer_schema": "detailed",
                    "timers": {"compileInner": {"median": value}},
                }], fh)
            with analyze.open_output(os.path.join(path, "meta.json")) as fh:
                json.dump({"date": date, "commit": label[-9:],
                           "commit_time": f"{date}T00:00:00Z", "runner": "r1"}, fh)

        for label, date, value in (
                ("2026-01-01-aaaaaaaaa", "2026-01-01", 100.0),
                ("2026-01-02-bbbbbbbbb", "2026-01-02", 100.0),
                (shared_label, "2026-01-03", 100.0),
                ("2026-01-05-eeeeeeeee", "2026-01-05", 1000.0),
                ("2026-01-06-fffffffff", "2026-01-06", 1000.0)):
            write_daily(label, date, value)

        for key in saved_env:
            os.environ.pop(key, None)
        sys.argv = ["trend.py", "--results", d, "--label", current_label,
                    "--window", "3"]
        stdout = io.StringIO()
        code = 0
        with contextlib.redirect_stdout(stdout):
            try:
                main()
            except SystemExit as e:
                code = e.code or 0
        report = stdout.getvalue()
        assert code == EXIT_REGRESSION and "1 regression(s)" in report, \
            "three restored on-disk baselines must make the 1.20x rise judgeable"
        assert "trailing 3 point(s)" in report, \
            "the shared label must be deduplicated and both omitted nights restored"
        assert "2026-01-01-aaaaaaaaa..2026-01-03-ccccccccc" in report, \
            "future on-disk points must not cross the current-point cutoff"
    finally:
        sys.argv = saved_argv
        for key, value in saved_env.items():
            if value is None:
                os.environ.pop(key, None)
            else:
                os.environ[key] = value
        shutil.rmtree(d, ignore_errors=True)


_daily_baseline_selfcheck()
del _daily_baseline_selfcheck


def _abort_exit_code_selfcheck():
    """Drive main()'s "cannot judge" exits and pin their code and diagnostic.

    These are the two aborts behind the false-alert incident, and neither is
    reachable from the pure classifier checks above — main() has to run for
    them at all. The exit CODE is the assertion that matters: swapping either
    abort() back to `raise SystemExit(f"...")` reproduces the message verbatim
    while silently returning the code to 1, and nothing else here would see
    it.
    """
    import contextlib
    import io
    import shutil
    import tempfile

    d = tempfile.mkdtemp(prefix="trend_abort_selfcheck_")
    saved_argv = sys.argv
    saved_actions = os.environ.get("GITHUB_ACTIONS")
    try:
        os.environ.pop("GITHUB_ACTIONS", None)
        os.makedirs(os.path.join(d, "tracking"))
        # Enough points that the missing-label case reaches the label lookup
        # and aborts THERE, rather than at the earlier "not enough points"
        # return, which is a clean exit and a different path entirely.
        with analyze.open_output(os.path.join(d, "tracking", "tracking.json")) as fh:
            json.dump({"runner": "r1",
                       "points": [{"label": f"2026-01-0{i}-aaaaaaaaa",
                                   "date": f"2026-01-0{i}", "kind": "daily",
                                   "runner": "r1",
                                   "metrics": {"minimal|compileInner": 100.0}}
                                  for i in range(1, 6)]}, fh)

        # (results dir, label, expected fragment of the diagnostic)
        for results, label, want in (
                (os.path.join(d, "no-such-dir"), "2026-01-09-zzzzzzzzz",
                 "no tracking series"),
                (d, "2026-01-09-not-a-real-label", "no such point")):
            sys.argv = ["trend.py", "--results", results, "--label", label]
            err, code = io.StringIO(), 0
            try:
                with contextlib.redirect_stdout(io.StringIO()), \
                        contextlib.redirect_stderr(err):
                    main()
            except SystemExit as e:
                code = e.code
            assert code == EXIT_CANNOT_EVALUATE, \
                (f"trend abort fixture: --label {label} exited {code!r}, "
                 f"expected EXIT_CANNOT_EVALUATE ({EXIT_CANNOT_EVALUATE}) — a "
                 f"run that judged nothing must not exit with a code the Slack "
                 f"step could read as a measured regression")
            assert want in err.getvalue(), \
                (f"trend abort fixture: --label {label} must say why it gave "
                 f"up; got {err.getvalue()!r}")
    finally:
        sys.argv = saved_argv
        if saved_actions is None:
            os.environ.pop("GITHUB_ACTIONS", None)
        else:
            os.environ["GITHUB_ACTIONS"] = saved_actions
        shutil.rmtree(d, ignore_errors=True)


_abort_exit_code_selfcheck()
del _abort_exit_code_selfcheck


if __name__ == "__main__":
    main()
