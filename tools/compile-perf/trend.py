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
    python3 trend.py --results <dir> --window 7 --rel 1.25 --abs 2.0
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
    # --rel 1.25: flag a 25% rise vs trailing median. The per-PR gate uses 15%;
    #   25% here catches gradual drift that accumulates across many PRs without
    #   any single one tripping the per-PR gate.
    # --abs 2.0: ignore sub-2 ms absolute deltas regardless of ratio — a 50%
    #   rise in a 3 ms timer is within measurement noise, not a real regression.
    # --window 7: trailing-7-point median spans ~one week of nightly runs,
    #   long enough to be stable against one bad night but short enough to track
    #   genuine drift (a regression from 3 weeks ago is already known).
    # --min-baseline 3: require at least 3 prior same-runner points before judging,
    #   so the first few nights after a new runner don't produce false positives.
    ap.add_argument("--window", type=int, default=7, help="trailing points for the median")
    ap.add_argument("--rel", type=float, default=1.25, help="relative regression threshold")
    ap.add_argument("--abs", type=float, default=2.0, help="min absolute ms delta to flag")
    ap.add_argument("--min-baseline", type=int, default=3,
                    help="min trailing points required to judge a metric")
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

    tpath = os.path.join(args.results, "tracking", "tracking.json")
    if not os.path.exists(tpath):
        raise SystemExit(f"no tracking series at {tpath}; run track.py rebuild first")
    series = json.load(open(tpath))
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
        if ratio >= args.rel and delta >= args.abs:
            regressions.append((wl, timer, med, cur, ratio, delta))

    regressions.sort(key=lambda r: -r[4])

    print(f"baseline: trailing {len(window)} point(s) [{base_labels}], "
          f"median per metric; flag at ratio >= {args.rel} and >= {args.abs} ms\n")
    if not regressions:
        print(f"OK — no compile-perf regression in {current['label']} vs trailing median.")
        write_step_summary(f"### Compile-perf trend — {current['label']}\n\n"
                f"OK — no regression vs trailing {len(window)}-point median "
                f"(`{base_labels}`).")
        return

    print(f"{'workload':20s}{'timer':26s}{'median':>10}{'current':>10}{'ratio':>8}{'Δms':>9}")
    rows = ["### ⚠️ Compile-perf regressions — " + current["label"],
            f"\nvs trailing {len(window)}-point median (`{base_labels}`), "
            f"runner `{cur_runner}`.\n",
            "| workload | timer | median (ms) | current (ms) | ratio | Δ ms |",
            "|---|---|--:|--:|--:|--:|"]
    for wl, timer, med, cur, ratio, delta in regressions:
        print(f"{wl:20s}{timer:26s}{med:10.1f}{cur:10.1f}{ratio:7.2f}x{delta:+9.1f}")
        emit_gha_command(f"::error title=Perf regression {wl}/{timer}::"
           f"{ratio:.2f}x ({med:.1f} -> {cur:.1f} ms, +{delta:.1f}) vs trailing median")
        rows.append(f"| {wl} | {timer} | {med:.1f} | {cur:.1f} | "
                    f"{ratio:.2f}× | +{delta:.1f} |")
    write_step_summary("\n".join(rows))

    print(f"\n{len(regressions)} regression(s) flagged.")
    if not args.no_fail:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
