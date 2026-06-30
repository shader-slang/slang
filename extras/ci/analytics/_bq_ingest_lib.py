"""Shared helpers for the BigQuery ingester and workflow parser.

Kept separate from the CLI entry points so workflow_parser.py can be unit-tested
without importing argparse / GH-API plumbing.
"""

from __future__ import annotations

import hashlib
import json
import subprocess
from typing import Any


# Tier 1 (`parse_graph`) workflows: full YAML fetch + graph-node materialization
# + (eventually) build_stats artifacts. Matched by display `name:` field, not
# filename — that's what GitHub's `on: workflow_run:` clause uses.
#
# Everything else that reaches the ingester is Tier 2 (`record_only`): concrete
# jobs only, NULL graph columns. The `ci-telemetry.yml` `on: workflow_run:`
# filter is the union of both tiers; the dispatch happens here.
#
# MaterialX Integration Tests is intentionally NOT in Tier 1: it's
# workflow_call:-only, so it never fires its own workflow_run event. Its jobs
# arrive transitively via CI's caller_node rows.
PARSE_GRAPH_WORKFLOWS: frozenset[str] = frozenset({
    "CI",
    "CMake Options",
    "Falcor Tests",
    "Falcor Compiler Perf-Test",
    "RTX Remix Shaders (Nightly)",
    "Sascha Willems Vulkan Shaders (Nightly)",
    "VK-GL-CTS Nightly",
})


def workflow_tier(workflow_name: str | None) -> str:
    """Return 'parse_graph' or 'record_only' for a workflow's display name."""
    if workflow_name in PARSE_GRAPH_WORKFLOWS:
        return "parse_graph"
    return "record_only"


def gh_api(path: str) -> dict[str, Any]:
    """Single GH API call returning parsed JSON."""
    proc = subprocess.run(
        ["gh", "api", path],
        check=True,
        capture_output=True,
        text=True,
    )
    return json.loads(proc.stdout)


def gh_api_paginated(path: str, list_key: str) -> list[dict[str, Any]]:
    """Paginated GH API walk. `path` may already contain a query string.

    Uses `gh api --paginate` which handles Link headers transparently and
    concatenates the named list field across pages.
    """
    sep = "&" if "?" in path else "?"
    full_path = f"{path}{sep}per_page=100"
    proc = subprocess.run(
        ["gh", "api", "--paginate", full_path],
        check=True,
        capture_output=True,
        text=True,
    )
    # --paginate emits one JSON object per page concatenated.
    out: list[dict[str, Any]] = []
    decoder = json.JSONDecoder()
    text = proc.stdout.strip()
    idx = 0
    while idx < len(text):
        obj, end = decoder.raw_decode(text, idx)
        out.extend(obj.get(list_key, []))
        idx = end
        while idx < len(text) and text[idx].isspace():
            idx += 1
    return out


def sha256_hex(s: str) -> str:
    return hashlib.sha256(s.encode()).hexdigest()


# Row-key formulas — one place to look when chasing schema bugs.

def rk_workflow_run(run_id: int, run_attempt: int) -> str:
    return sha256_hex(f"workflow_run:{run_id}:{run_attempt}")


def rk_concrete(job_id: int) -> str:
    return sha256_hex(f"concrete:{job_id}")


def rk_caller_node(
    run_id: int,
    run_attempt: int,
    caller_workflow_file: str,
    caller_job_key: str,
    graph_instance_key: str,
) -> str:
    return sha256_hex(
        f"caller:{run_id}:{run_attempt}:{caller_workflow_file}:"
        f"{caller_job_key}:{graph_instance_key}"
    )


def rk_direct_node(
    run_id: int,
    run_attempt: int,
    caller_workflow_file: str,
    caller_job_key: str,
    graph_instance_key: str,
) -> str:
    return sha256_hex(
        f"direct:{run_id}:{run_attempt}:{caller_workflow_file}:"
        f"{caller_job_key}:{graph_instance_key}"
    )


def rk_step(job_id: int, step_number: int) -> str:
    return sha256_hex(f"step:{job_id}:{step_number}")


def canonical_matrix_key(matrix: dict[str, Any]) -> str:
    """Canonical-stringified caller-side matrix tuple — `key=val,key=val`,
    keys sorted alphabetically, values verbatim. Empty string when no matrix.

    The plan specifies this exact shape so row_keys are stable across ingester
    runs and across rewrites.
    """
    if not matrix:
        return ""
    parts = [f"{k}={matrix[k]}" for k in sorted(matrix)]
    return ",".join(parts)
