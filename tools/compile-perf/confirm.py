#!/usr/bin/env python3
"""Rerun nightly alert candidates once, then report only reproducible changes.

The measure command runs beside the original compiler on the performance host.
It preserves the original results and writes confirmation.json beside them.
The report command runs on the notification host and reads the archived original
results, metadata and confirmation.
Neither command replaces the original point or adds retries to rolling history.
"""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import subprocess
import sys
import tempfile
from types import SimpleNamespace

import bench
from lib import analyze, manifest
import trend
from slack_status import EXIT_CANNOT_EVALUATE


def source_digest(directory):
    """Bind confirmation to the exact original results and commit/runner metadata."""
    digest = hashlib.sha256()
    for name in ("results.json", "meta.json"):
        data = (directory / name).read_bytes()
        digest.update(len(data).to_bytes(8, "big"))
        digest.update(data)
    return digest.hexdigest()


def candidate_rows(plan):
    """Read archived MetricChange objects and pair each with its initial severity."""
    for tier, key in (("error", "regressions"), ("warning", "warnings")):
        for row in plan[key]:
            yield tier, trend.MetricChange(**row)


def rerun(plan, original, slangc):
    """Measure each affected workload/size once more, using its original sample count.

    Several counters may flag in one workload. They share one confirmation batch,
    not separate retries. A size sweep confirms only the canonical size judged by
    trend. The deterministic generators and pinned corpus are the same as the
    first sweep, and the compiler executable is reused without rebuilding it.
    """
    wanted = {change.workload for _, change in candidate_rows(plan)}
    selected = [r for r in analyze.canonical_runs(original) if r["workload"] in wanted]
    if len(selected) != len(wanted):
        raise ValueError("candidate workload missing from original results")
    if not wanted:
        return []
    counts = {(r["samples"], r["warmup"]) for r in selected}
    if len(counts) != 1:
        raise ValueError("candidate workloads have inconsistent sample/warmup counts")
    samples, warmup = counts.pop()
    cases = [(manifest.BY_NAME[r["workload"]], r["size"]) for r in selected]
    with tempfile.TemporaryDirectory(prefix="perf_confirmation_") as scratch:
        api = None
        if any(spec.mode == "api" for spec, _ in cases):
            driver = bench.build_api_driver(scratch)
            library = bench.find_libslang(slangc)
            if driver and library:
                api = {"driver": driver, "libslang": library}
        return bench.run_workloads(slangc, cases, samples, warmup, scratch, scratch,
                                   api=api, verbose=True)


def confirmed_changes(plan, original, repeated):
    """Require the same counter to cross the frozen baseline in both valid batches.

    For example, a 12% first increase followed by 3% is unconfirmed; 12% followed
    by 7% confirms only a warning. A 7% first increase followed by 12% also confirms
    only a warning: two error-level measurements are required for a red alarm.
    Return (errors, warnings, cleared) lists of MetricChange objects. A valid
    rerun below both thresholds goes in cleared. Missing, invalid, incomplete
    or incompatible measurements instead raise ValueError: callers report an
    evaluation failure, never recovery.
    """
    first = {r["workload"]: r for r in analyze.canonical_runs(original)}
    second = {r["workload"]: r for r in analyze.canonical_runs(repeated)}
    errors, warnings, cleared = [], [], []
    limits = plan["thresholds"]
    for initial_tier, candidate in candidate_rows(plan):
        wl, counter = candidate.workload, candidate.counter
        baseline, initial = candidate.baseline, candidate.value
        a, b = first.get(wl), second.get(wl)
        if a is None or b is None or not a.get("ok") or not b.get("ok"):
            raise ValueError(f"{wl}: original or confirmation workload failed or is missing")
        for field in ("size", "timer_schema", "sampling_strategy", "samples", "warmup"):
            if a.get(field) is None or a[field] != b.get(field):
                raise ValueError(f"{wl}: confirmation {field} differs from original")
        if b["sampling_strategy"] != "interleaved":
            raise ValueError(f"{wl}: confirmation requires interleaved sampling")
        for record in (a, b):
            stat = record["timers"].get(counter)
            if (not stat or stat.get("n") != record["samples"]
                    or len(stat.get("samples", [])) != record["samples"]
                    or not all(math.isfinite(v) for v in stat["samples"])
                    or not math.isfinite(stat["median"])):
                raise ValueError(f"{wl}/{counter}: incomplete or invalid measurements")
        if a["timers"][counter]["median"] != initial:
            raise ValueError(f"{wl}/{counter}: candidate does not match original results")
        if not math.isfinite(baseline) or baseline <= 0:
            raise ValueError(f"{wl}/{counter}: invalid frozen baseline")
        value = b["timers"][counter]["median"]
        ratio, delta = value / baseline, value - baseline
        tier = trend.classify_metric(ratio, delta, limits["rel"], limits["warn_rel"],
                                     trend.abs_floor_for(counter, limits["abs"]))
        item = trend.MetricChange(wl, counter, baseline, value)
        if tier is None:
            cleared.append(item)
        elif initial_tier == "error" and tier == "error":
            errors.append(item)
        else:
            warnings.append(item)
    return errors, warnings, cleared


