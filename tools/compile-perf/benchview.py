"""Export compile-perf results to the Slang project in BenchView (stdlib only)."""

import argparse
import datetime
import hashlib
import json
import math
import os
import re
import statistics
import sys
import tempfile
from pathlib import Path

from lib.buckets import TREE, SOURCE_TREE, API_TREE, buckets, tree_for
from lib.manifest import BY_NAME


def metric(metric_id, name, unit, stats):
    """Preserve raw samples rather than recomputing rounded source summaries."""
    samples = stats.get("samples") if isinstance(stats, dict) else None
    if not isinstance(samples, list) or not samples or any(isinstance(x, bool) or not isinstance(x, (int, float))
                          or not math.isfinite(x) for x in samples):
        raise ValueError(f"{name}: finite raw samples are required")
    return {"id": "compile." + metric_id, "name": name, "unit": unit,
            "direction": "lower", "distribution": {"samples": samples}}


def tree_names(tree):
    """Enumerate timers used by the suite's phase allocation model."""
    return [tree[0]] + [name for child in tree[1] for name in tree_names(child)]


def phase_breakdown(timers, tree):
    """Apply the suite's overlap-normalizing allocation to each aligned sample.

    These are derived allocations, not independently measured exclusive phases.
    Missing timers follow the suite's zero allocation convention. Unequal sample
    counts cannot establish alignment and are rejected instead of zipped silently.
    """
    names = [name for name in tree_names(tree) if name in timers]
    total = timers[tree[0]]["samples"]
    if any(len(timers[name]["samples"]) != len(total) for name in names):
        raise ValueError("Cannot align phase samples")
    allocated = [buckets({name: timers[name]["samples"][i] for name in names}, tree)
                 for i in range(len(total))]
    # An entirely zero root still needs one component in the API contract.
    labels = sorted({name for sample in allocated for name in sample}) or [tree[0] + " (self)"]
    components = [
        {"id": "phase." + name.lower().replace(" (self)", ".self"),
         "name": name, "distribution": {"samples": [s.get(name, 0.0) for s in allocated]}}
        for name in labels]
    if len({component["id"] for component in components}) != len(components):
        raise ValueError("Breakdown component names collide after normalization")
    return {"mode": "complete", "components": components}


