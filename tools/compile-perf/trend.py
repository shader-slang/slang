#!/usr/bin/env python3
"""Drift alert for the nightly compile-perf tracking series.

Reads the tracking series (track.py's tracking/tracking.json) and compares the
latest point — tonight's tip-of-tree (ToT) sweep — against the trailing median of
the previous N points, per (workload, primary-timer). A metric that rises beyond
both a relative and an absolute threshold is flagged: printed, emitted as a
GitHub Actions ::error:: annotation + step-summary row, and (unless --no-fail)
the process exits non-zero so the nightly job goes red. This catches the gradual
drift a per-PR step gate structurally misses.

Absolute compile times are runner-specific, so comparisons are restricted to
points sharing the current point's runner fingerprint. If the latest point ran on
a different runner than the release history was built on, the history is stale for
this machine — re-run compile-perf-release-sweep (force=true); trend.py warns and
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


def timers_for(workload):
    """The timers worth alerting on for a workload: its manifest primary timers,
    always including compileInner (the holistic signal)."""
    spec = manifest.BY_NAME.get(workload)
    timers = set(spec.primary_timers) if spec else set()
    timers.add("compileInner")
    return timers


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
        raise SystemExit(f"--warn-rel ({warn_rel}) must be < --rel ({rel}): "
                         f"the warning band sits below the error band, and a "
                         f"pair that is equal or inverted silently promotes "
                         f"every warning-level change to an error")


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
        raise SystemExit(f"no tracking series at {tpath}; run track.py rebuild first")
    series = analyze.read_json(tpath)
    pts = series.get("points", [])
    if len(pts) < 2:
        print("not enough points to trend (need >= 2)")
        return

    hist_runner = series.get("runner", "")
    if args.label:
        cur_idx = next((i for i, p in enumerate(pts) if p.get("label") == args.label), None)
        if cur_idx is None:
            raise SystemExit(f"--label {args.label}: no such point in the tracking series "
                             f"(was track.py register run for it?)")
    else:
        cur_idx = len(pts) - 1
    current = pts[cur_idx]
    earlier = pts[:cur_idx]
    # Release points carry no per-point runner field by design: they are all built
    # by the release-sweep job on the machine recorded in runner.json (hist_runner).
    # The `or hist_runner` below is not defensive fallback — it is that data-model
    # invariant: a missing runner field means "this is a release point, use hist_runner".
    cur_runner = current.get("runner") or hist_runner

    print(f"trend: current={current['label']} ({current['date']}, {current['kind']})  "
          f"runner={cur_runner or 'unset'}")

    # Restrict the baseline to points on the same runner, strictly before the
    # judged point in series order.
    prior = [p for p in earlier if (p.get("runner") or hist_runner) == cur_runner]
    # Baseline defaults to DAILY points only: release points are official
    # prebuilt binaries while dailies are runner-built with matched flags but a
    # different MSVC toolset — a build-provenance offset (uniform few-%, and
    # 30%+ on single hot loops) that is not a code regression. Judging tonight
    # against recent nights keeps the baseline provenance-consistent; use
    # --baseline-kind any for ad-hoc cross-kind comparisons.
    if args.baseline_kind == "daily":
        prior = [p for p in prior if p.get("kind") == "daily"]
    window = prior[-args.window:]

    if hist_runner and cur_runner and cur_runner != hist_runner:
        msg = (f"latest point ran on runner '{cur_runner}' but the release history "
               f"was built on '{hist_runner}'. Comparing against same-runner points "
               f"only; re-run compile-perf-release-sweep (force=true) to resync the "
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
    for key, cur in sorted(current.get("metrics", {}).items()):
        wl, _, timer = key.partition("|")
        if timer not in timers_for(wl):
            continue
        baseline = [p["metrics"][key] for p in window if key in p.get("metrics", {})]
        if len(baseline) < args.min_baseline:
            continue
        med = statistics.median(baseline)
        if med <= 0:
            continue
        ratio = cur / med
        delta = cur - med
        # args.abs is passed as the floor rather than read inside
        # classify_metric so a per-counter floor can be substituted here
        # without touching the classifier or its self-checks — the memory
        # counters landing in this series want a 1 MiB floor, not 2 ms.
        verdict = classify_metric(ratio, delta, args.rel, args.warn_rel, args.abs)
        if verdict == "error":
            regressions.append((wl, timer, med, cur, ratio, delta))
        elif verdict == "warning":
            warnings.append((wl, timer, med, cur, ratio, delta))

    regressions.sort(key=lambda r: -r[4])
    warnings.sort(key=lambda r: -r[4])

    print(f"baseline: trailing {len(window)} point(s) [{base_labels}], "
          f"median per metric; ERROR at ratio >= {args.rel}, WARNING at "
          f">= {args.warn_rel}, both gated on >= {args.abs} ms\n")

    # `warnings` is the ONLY key the workflow reads, and the only one it
    # needs: the Slack step distinguishes a warnings-only night from a clean
    # one, which the exit code alone cannot do since warnings deliberately do
    # not fail the job. A regression is already carried by the exit code
    # (steps.trend.outcome == 'failure'), so an `errors` key would be a second
    # spelling of the same fact — one that no reader would notice going stale.
    #
    # This write must stay AHEAD of every path that leaves main() below — the
    # clean-night `return` and the regression `SystemExit(1)`. Both are exits
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

    print(f"{'workload':20s}{'timer':26s}{'median':>10}{'current':>10}{'ratio':>8}{'Δms':>9}")
    rows = [f"### {'🔴' if regressions else '⚠️'} Compile-perf trend — " + current["label"],
            f"\nvs trailing {len(window)}-point median (`{base_labels}`), "
            f"runner `{cur_runner}`. ERROR ≥ {args.rel}×, WARNING ≥ {args.warn_rel}×.\n"]

    def table(items, kind, gha):
        rows.append(f"\n**{kind}** ({len(items)}):\n")
        rows.append("| workload | timer | median (ms) | current (ms) | ratio | Δ ms |")
        rows.append("|---|---|--:|--:|--:|--:|")
        for wl, timer, med, cur, ratio, delta in items:
            print(f"{wl:20s}{timer:26s}{med:10.1f}{cur:10.1f}{ratio:7.2f}x{delta:+9.1f}")
            emit_gha_command(f"::{gha} title=Perf {kind.lower()} {wl}/{timer}::"
               f"{ratio:.2f}x ({med:.1f} -> {cur:.1f} ms, +{delta:.1f}) vs trailing median")
            rows.append(f"| {wl} | {timer} | {med:.1f} | {cur:.1f} | "
                        f"{ratio:.2f}× | +{delta:.1f} |")

    if regressions:
        table(regressions, "Regressions", "error")
    if warnings:
        table(warnings, "Warnings", "warning")
    write_step_summary("\n".join(rows))

    print(f"\n{len(regressions)} regression(s), {len(warnings)} warning(s) flagged.")
    if regressions and not args.no_fail:
        raise SystemExit(1)


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
    try:
        check_threshold_order(*_bad)
    except SystemExit:
        pass
    else:
        raise AssertionError(f"check_threshold_order{_bad} must be rejected")
del _REL, _WARN, _ABS, _bad


if __name__ == "__main__":
    main()
