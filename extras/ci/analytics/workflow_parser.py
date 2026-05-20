"""Workflow YAML parser for BigQuery ingestion.

Extracts only the declared fields the plan whitelists:

    jobs.<key>.name
    jobs.<key>.needs
    jobs.<key>.strategy.matrix
    jobs.<key>.uses
    jobs.<key>.with
    on.workflow_call.inputs.*       (called workflow side)

The parser does NOT evaluate `${{ ... }}` expressions. Matrix-axis values that
contain expressions are recorded as raw strings; the ingester resolves them by
joining against the concrete child job's API-reported matrix values.

Inputs are fetched as data (never checked out) via gh api contents. The result
is cached by (workflow_file, head_sha) so the same YAML is not re-fetched per
attempt.
"""

from __future__ import annotations

import base64
from dataclasses import dataclass, field
from functools import lru_cache
from typing import Any

import yaml

from _bq_ingest_lib import gh_api


@dataclass
class JobSpec:
    """Parsed top-level YAML job entry."""

    job_key: str
    name: str | None
    needs: list[str] = field(default_factory=list)
    matrix: dict[str, list[Any]] = field(default_factory=dict)
    uses: str | None = None  # e.g. "./.github/workflows/ci-slang-build.yml"
    with_inputs: dict[str, Any] = field(default_factory=dict)

    @property
    def is_reusable_call(self) -> bool:
        return self.uses is not None

    @property
    def is_matrix(self) -> bool:
        return bool(self.matrix)


@dataclass
class WorkflowSpec:
    """Parsed workflow YAML."""

    workflow_file: str  # e.g. ".github/workflows/ci.yml"
    name: str | None
    jobs: dict[str, JobSpec] = field(default_factory=dict)
    # For called workflows: declared inputs (just names + default values, not types).
    workflow_call_inputs: dict[str, Any] = field(default_factory=dict)


MAX_WORKFLOW_BYTES = 256 * 1024
MAX_JOBS_PER_WORKFLOW = 1000


def _parse_workflow_yaml(raw: str, workflow_file: str) -> WorkflowSpec:
    if len(raw) > MAX_WORKFLOW_BYTES:
        raise ValueError(
            f"workflow {workflow_file} exceeds {MAX_WORKFLOW_BYTES} byte cap"
        )
    # yaml.safe_load only — never yaml.load (unsafe).
    doc = yaml.safe_load(raw) or {}

    spec = WorkflowSpec(workflow_file=workflow_file, name=doc.get("name"))

    # Called-side inputs.
    on_clause = doc.get("on") or doc.get(True) or {}
    if isinstance(on_clause, dict):
        wc = on_clause.get("workflow_call") or {}
        if isinstance(wc, dict):
            inputs = wc.get("inputs") or {}
            if isinstance(inputs, dict):
                spec.workflow_call_inputs = {
                    k: (v.get("default") if isinstance(v, dict) else None)
                    for k, v in inputs.items()
                }

    jobs = doc.get("jobs") or {}
    if not isinstance(jobs, dict):
        return spec
    if len(jobs) > MAX_JOBS_PER_WORKFLOW:
        raise ValueError(
            f"workflow {workflow_file} has {len(jobs)} jobs, cap is "
            f"{MAX_JOBS_PER_WORKFLOW}"
        )

    for job_key, body in jobs.items():
        if not isinstance(body, dict):
            continue
        js = JobSpec(job_key=job_key, name=body.get("name"))

        needs = body.get("needs")
        if isinstance(needs, str):
            js.needs = [needs]
        elif isinstance(needs, list):
            js.needs = [str(n) for n in needs if isinstance(n, (str, int))]

        strategy = body.get("strategy") or {}
        if isinstance(strategy, dict):
            matrix = strategy.get("matrix")
            if isinstance(matrix, dict):
                # Take only scalar list-axes; ignore include/exclude for the
                # vertical slice. (Slang's CI uses `with:` for matrix-like
                # axes today, so this is mostly defensive.)
                js.matrix = {
                    k: v
                    for k, v in matrix.items()
                    if k not in ("include", "exclude") and isinstance(v, list)
                }

        uses = body.get("uses")
        if isinstance(uses, str):
            js.uses = uses

        with_block = body.get("with")
        if isinstance(with_block, dict):
            js.with_inputs = with_block

        spec.jobs[job_key] = js

    return spec


@lru_cache(maxsize=64)
def fetch_workflow_yaml(repo: str, workflow_file: str, head_sha: str) -> WorkflowSpec:
    """Fetch a workflow file at the given commit as DATA (no checkout) and parse.

    Cached by (workflow_file, head_sha) so the same YAML isn't re-fetched per
    attempt or per job that references it.
    """
    payload = gh_api(f"repos/{repo}/contents/{workflow_file}?ref={head_sha}")
    if payload.get("encoding") != "base64":
        raise ValueError(
            f"unexpected encoding {payload.get('encoding')!r} for {workflow_file}"
        )
    raw = base64.b64decode(payload["content"]).decode("utf-8")
    return _parse_workflow_yaml(raw, workflow_file)