def observation(record, observed, runner, os_name):
    """Map native quantities and retain every source field in a JSON artifact."""
    # Failed samples may leave partial, unaligned streams. Retain those in the
    # source artifact, but do not present them as a successful timing result.
    timers = (record.get("timers") or {}) if record["ok"] else {}
    priority = list(dict.fromkeys(tree_names(TREE) + tree_names(SOURCE_TREE) + tree_names(API_TREE)
                                 + record.get("primary_timers", [])))
    names = [n for n in priority if n in timers]
    names += [n for n in sorted(timers) if n not in names]
    metrics = []
    for key, unit in (("wall_ms", "ms"), ("rss_kb", "KiB")):
        if record["ok"] and record.get(key):
            metrics.append(metric(key, key, unit, record[key]))
    for name, stats in ((record.get("memory") or {}) if record["ok"] else {}).items():
        metrics.append(metric("memory." + name.lower(), name, "KiB", stats))
    if len(metrics) > 64:
        raise ValueError("Memory metrics exceed BenchView's per-observation limit")
    selected = names[:64 - len(metrics)]
    primary_name = "apiTotal" if record["mode"] == "api" else "compileInner"
    test = {"id": "compile-perf/" + record["workload"], "name": record["workload"]}
    # Keep test metadata stable across failures; passed records must carry the named metric.
    if not record["ok"] or primary_name in selected:
        test["metadata"] = {"primaryMetric": "compile.timer." + primary_name.lower()}
    else:
        print(f"Warning: {record['workload']}: omitted primaryMetric; "
              f"{primary_name} is not exported", file=sys.stderr)
    # Validate distributions before selecting a tree or aligning samples. The
    # median selects the same source-emission hierarchy as the existing reports.
    timer_metrics = {name: metric("timer." + name.lower(), name, "ms", timers[name])
                     for name in names}
    compiler_tree = tree_for({name: statistics.median(entry["distribution"]["samples"])
                              for name, entry in timer_metrics.items()})
    # Labels follow the timer itself, including non-primary totals in mixed records.
    total_labels = {"compileInner": "Total compile time", "apiTotal": "Total API time"}
    for name in selected:
        entry = timer_metrics[name]
        if name in total_labels:
            entry["name"] = total_labels[name]
            tree = compiler_tree if name == "compileInner" else API_TREE
            if any(len(timers[n]["samples"]) != len(timers[name]["samples"])
                   for n in tree_names(tree) if n in timers):
                entry["metadata"] = {"breakdownOmitted": "Cannot align phase samples"}
                print(f"Warning: {record['workload']}/{name}: omitted unaligned breakdown",
                      file=sys.stderr)
            else:
                entry["breakdown"] = phase_breakdown(timers, tree)
                entry["metadata"] = {"allocation": "lib.buckets per sample; overlapping children "
                                     "scaled proportionally to parent budget; missing timers zero"}
        metrics.append(entry)
    if len({m["id"] for m in metrics}) != len(metrics):
        raise ValueError("Metric names collide after normalization")
    if record["ok"] and not metrics:
        raise ValueError("A passed workload requires at least one metric")
    source = json.dumps(record, allow_nan=False)
    if len(source.encode()) > 262144:
        raise ValueError("Source record exceeds BenchView's inline artifact limit")
    result = {
        "test": test,
        "case": {"dimensions": {"size": record["size"], "mode": record["mode"]}},
        "observedAt": observed, "status": "passed" if record["ok"] else "failed",
        "environment": {"identity": {"machine": runner, "os": {"name": os_name}}},
        "metrics": metrics,
        "metadata": {"observedAtBasis": "caller-supplied measurement timestamp",
                     "ungraphedTimers": names[len(selected):]},
        "artifacts": [{"id": "source-record", "name": "Original compile-perf record",
                       "fileName": "record.json", "mediaType": "application/json",
                       "content": {"kind": "inline-text", "text": source}}],
    }
    if not record["ok"]:
        result["error"] = {"message": str(record.get("error") or "Workload failed")[:4096]}
        result["metadata"]["metricsOmitted"] = "Failed workload; partial samples retained in source record"
    return result


def encode(value):
    """Encode deterministically so replaying a saved result is idempotent."""
    return json.dumps(value, sort_keys=True, allow_nan=False, separators=(",", ":")).encode()


