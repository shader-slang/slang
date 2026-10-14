"""Export compile-perf results to the Slang project in BenchView (stdlib only)."""

import argparse
import datetime
import hashlib
import json
import math
import os
import re
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path

from lib.buckets import TREE, API_TREE, buckets


def metric(metric_id, name, unit, stats):
    """Preserve raw samples rather than recomputing rounded source summaries."""
    samples = stats.get("samples")
    if not samples or any(isinstance(x, bool) or not isinstance(x, (int, float))
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
    return {"mode": "complete", "components": [
        {"id": "phase." + name.lower().replace(" (self)", ".self"),
         "name": name, "distribution": {"samples": [s.get(name, 0.0) for s in allocated]}}
        for name in labels]}


def observation(record, observed, runner, os_name):
    """Map native quantities and retain every source field in a JSON artifact."""
    timers = record.get("timers") or {}
    priority = list(dict.fromkeys(tree_names(TREE) + tree_names(API_TREE)
                                 + record.get("primary_timers", [])))
    names = [n for n in priority if n in timers]
    names += [n for n in sorted(timers) if n not in names]
    metrics = []
    for key, unit in (("wall_ms", "ms"), ("rss_kb", "KiB")):
        if record.get(key):
            metrics.append(metric(key, key, unit, record[key]))
    for name, stats in (record.get("memory") or {}).items():
        metrics.append(metric("memory." + name.lower(), name, "KiB", stats))
    if len(metrics) > 64:
        raise ValueError("Memory metrics exceed BenchView's per-observation limit")
    selected = names[:64 - len(metrics)]
    for name in selected:
        entry = metric("timer." + name.lower(), name, "ms", timers[name])
        if name in ("compileInner", "apiTotal"):
            tree = TREE if name == "compileInner" else API_TREE
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
        "test": {"id": "compile-perf/" + record["workload"], "name": record["workload"]},
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
    return result


def encode(value):
    """Encode deterministically so replaying a saved result is idempotent."""
    return json.dumps(value, sort_keys=True, allow_nan=False, separators=(",", ":")).encode()


def submissions(records, meta, os_name, observed, execution, release=None, branch=None):
    """Convert one measured compiler run, splitting requests within API limits."""
    timestamp(observed)
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
    if branch:
        run["vcs"]["branch"] = branch
    if release:
        run["metadata"] = {"release": release}
    envelope = {"schemaVersion": 1, "project": {"id": "slang", "name": "Slang"},
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
    if not re.fullmatch(r"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:\.\d+)?(?:Z|[+-]\d{2}:\d{2})", value):
        raise ValueError("Expected an RFC3339 timestamp with timezone")
    datetime.datetime.fromisoformat(value.replace("Z", "+00:00"))


class NoRedirect(urllib.request.HTTPRedirectHandler):
    """Never forward the write credential to a redirect destination."""

    def redirect_request(self, req, fp, code, msg, headers, newurl):
        """Reject redirects even when the target is on the same host."""
        return None


def submit(payload, base_url, key):
    """Retry transient transport failures with the exact same idempotency key."""
    parsed = urllib.parse.urlsplit(base_url)
    if (parsed.scheme not in ("http", "https") or not parsed.hostname
            or parsed.username or parsed.password or parsed.query or parsed.fragment):
        raise ValueError("Expected a BenchView HTTP(S) base URL without credentials or query")
    request = urllib.request.Request(
        base_url.rstrip("/") + "/api/v1/submissions", data=encode(payload),
        headers={"Content-Type": "application/json", "Authorization": "Bearer " + key})
    opener = urllib.request.build_opener(NoRedirect())
    for attempt in range(3):
        try:
            with opener.open(request, timeout=30) as response:
                if response.status not in (200, 201):
                    raise RuntimeError(f"Unexpected BenchView status: {response.status}")
                return
        except urllib.error.HTTPError as error:
            status = error.code
            error.close()
            if status not in (408, 429, 500, 502, 503, 504) or attempt == 2:
                raise RuntimeError(f"BenchView submission failed: HTTP {status}") from None
        except (urllib.error.URLError, TimeoutError, OSError):
            if attempt == 2:
                raise RuntimeError("BenchView submission failed: connection error") from None
        time.sleep(2 ** attempt)


def main():
    """Save reviewable payloads and optionally submit using BENCHVIEW_WRITE_KEY."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results", required=True, type=Path)
    parser.add_argument("--meta", required=True, type=Path)
    parser.add_argument("--os", required=True, help="Measured OS, not the importing host's OS")
    parser.add_argument("--observed-at", required=True, help="Measurement time in RFC3339")
    parser.add_argument("--execution-id", required=True)
    parser.add_argument("--branch")
    parser.add_argument("--release", help="Slang version tested, e.g. v2026.19")
    parser.add_argument("--released-at", help="Actual release timestamp, if known")
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--url", help="Omit to export without submitting")
    args = parser.parse_args()
    if args.released_at and not args.release:
        parser.error("--released-at requires --release")
    release = {"label": args.release} if args.release else None
    if args.released_at:
        release["releasedAt"] = args.released_at
    payloads = submissions(json.loads(args.results.read_text(encoding="utf-8")),
                           json.loads(args.meta.read_text(encoding="utf-8")), args.os,
                           args.observed_at, args.execution_id, release, args.branch)
    args.output.mkdir(parents=True, exist_ok=True)
    for i, payload in enumerate(payloads):
        (args.output / f"submission-{i + 1:04d}.json").write_bytes(encode(payload) + b"\n")
    print(f"Exported {sum(len(p['observations']) for p in payloads)} observations")
    if args.url:
        key = os.environ.get("BENCHVIEW_WRITE_KEY")
        if not key:
            raise ValueError("BENCHVIEW_WRITE_KEY is required for submission")
        for payload in payloads:
            submit(payload, args.url, key)
        print(f"Submitted {len(payloads)} batch(es) to BenchView")


if __name__ == "__main__":
    try:
        main()
    except (ValueError, KeyError, RuntimeError, OSError) as error:
        print(f"BenchView export failed: {error}", file=sys.stderr)
        sys.exit(1)


