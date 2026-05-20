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
    """Parsed top-level YAML job entry. All fields are scalar-or-None after
    parser normalization; nested structures are dropped before this point.
    """

    job_key: str
    name: str | None
    needs: list[str] = field(default_factory=list)
    matrix: dict[str, list[str]] = field(default_factory=dict)
    uses: str | None = None  # e.g. "./.github/workflows/ci-slang-build.yml"
    with_inputs: dict[str, str] = field(default_factory=dict)

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
    # For called workflows: declared inputs (name → scalar default or None).
    workflow_call_inputs: dict[str, str | None] = field(default_factory=dict)


MAX_WORKFLOW_BYTES = 256 * 1024
MAX_JOBS_PER_WORKFLOW = 1000


def _safe_str(v: Any) -> str | None:
    """Return v as a string if it's a scalar type (str/int/bool/float),
    otherwise None. The plan's whitelist treats every extracted YAML field as
    a scalar; non-scalars survive `safe_load` (nested dicts, lists, None) and
    must be rejected before they reach BigQuery.
    """
    if isinstance(v, str):
        return v
    if isinstance(v, bool):
        # bool is a subclass of int; check first so we don't lose `true/false`
        return "true" if v else "false"
    if isinstance(v, (int, float)):
        return str(v)
    return None


def _parse_workflow_yaml(raw: str, workflow_file: str) -> WorkflowSpec:
    # Cap is on the raw (post-decode) string — base64-decoded content can be
    # much larger than the on-wire payload.
    if len(raw) > MAX_WORKFLOW_BYTES:
        raise ValueError(
            f"workflow {workflow_file} exceeds {MAX_WORKFLOW_BYTES} byte cap "
            f"(saw {len(raw)} bytes)"
        )
    # yaml.safe_load only — never yaml.load (unsafe constructors).
    doc = yaml.safe_load(raw) or {}
    if not isinstance(doc, dict):
        raise ValueError(
            f"workflow {workflow_file} top-level is not a mapping "
            f"(got {type(doc).__name__})"
        )

    spec = WorkflowSpec(workflow_file=workflow_file, name=_safe_str(doc.get("name")))

    # Called-side inputs. PyYAML maps the bare `on:` key to Python True; both
    # spellings show up in the wild depending on the YAML version.
    on_clause = doc.get("on") or doc.get(True) or {}
    if isinstance(on_clause, dict):
        wc = on_clause.get("workflow_call") or {}
        if isinstance(wc, dict):
            inputs = wc.get("inputs") or {}
            if isinstance(inputs, dict):
                spec.workflow_call_inputs = {
                    k: (
                        _safe_str(v.get("default"))
                        if isinstance(v, dict)
                        else _safe_str(v)
                    )
                    for k, v in inputs.items()
                    if isinstance(k, str)
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
        if not isinstance(job_key, str) or not isinstance(body, dict):
            continue
        js = JobSpec(job_key=job_key, name=_safe_str(body.get("name")))

        needs = body.get("needs")
        if isinstance(needs, str):
            js.needs = [needs]
        elif isinstance(needs, list):
            js.needs = [n for n in (_safe_str(x) for x in needs) if n]

        strategy = body.get("strategy") or {}
        if isinstance(strategy, dict):
            matrix = strategy.get("matrix")
            if isinstance(matrix, dict):
                # Only scalar list-axes survive; ignore include/exclude (full
                # matrix expansion is future work) and any axis whose values
                # are not all scalars.
                cleaned: dict[str, list[Any]] = {}
                for k, v in matrix.items():
                    if not isinstance(k, str) or k in ("include", "exclude"):
                        continue
                    if not isinstance(v, list):
                        continue
                    scalar_vals = [_safe_str(item) for item in v]
                    if all(s is not None for s in scalar_vals):
                        cleaned[k] = scalar_vals
                js.matrix = cleaned

        uses = body.get("uses")
        if isinstance(uses, str):
            js.uses = uses

        with_block = body.get("with")
        if isinstance(with_block, dict):
            # Drop non-scalar values; the parser doesn't try to flatten lists
            # or nested mappings. Values containing `${{ ... }}` expressions
            # are kept verbatim (resolved later against API job data).
            cleaned_with: dict[str, str] = {}
            for k, v in with_block.items():
                if not isinstance(k, str):
                    continue
                s = _safe_str(v)
                if s is not None:
                    cleaned_with[k] = s
            js.with_inputs = cleaned_with

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