def submissions(records, meta, os_name, observed, execution, release=None, branch=None):
    """Convert one measured compiler run, splitting requests within API limits."""
    timestamp(observed)
    # track.runner_id() prefixes its fingerprint with platform.system(). Check
    # known prefixes without guessing the OS of arbitrary historical runner IDs.
    runner_os = meta["runner"].split("-", 1)[0]
    if runner_os in ("Windows", "Linux", "Darwin") and runner_os != os_name:
        raise ValueError("Measured OS disagrees with the recorded runner fingerprint")
    if release:
        if not isinstance(release["label"], str) or not 1 <= len(release["label"]) <= 128:
            raise ValueError("Release labels must contain 1 to 128 characters")
        if "releasedAt" in release:
            timestamp(release["releasedAt"])
    revision = meta["commit"]
    if not re.fullmatch(r"[0-9a-fA-F]{40}", revision):
        raise ValueError("A full compiler commit SHA is required")
    dimensions = {"kind": "release" if release else "daily",
                  "corpus": meta.get("corpus_sha") or "unrecorded"}
    run = {"key": "git:" + revision + "/suite:compile-perf/config:"
           + hashlib.sha256(encode(dimensions)).hexdigest(),
           "suite": {"id": "compile-perf", "name": "Compiler performance"},
           "vcs": {"repository": "https://github.com/shader-slang/slang", "revision": revision},
           "dimensions": dimensions}
    if meta.get("commit_time"):
        run["vcs"]["commitTime"] = meta["commit_time"]
    # Branch is verified revision provenance, not just a display grouping; callers
    # omit it for detached daily checkouts or unverified release targets.
    if branch:
        run["vcs"]["branch"] = branch
    if release:
        run["metadata"] = {"release": release}
    envelope = {"schemaVersion": 1, "project": {"id": "slang", "name": "Slang",
                "metadata": {"defaultBranch": "master", "breakdownTooltipMaxItems": 1}},
                "producer": {"name": "slang-compile-perf", "version": "1"},
                "run": run, "execution": {"id": execution, "metadata": meta}}
    batches, batch, identities = [], [], set()
    for record in records:
        identity = (record["workload"], record["mode"], record["size"])
        if identity in identities:
            raise ValueError(f"Duplicate workload case: {identity}")
        identities.add(identity)
        item = observation(record, observed, meta["runner"], os_name)
        if batch and (len(batch) == 500 or
                      len(encode(dict(envelope, observations=batch + [item]))) > 7 * 1024 * 1024):
            batches.append(batch)
            batch = []
        batch.append(item)
    if batch:
        batches.append(batch)
    if not batches:
        raise ValueError("No observations to export")
    payloads = []
    for batch in batches:
        payload = dict(envelope, observations=batch)
        payload["idempotencyKey"] = "compile-perf:" + hashlib.sha256(encode(payload)).hexdigest()
        if len(encode(payload)) > 8 * 1024 * 1024:
            raise ValueError("A single observation exceeds the request size limit")
        payloads.append(payload)
    return payloads


def timestamp(value):
    """Require an actual RFC3339 timestamp instead of guessing from file mtimes."""
    if not isinstance(value, str) or not re.fullmatch(r"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:\.\d+)?(?:Z|[+-]\d{2}:\d{2})", value):
        raise ValueError("Expected an RFC3339 timestamp with timezone")
    datetime.datetime.fromisoformat(value.replace("Z", "+00:00"))


def write_atomic(path, content):
    """Replace a file only after its complete contents have been written and closed."""
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(content)
        os.replace(temporary, path)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def export_content(payloads):
    """Validate collector limits before publishing or adding an optional release."""
    if not 1 <= len(payloads) <= 64:
        raise ValueError("An export requires 1 to 64 submission batches")
    wrapper = {"schemaVersion": 1, "kind": "slang-compile-perf-benchview-export",
               "submissionSchema": "benchmark-submission-v1", "submissions": payloads}
    content = encode(wrapper) + b"\n"
    if len(content) > 8 * 1024 * 1024:
        raise ValueError("Collector export exceeds the 8 MiB limit")
    return content


def export(payloads, output):
    """Publish a bounded host-collector bundle, then its ready marker; never send credentials."""
    output.mkdir(parents=True, exist_ok=True)
    ready = output / "submissions.ready"
    ready.unlink(missing_ok=True)
    write_atomic(output / "submissions.json", export_content(payloads))
    write_atomic(ready, b"ready\n")


def is_current_default_case(record):
    """Keep release cases comparable with daily runs, excluding retired tests and sweep sizes."""
    spec = BY_NAME.get(record["workload"])
    # Match sweep.workload_complete's typed-size convention: False and 0.0 are not 0.
    return (spec is not None and type(record["size"]) is type(spec.default_size)
            and record["size"] == spec.default_size)


