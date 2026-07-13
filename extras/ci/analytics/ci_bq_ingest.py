#!/usr/bin/env python3
"""Phase 1 BigQuery ingester for Slang CI telemetry.

Tier 1 (`parse_graph`) ingest of a single (run_id, run_attempt):

    ci_bq_ingest.py --run-id N --run-attempt M [--repo shader-slang/slang]
                    [--validate] [--table workflow_runs|jobs|job_steps|all]

Emits NDJSON to stdout, one row per output line, prefixed by `{"_table": "..."}`
when --table=all so the consumer can sort rows by destination table. Single-table
modes emit bare rows (suitable for `bq load`).

No BigQuery writes; that's a later slice.
"""

from __future__ import annotations

import argparse
import json
import sys
from collections import defaultdict
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))

from _bq_ingest_lib import (
    gh_api,
    gh_api_paginated,
    rk_caller_node,
    rk_concrete,
    rk_direct_node,
    rk_step,
    rk_workflow_run,
    workflow_tier,
)
from bq_loader import load_rows_to_bq
from workflow_parser import JobSpec, WorkflowSpec, fetch_workflow_yaml


# ---------- API → row builders ----------


def build_workflow_run_row(repo: str, run_id: int, run_attempt: int) -> dict[str, Any]:
    payload = gh_api(f"repos/{repo}/actions/runs/{run_id}/attempts/{run_attempt}")
    actor = payload.get("triggering_actor") or {}
    return {
        "row_key": rk_workflow_run(run_id, run_attempt),
        "run_id": payload["id"],
        "run_attempt": payload["run_attempt"],
        "workflow_file": payload.get("path"),
        "workflow_name": payload.get("name"),
        "event": payload.get("event"),
        "head_sha": payload.get("head_sha"),
        "head_branch": payload.get("head_branch"),
        "triggering_actor": actor.get("login"),
        "parent_run_id": None,
        "created_at": payload.get("created_at"),
        "started_at": payload.get("run_started_at"),
        "completed_at": payload.get("updated_at"),
        "conclusion": payload.get("conclusion"),
    }


def fetch_concrete_jobs(repo: str, run_id: int, run_attempt: int) -> list[dict[str, Any]]:
    return gh_api_paginated(
        f"repos/{repo}/actions/runs/{run_id}/attempts/{run_attempt}/jobs",
        "jobs",
    )


# ---------- Caller-key parsing ----------


def split_concrete_name(name: str) -> tuple[str, str | None]:
    """Concrete job names from a reusable-workflow caller look like
    `<caller_job_key> / <called_job_name>`. For direct jobs (no `uses:`) the
    name is just `<caller_job_key>` (or its `name:` override).

    Returns (caller_part, called_part_or_None). The caller_part still needs to
    be mapped back to a YAML key — GH may have substituted matrix values into
    the job's display name.
    """
    if " / " in name:
        head, tail = name.split(" / ", 1)
        return head.strip(), tail.strip()
    return name.strip(), None


def strip_matrix_suffix(caller_part: str) -> str:
    """Strip a trailing matrix-value tuple from a job name.

    GH expands `strategy.matrix` job names to `<job_key> (<v1>, <v2>, ...)`.
    The opening ` (` is the canonical separator. We don't try to parse the
    tuple — the caller-side matrix axes are reconstructed separately from the
    GH API's `runner_labels`/job metadata.

    GH truncates job names to ~100 chars in the API response, which can drop
    the closing `)`. We therefore also strip when the trailing suffix is
    `(...)` (closed) OR ends with the literal truncation marker `...` after
    an unclosed `(`.

    Examples:
      'build (windows, release, cl, x86_64)' -> 'build'
      'build (windows, cl, x86_64, ..., vu...' -> 'build'  (truncated)
      'build'                                -> 'build'
      'name (with spaces) (a, b)'            -> 'name (with spaces)'
    """
    idx = caller_part.rfind(" (")
    if idx == -1:
        return caller_part
    suffix = caller_part[idx + 2 :]
    if caller_part.endswith(")") or suffix.endswith("..."):
        return caller_part[:idx].strip()
    return caller_part