def measure(args, directory):
    """Freeze candidate baselines, execute one rerun, and archive even failed attempts."""
    archive = directory / "confirmation.json"
    result = {"label": args.label, "status": "incomplete", "repeated": [],
              "run_id": os.environ.get("GITHUB_RUN_ID"),
              "run_attempt": os.environ.get("GITHUB_RUN_ATTEMPT")}
    # Replace a previous attempt's archive before starting a process that could
    # be interrupted. A killed rerun must never leave a usable old verdict.
    with analyze.open_output(str(archive)) as fh:
        json.dump(result, fh, indent=2)
    try:
        result["source_digest"] = source_digest(directory)
        with tempfile.TemporaryDirectory(prefix="perf_candidates_") as scratch:
            plan_path = Path(scratch) / "candidates.json"
            subprocess.run([sys.executable, str(Path(__file__).with_name("trend.py")),
                            "--results", str(args.results), "--label", args.label,
                            "--candidates", str(plan_path)], check=True)
            result["plan"] = analyze.read_json(plan_path)
        original = analyze.read_json(directory / "results.json")
        workloads = sorted({change.workload for _, change in candidate_rows(result["plan"])})
        print("Confirmation workloads: " + (", ".join(workloads) or "none"), flush=True)
        result["repeated"] = rerun(result["plan"], original, str(args.slangc.resolve()))
        confirmed_changes(result["plan"], original, result["repeated"])
        result["status"] = "complete"
    except Exception as exc:
        result["error"] = str(exc) or type(exc).__name__
    finally:
        with analyze.open_output(str(archive)) as fh:
            json.dump(result, fh, indent=2)
    if result["status"] != "complete":
        trend.abort("confirmation could not evaluate: " + result["error"])


def report(args, directory):
    """Report archived confirmation without recomputing a moving historical baseline."""
    result = analyze.read_json(directory / "confirmation.json")
    if (result.get("run_id") != os.environ.get("GITHUB_RUN_ID")
            or result.get("run_attempt") != os.environ.get("GITHUB_RUN_ATTEMPT")):
        raise ValueError("confirmation belongs to a different workflow attempt")
    if result["status"] != "complete":
        raise ValueError(result.get("error", "confirmation did not complete"))
    if result["label"] != args.label or result["source_digest"] != source_digest(directory):
        raise ValueError("confirmation does not belong to these original results")
    plan = result["plan"]
    if plan["label"] != args.label:
        raise ValueError("candidate label does not match confirmation")
    original = analyze.read_json(directory / "results.json")
    errors, warnings, cleared = confirmed_changes(plan, original, result["repeated"])
    for note in plan["notes"]:
        print("WARNING: " + note)
        trend.emit_gha_command("::warning title=Perf comparison coverage::" + note)
        trend.write_step_summary(note)
    candidate_count = sum(1 for _ in candidate_rows(plan))
    message = (f"Confirmation: {candidate_count} candidate counter(s); "
               f"{len(errors)} confirmed regression(s), {len(warnings)} confirmed warning(s), "
               f"{len(cleared)} not reproduced. Original and rerun samples are archived.")
    print(message)
    labels = plan["baseline_labels"]
    limits = SimpleNamespace(**plan["thresholds"], no_fail=False)
    # Append the confirmation explanation even when the shared renderer exits
    # with EXIT_REGRESSION.
    try:
        trend.report_changes(limits, args.label, plan["runner"],
                             f"{labels[0]}..{labels[-1]}" if labels else "none",
                             len(labels), errors, warnings, plan["judged_count"])
    finally:
        trend.write_step_summary(message)


def main():
    for stream in (sys.stdout, sys.stderr):
        if hasattr(stream, "reconfigure"):
            stream.reconfigure(encoding="utf-8", errors="replace")
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("measure", "report"))
    parser.add_argument("--results", required=True, type=Path)
    parser.add_argument("--label", required=True)
    parser.add_argument("--slangc", type=Path)
    args = parser.parse_args()
    if args.command == "measure" and args.slangc is None:
        parser.error("measure requires --slangc")
    directory = args.results / "daily" / args.label
    try:
        if args.command == "measure":
            measure(args, directory)
        else:
            report(args, directory)
    # SystemExit(EXIT_REGRESSION) must reach the shell unchanged.
    except Exception as exc:
        print(f"confirmation could not evaluate: {exc}", file=sys.stderr)
        raise SystemExit(EXIT_CANNOT_EVALUATE)


if __name__ == "__main__":
    main()