def release_submissions(completed, results_dir, daily_meta, os_name, execution, existing=()):
    """Build payloads from new_release_check.py --completed and <results_dir>/<tag>/results.json.

    Each manifest entry requires tag, commit, commit_time, released_at and observed_at;
    branch is optional. Invalid releases are warned about and skipped independently.
    Existing payloads reserve collector capacity for the daily results.
    """
    if not isinstance(completed, list):
        raise ValueError("Completed releases must be a list")
    payloads = []
    for info in completed:
        tag = "<unknown>"
        try:
            tag = info["tag"]
            if not isinstance(tag, str) or not re.fullmatch(r"v\d{4}\.\d+(?:\.\d+)?", tag):
                raise ValueError("Invalid completed release tag")
            records = json.loads((results_dir / tag / "results.json").read_text(encoding="utf-8"))
            selected = [r for r in records if is_current_default_case(r)]
            # Daily and release measurements share this job's host and corpus checkout.
            # Do not inherit daily labels, dates or compiler paths as release provenance.
            meta = {key: daily_meta[key] for key in ("runner", "corpus_sha") if key in daily_meta}
            meta.update(commit=info["commit"], commit_time=info["commit_time"])
            candidate = submissions(selected, meta, os_name, info["observed_at"],
                                    execution + "-" + tag,
                                    {"label": tag, "releasedAt": info["released_at"]},
                                    branch=info.get("branch"))
            export_content(list(existing) + payloads + candidate)
            payloads.extend(candidate)
        except (ValueError, KeyError, TypeError, AttributeError, RuntimeError, OSError) as error:
            print(f"Warning: skipped BenchView release {tag}: {error}; "
                  "export its archived results separately to retry", file=sys.stderr)
    return payloads


def main():
    """Export results for collection and delivery by the benchmark host."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results", required=True, type=Path)
    parser.add_argument("--meta", required=True, type=Path)
    parser.add_argument("--os", required=True, help="Measured OS, not the importing host's OS")
    parser.add_argument("--observed-at", required=True, help="Measurement time in RFC3339")
    parser.add_argument("--execution-id", required=True)
    parser.add_argument("--branch")
    parser.add_argument("--release", help="Slang version tested, e.g. v2026.19")
    parser.add_argument("--released-at", help="Actual release timestamp, if known")
    parser.add_argument("--completed-releases", type=Path,
                        help="Metadata written by new_release_check.py for this invocation")
    parser.add_argument("--release-results", type=Path,
                        help="Directory containing the completed releases' results")
    parser.add_argument("--output", required=True, type=Path,
                        help="Directory for submissions.json and its submissions.ready marker")
    args = parser.parse_args()
    # Invalidate a previous export before reading inputs or converting any observations.
    (args.output / "submissions.ready").unlink(missing_ok=True)
    if args.released_at and not args.release:
        parser.error("--released-at requires --release")
    if bool(args.completed_releases) != bool(args.release_results):
        parser.error("--completed-releases and --release-results must be used together")
    if args.completed_releases and args.release:
        parser.error("--completed-releases cannot be combined with --release")
    release = {"label": args.release} if args.release else None
    if args.released_at:
        release["releasedAt"] = args.released_at
    meta = json.loads(args.meta.read_text(encoding="utf-8"))
    payloads = submissions(json.loads(args.results.read_text(encoding="utf-8")),
                           meta, args.os,
                           args.observed_at, args.execution_id, release, args.branch)
    if args.completed_releases:
        try:
            payloads.extend(release_submissions(
                json.loads(args.completed_releases.read_text(encoding="utf-8")),
                args.release_results, meta, args.os, args.execution_id, existing=payloads))
        except (ValueError, OSError) as error:
            print(f"Warning: skipped BenchView release manifest: {error}; "
                  "export its archived results separately to retry", file=sys.stderr)
    export(payloads, args.output)
    print(f"Exported {sum(len(p['observations']) for p in payloads)} observations")


if __name__ == "__main__":
    try:
        main()
    except (ValueError, KeyError, TypeError, AttributeError, RuntimeError, OSError) as error:
        print(f"BenchView export failed: {error}", file=sys.stderr)
        sys.exit(1)