def resolve_caller_key(
    caller_part: str, workflow_spec: WorkflowSpec
) -> tuple[str | None, str]:
    """Map the API-reported caller part of a job name back to a YAML job key.

    Returns (job_key, graph_instance_key). graph_instance_key is the canonical
    matrix tuple for the caller-side matrix, or empty string when none. (Real
    matrix-tuple reconstruction is future work; for now we strip the suffix
    so the key resolves and leave graph_instance_key empty.)

    Resolution order, tried first with the raw caller_part and again with a
    matrix-suffix-stripped variant (so a job with both `strategy.matrix` and a
    custom top-level `name:` still resolves):
      1. Exact YAML-key match — covers no-matrix jobs.
      2. Match against a declared `name:` field.
      3. Stripped/lowercased match for workflows that use spaces in names.
    """
    candidates = [caller_part]
    stripped = strip_matrix_suffix(caller_part)
    if stripped != caller_part:
        candidates.append(stripped)

    for candidate in candidates:
        if candidate in workflow_spec.jobs:
            return candidate, ""
        for job_key, spec in workflow_spec.jobs.items():
            if spec.name and spec.name == candidate:
                return job_key, ""
        normalized = candidate.replace(" ", "-").lower()
        for job_key in workflow_spec.jobs:
            if job_key.lower() == normalized:
                return job_key, ""

    return None, ""


# ---------- Graph-node materialization ----------


def graph_node_kind(spec: JobSpec) -> str:
    return "caller_node" if spec.is_reusable_call else "direct_node"


def graph_node_row_key(
    spec: JobSpec, run_id: int, run_attempt: int, workflow_file: str, gik: str
) -> str:
    if spec.is_reusable_call:
        return rk_caller_node(run_id, run_attempt, workflow_file, spec.job_key, gik)
    return rk_direct_node(run_id, run_attempt, workflow_file, spec.job_key, gik)


def parse_ts(ts: str | None) -> datetime | None:
    if not ts:
        return None
    return datetime.fromisoformat(ts.replace("Z", "+00:00"))


def fmt_ts(dt: datetime | None) -> str | None:
    if dt is None:
        return None
    return dt.astimezone(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def _child_ran(child: dict[str, Any]) -> bool:
    """Did this concrete child actually execute?

    The plan says graph-node timing is over "children that actually ran". GH
    Actions reports skipped jobs with conclusion='skipped' and bogus timing
    (started_at == completed_at, or in some cases an out-of-order pair that
    yields a negative duration when subtracted). Excluding them keeps the
    aggregates honest.
    """
    return child.get("conclusion") not in ("skipped", None)


def aggregate_graph_timing(children: list[dict[str, Any]]) -> dict[str, Any]:
    """MIN/MAX over concrete children that ran + critical-path proxy.

    The full critical path requires the called workflow's internal needs[] DAG;
    for this slice we use MAX(child.duration) as a conservative approximation
    (correct when called jobs have no internal needs[], otherwise a lower
    bound). The internal-DAG walk is a follow-up.
    """
    ran = [c for c in children if _child_ran(c)]
    if not ran:
        return {
            "started_at": None,
            "completed_at": None,
            "duration_seconds": None,
            "queue_wait_seconds": None,
        }

    started = [parse_ts(c.get("started_at")) for c in ran]
    completed = [parse_ts(c.get("completed_at")) for c in ran]
    queue_waits = []
    durations = []
    for c in ran:
        cr = parse_ts(c.get("created_at"))
        st = parse_ts(c.get("started_at"))
        cp = parse_ts(c.get("completed_at"))
        if cr and st:
            queue_waits.append((st - cr).total_seconds())
        if st and cp:
            durations.append((cp - st).total_seconds())

    started_nn = [s for s in started if s is not None]
    completed_nn = [c for c in completed if c is not None]

    return {
        "started_at": fmt_ts(min(started_nn)) if started_nn else None,
        "completed_at": fmt_ts(max(completed_nn)) if completed_nn else None,
        "duration_seconds": max(durations) if durations else None,
        "queue_wait_seconds": min(queue_waits) if queue_waits else None,
    }


# ---------- Main ingest ----------


def _emit_steps_for_job(
    out: dict[str, list[dict[str, Any]]],
    api_job: dict[str, Any],
    *,
    run_id: int,
    run_attempt: int,
    run_created_at: str | None,
) -> None:
    """Append job_steps rows for one API job. Shared by both tiers."""
    job_id = api_job["id"]
    for step in api_job.get("steps") or []:
        step_number = step.get("number")
        if step_number is None:
            continue
        out["job_steps"].append(
            {
                "row_key": rk_step(job_id, step_number),
                "job_id": job_id,
                "run_id": run_id,
                "run_attempt": run_attempt,
                "run_created_at": run_created_at,
                "step_number": step_number,
                "step_name": step.get("name"),
                "conclusion": step.get("conclusion"),
                "started_at": step.get("started_at"),
                "completed_at": step.get("completed_at"),
                "duration_seconds": _duration(step),
            }
        )


def _tier2_concrete_row(
    api_job: dict[str, Any],
    *,
    run_id: int,
    run_attempt: int,
    run_created_at: str | None,
) -> dict[str, Any]:
    """Build a Tier 2 (record_only) concrete_job row. All graph columns NULL.

    Tier 2 workflows don't get their YAML parsed, so we have no caller-side
    keys, no needs[] edges, and no matrix axes. Querying these rows still
    works for hosted-runner concurrency and conclusion/duration analytics
    because workflow_name is joinable through (run_id, run_attempt) on
    workflow_runs.

    Per the plan, every graph column is NULL on Tier 2 rows — including
    caller_workflow_file. Workflow attribution is one JOIN away via
    workflow_runs.workflow_file. Validation enforces this.
    """
    return {
        "row_key": rk_concrete(api_job["id"]),
        "job_id": api_job["id"],
        "run_id": run_id,
        "run_attempt": run_attempt,
        "name": api_job.get("name"),
        "caller_workflow_file": None,
        "caller_job_key": None,
        "called_workflow_file": None,
        "called_job_key": None,
        "node_kind": "concrete_job",
        "graph_instance_key": None,
        "needs": [],
        "conclusion": api_job.get("conclusion"),
        "started_at": api_job.get("started_at"),
        "completed_at": api_job.get("completed_at"),
        "duration_seconds": _duration(api_job),
        "queue_wait_seconds": _queue_wait(api_job),
        "runner_name": api_job.get("runner_name"),
        "runner_labels": api_job.get("labels") or [],
        "job_type": None,
        "matrix_os": None,
        "matrix_arch": None,
        "matrix_compiler": None,
        "matrix_config": None,
        "matrix_gpu_tier": None,
        "run_created_at": run_created_at,
    }


def build_rows(
    repo: str, run_id: int, run_attempt: int
) -> tuple[dict[str, list[dict[str, Any]]], int, str]:
    """Return (rows-keyed-by-table-name, api_job_count, tier).

    Dispatches on the workflow's display name: Tier 1 (parse_graph) fetches
    YAML and materializes graph nodes; Tier 2 (record_only) emits concrete
    rows only with NULL graph columns. See PARSE_GRAPH_WORKFLOWS in
    _bq_ingest_lib.py for the Tier 1 list.
    """
    out: dict[str, list[dict[str, Any]]] = defaultdict(list)

    # workflow_runs (1 row)
    wfr_row = build_workflow_run_row(repo, run_id, run_attempt)
    out["workflow_runs"].append(wfr_row)
    head_sha = wfr_row["head_sha"]
    workflow_file = wfr_row["workflow_file"]
    run_created_at = wfr_row["created_at"]
    tier = workflow_tier(wfr_row.get("workflow_name"))

    # All concrete jobs in the attempt.
    api_jobs = fetch_concrete_jobs(repo, run_id, run_attempt)

    if tier == "record_only":
        for api_job in api_jobs:
            out["jobs"].append(
                _tier2_concrete_row(
                    api_job,
                    run_id=run_id,
                    run_attempt=run_attempt,
                    run_created_at=run_created_at,
                )
            )
            _emit_steps_for_job(
                out,
                api_job,
                run_id=run_id,
                run_attempt=run_attempt,
                run_created_at=run_created_at,
            )
        return out, len(api_jobs), tier

    # Tier 1 path: full graph materialization below.

    # YAML at head_sha (data only).
    workflow_spec = fetch_workflow_yaml(repo, workflow_file, head_sha)

    # Group concrete jobs by caller-side YAML key. Each group becomes one
    # graph-node row (caller_node or direct_node).
    children_by_caller: dict[str, list[dict[str, Any]]] = defaultdict(list)
    unresolved: list[dict[str, Any]] = []
    for api_job in api_jobs:
        caller_part, _called_part = split_concrete_name(api_job.get("name", ""))
        caller_key, _gik = resolve_caller_key(caller_part, workflow_spec)
        if caller_key is None:
            unresolved.append(api_job)
            continue
        children_by_caller[caller_key].append(api_job)

    if unresolved:
        sys.stderr.write(
            f"warning: {len(unresolved)} concrete jobs did not resolve to a caller "
            f"YAML key: {[j.get('name') for j in unresolved][:5]}{'...' if len(unresolved) > 5 else ''}\n"
        )

    # Graph-node rows — one per top-level YAML job, whether or not any
    # concrete child ran. Per the plan, every top-level YAML job becomes one
    # logical graph node; jobs whose children never ran still need a row so
    # downstream needs[] edges resolve.
    graph_node_row_key_by_caller: dict[str, str] = {}
    for caller_key, spec in workflow_spec.jobs.items():
        children = children_by_caller.get(caller_key, [])
        gik = ""  # caller-side strategy.matrix not yet supported; empty string per plan
        rk = graph_node_row_key(spec, run_id, run_attempt, workflow_file, gik)
        graph_node_row_key_by_caller[caller_key] = rk

        timing = aggregate_graph_timing(children)
        conclusion_priority = {
            "failure": 0,
            "timed_out": 1,
            "cancelled": 2,
            "skipped": 3,
            "success": 4,
            None: 5,
        }
        # Worst child conclusion in the obvious failure ordering, restricted
        # to children that ran. Three cases for the graph-node conclusion:
        #   - any child ran     → worst conclusion among ran children
        #   - all children skipped → "skipped"
        #   - no children at all  → NULL (job did not produce any concrete
        #                            row in this attempt — e.g. its caller
        #                            was filtered out by `if:`)
        ran_children = [c for c in children if _child_ran(c)]
        if ran_children:
            worst = min(
                (c.get("conclusion") for c in ran_children),
                key=lambda c: conclusion_priority.get(c, 99),
            )
        elif children:
            worst = "skipped"
        else:
            worst = None

        out["jobs"].append(
            {
                "row_key": rk,
                "job_id": None,
                "run_id": run_id,
                "run_attempt": run_attempt,
                "name": spec.name or caller_key,
                "caller_workflow_file": workflow_file,
                "caller_job_key": caller_key,
                "called_workflow_file": _normalize_uses(spec.uses),
                "called_job_key": None,
                "node_kind": graph_node_kind(spec),
                "graph_instance_key": gik,
                # needs[] is the row_keys of upstream graph nodes. Fill in
                # after we know all caller→rk mappings; see fixup pass below.
                "needs": [],
                "conclusion": worst,
                "started_at": timing["started_at"],
                "completed_at": timing["completed_at"],
                "duration_seconds": timing["duration_seconds"],
                "queue_wait_seconds": timing["queue_wait_seconds"],
                "runner_name": None,
                "runner_labels": [],
                "job_type": _classify_job(spec, caller_key),
                "matrix_os": _resolve_matrix_input(spec, "os"),
                "matrix_arch": _resolve_matrix_input(spec, "arch"),
                "matrix_compiler": _resolve_matrix_input(spec, "compiler"),
                "matrix_config": _resolve_matrix_input(spec, "config"),
                "matrix_gpu_tier": _resolve_matrix_input(spec, "gpu-tier"),
                "run_created_at": run_created_at,
            }
        )

    # Fixup pass: resolve needs[] for graph nodes (caller-key references → row_key references).
    for row in out["jobs"]:
        caller_key = row["caller_job_key"]
        spec = workflow_spec.jobs.get(caller_key)
        if spec is None:
            continue
        resolved = []
        for needed_key in spec.needs:
            target_rk = graph_node_row_key_by_caller.get(needed_key)
            if target_rk is not None:
                resolved.append(target_rk)
            else:
                sys.stderr.write(
                    f"warning: needs[] target {needed_key!r} from {caller_key!r} "
                    f"did not resolve to a graph node\n"
                )
        row["needs"] = resolved

    # Concrete-job rows + steps.
    for caller_key, children in children_by_caller.items():
        graph_rk = graph_node_row_key_by_caller[caller_key]
        for api_job in children:
            job_id = api_job["id"]
            out["jobs"].append(
                {
                    "row_key": rk_concrete(job_id),
                    "job_id": job_id,
                    "run_id": run_id,
                    "run_attempt": run_attempt,
                    "name": api_job.get("name"),
                    "caller_workflow_file": workflow_file,
                    "caller_job_key": caller_key,
                    "called_workflow_file": _normalize_uses(
                        workflow_spec.jobs[caller_key].uses
                    ),
                    "called_job_key": split_concrete_name(api_job.get("name", ""))[1],
                    "node_kind": "concrete_job",
                    # graph_instance_key NULL for concrete rows per the plan
                    "graph_instance_key": None,
                    "needs": [graph_rk],
                    "conclusion": api_job.get("conclusion"),
                    "started_at": api_job.get("started_at"),
                    "completed_at": api_job.get("completed_at"),
                    "duration_seconds": _duration(api_job),
                    "queue_wait_seconds": _queue_wait(api_job),
                    "runner_name": api_job.get("runner_name"),
                    "runner_labels": api_job.get("labels") or [],
                    "job_type": _classify_job(workflow_spec.jobs[caller_key], caller_key),
                    "matrix_os": _resolve_matrix_input(workflow_spec.jobs[caller_key], "os"),
                    "matrix_arch": _resolve_matrix_input(workflow_spec.jobs[caller_key], "arch"),
                    "matrix_compiler": _resolve_matrix_input(workflow_spec.jobs[caller_key], "compiler"),
                    "matrix_config": _resolve_matrix_input(workflow_spec.jobs[caller_key], "config"),
                    "matrix_gpu_tier": _resolve_matrix_input(workflow_spec.jobs[caller_key], "gpu-tier"),
                    "run_created_at": run_created_at,
                }
            )

            _emit_steps_for_job(
                out,
                api_job,
                run_id=run_id,
                run_attempt=run_attempt,
                run_created_at=run_created_at,
            )

    return out, len(api_jobs), tier


def _normalize_uses(uses: str | None) -> str | None:
    """Map `uses:` string to a repo-relative workflow path.

    GH Actions `uses:` for a same-repo reusable workflow looks like
    `./.github/workflows/foo.yml`. Strip the leading `./` to match the
    `caller_workflow_file` shape (`.github/workflows/ci.yml`). External
    `owner/repo/.github/...@ref` `uses:` strings are returned as-is.
    """
    if uses is None:
        return None
    if uses.startswith("./"):
        return uses[2:]
    return uses


def _duration(j: dict[str, Any]) -> float | None:
    """Wall-clock seconds between started_at and completed_at.

    Returns None when either timestamp is missing or for skipped jobs (GH
    reports nominal timestamps for skipped jobs that often differ by 1 second
    in the wrong order, yielding a negative duration). Treating these as NULL
    matches the BigQuery convention for "this job did not run".
    """
    if j.get("conclusion") == "skipped":
        return None
    st = parse_ts(j.get("started_at"))
    cp = parse_ts(j.get("completed_at"))
    if st and cp:
        d = (cp - st).total_seconds()
        return d if d >= 0 else None
    return None


def _queue_wait(j: dict[str, Any]) -> float | None:
    if j.get("conclusion") == "skipped":
        return None
    cr = parse_ts(j.get("created_at"))
    st = parse_ts(j.get("started_at"))
    if cr and st:
        d = (st - cr).total_seconds()
        return d if d >= 0 else None
    return None


def _classify_job(spec: JobSpec, caller_key: str) -> str:
    """Heuristic job_type from caller key / called workflow."""
    key_text = (spec.uses or caller_key).lower()
    if "build" in key_text:
        return "build"
    if "test" in key_text:
        return "test"
    if "report" in key_text or "publish" in key_text:
        return "report"
    return "other"


def _resolve_matrix_input(spec: JobSpec, key: str) -> str | None:
    """Best-effort matrix-axis resolution.

    Reads from `with:` inputs (Slang's CI uses this exclusively) when the value
    is a literal string; expressions like `${{ matrix.config }}` are not
    resolved here. Returns None if not present or not a literal scalar.
    """
    val = spec.with_inputs.get(key)
    if isinstance(val, (str, int, bool)):
        s = str(val)
        if "${{" in s:
            return None
        return s
    return None


# ---------- Validation ----------


def validate(
    rows: dict[str, list[dict[str, Any]]],
    *,
    api_job_count: int | None = None,
    tier: str = "parse_graph",
) -> list[str]:
    """Structural sanity checks on emitted rows.

    `api_job_count`, when provided, is the number of concrete jobs the GH API
    returned for this attempt. We use it to catch the silent-empty-output
    regression where every concrete job goes unresolved → 0 graph rows emit
    → validation passes despite zero useful data.

    `tier` selects the rule set: 'parse_graph' (Tier 1) requires every
    concrete row to have exactly one needs[] entry pointing at its graph
    node, while 'record_only' (Tier 2) expects no graph rows at all and
    empty needs[] on concrete rows.
    """
    errors: list[str] = []
    jobs = rows.get("jobs", [])
    graph_row_keys = {r["row_key"] for r in jobs if r["node_kind"] != "concrete_job"}
    concrete_row_count = sum(1 for r in jobs if r["node_kind"] == "concrete_job")

    if api_job_count is not None and api_job_count > 0 and concrete_row_count == 0:
        errors.append(
            f"GH API reported {api_job_count} concrete jobs but the ingester "
            "emitted zero concrete_job rows — likely a caller-key resolution "
            "regression. Re-check resolve_caller_key for this workflow's "
            "matrix expansion pattern."
        )

    if tier == "record_only" and graph_row_keys:
        errors.append(
            f"Tier 2 (record_only) ingest emitted {len(graph_row_keys)} graph "
            "rows; expected 0. Tier 2 must not materialize graph nodes."
        )

    # Per-tier graph-field NULL contract on concrete rows. Tier 2's whole
    # point is that these fields are unset — leaking a Tier 1-style value
    # into a Tier 2 row would corrupt the "rows with NULL graph_instance_key
    # are the lightweight tier" assumption downstream queries can rely on.
    TIER2_NULL_FIELDS = (
        "caller_workflow_file",
        "caller_job_key",
        "called_workflow_file",
        "called_job_key",
        "graph_instance_key",
        "job_type",
        "matrix_os",
        "matrix_arch",
        "matrix_compiler",
        "matrix_config",
        "matrix_gpu_tier",
    )

    for r in jobs:
        if r["node_kind"] == "concrete_job":
            if tier == "record_only":
                if r["needs"]:
                    errors.append(
                        f"Tier 2 concrete row {r['job_id']} ({r['name']}) has "
                        f"needs[]={r['needs']}, expected empty list."
                    )
                leaked = [
                    f for f in TIER2_NULL_FIELDS if r.get(f) is not None
                ]
                if leaked:
                    errors.append(
                        f"Tier 2 concrete row {r['job_id']} ({r['name']}) has "
                        f"non-NULL graph fields: "
                        + ", ".join(f"{k}={r[k]!r}" for k in leaked)
                    )
                continue
            if not r["needs"]:
                errors.append(
                    f"concrete row {r['job_id']} ({r['name']}) has empty needs[]"
                )
            elif len(r["needs"]) != 1:
                errors.append(
                    f"concrete row {r['job_id']} ({r['name']}) has needs[]={r['needs']}, "
                    "expected exactly one element (its graph node)"
                )
            elif r["needs"][0] not in graph_row_keys:
                errors.append(
                    f"concrete row {r['job_id']} ({r['name']}) needs[] target "
                    f"{r['needs'][0]} is not a graph-node row_key in this dump"
                )
        else:
            for n in r["needs"]:
                if n not in graph_row_keys:
                    errors.append(
                        f"graph row {r['caller_job_key']} needs[] target {n} "
                        "is not a graph-node row_key in this dump"
                    )

            # canonical_matrix_key shape (empty or sorted key=val,key=val)
            gik = r["graph_instance_key"]
            if gik:
                parts = gik.split(",")
                keys = [p.split("=", 1)[0] for p in parts if "=" in p]
                if keys != sorted(keys):
                    errors.append(
                        f"graph row {r['caller_job_key']} graph_instance_key "
                        f"{gik!r} is not sorted"
                    )

    return errors


# ---------- CLI ----------


def emit(rows: dict[str, list[dict[str, Any]]], table: str) -> None:
    if table == "all":
        for tbl in ("workflow_runs", "jobs", "job_steps"):
            for r in rows.get(tbl, []):
                sys.stdout.write(json.dumps({"_table": tbl, **r}))
                sys.stdout.write("\n")
        return
    for r in rows.get(table, []):
        sys.stdout.write(json.dumps(r))
        sys.stdout.write("\n")


def parse_args(argv: list[str]) -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--run-id", type=int, required=True)
    p.add_argument("--run-attempt", type=int, required=True)
    p.add_argument("--repo", default="shader-slang/slang")
    p.add_argument(
        "--table",
        choices=("workflow_runs", "jobs", "job_steps", "all"),
        default="all",
    )
    p.add_argument(
        "--validate",
        action="store_true",
        help="Run structural sanity checks; exit non-zero on any error.",
    )
    p.add_argument(
        "--load-to-bq",
        action="store_true",
        help=(
            "After building rows (and validating, if --validate is set), load "
            "them into per-attempt staging tables and atomically MERGE into "
            "the live slang_ci dataset. Staging tables are dropped on "
            "success and failure. No-ops --table emission to stdout."
        ),
    )
    return p.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    rows, api_job_count, tier = build_rows(args.repo, args.run_id, args.run_attempt)

    if args.validate:
        errs = validate(rows, api_job_count=api_job_count, tier=tier)
        if errs:
            for e in errs:
                sys.stderr.write(f"validation error: {e}\n")
            return 1
        sys.stderr.write(
            f"validation OK [{tier}]: {len(rows.get('workflow_runs', []))} workflow_runs, "
            f"{len(rows.get('jobs', []))} jobs ("
            f"{sum(1 for r in rows.get('jobs', []) if r['node_kind'] != 'concrete_job')} graph, "
            f"{sum(1 for r in rows.get('jobs', []) if r['node_kind'] == 'concrete_job')} concrete), "
            f"{len(rows.get('job_steps', []))} job_steps\n"
        )

    if args.load_to_bq:
        load_rows_to_bq(
            rows,
            src_run_id=args.run_id,
            src_run_attempt=args.run_attempt,
        )
        sys.stderr.write(
            f"loaded run_id={args.run_id} run_attempt={args.run_attempt} "
            f"into slang-runners:slang_ci\n"
        )
        return 0

    emit(rows, args.table)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
