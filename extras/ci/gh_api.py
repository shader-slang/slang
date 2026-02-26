"""
Shared GitHub API and JSON helpers for CI scripts.

Uses the `gh` CLI for authenticated API access. Handles pagination
correctly for GitHub's concatenated JSON output format.
"""

import json
import subprocess
import sys


def gh_api(endpoint):
    """Call gh api (single request) and return (parsed_json, error_string)."""
    cmd = ["gh", "api", endpoint]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        return None, result.stderr.strip()
    try:
        return json.loads(result.stdout), None
    except json.JSONDecodeError as e:
        return None, f"JSON parse error: {e}"


def parse_json_stream(payload):
    """Parse concatenated JSON objects from gh --paginate output.

    gh api --paginate concatenates multiple JSON response objects rather
    than merging them. This generator yields each parsed object.
    """
    decoder = json.JSONDecoder()
    idx = 0
    length = len(payload)
    while True:
        while idx < length and payload[idx].isspace():
            idx += 1
        if idx >= length:
            break
        obj, idx = decoder.raw_decode(payload, idx)
        yield obj


def gh_api_list(endpoint, key):
    """Call gh api --paginate and return a merged list for a given key.

    Returns (items_list, error_string). The key parameter specifies
    which field to extract and merge from each paginated response
    (e.g. "jobs", "workflow_runs", "runners").
    """
    cmd = ["gh", "api", "--paginate", endpoint]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        return None, result.stderr.strip()

    items = []
    try:
        for obj in parse_json_stream(result.stdout):
            if isinstance(obj, dict):
                items.extend(obj.get(key, []))
            elif isinstance(obj, list):
                items.extend(obj)
    except json.JSONDecodeError as e:
        return None, f"JSON parse error: {e}"
    return items, None


def coerce_jobs_data(data):
    """Normalize various input formats into a flat jobs list."""
    if isinstance(data, dict):
        if "jobs" in data:
            return data["jobs"]
        # gh --jq '.jobs[]' emits one job object per line
        if "name" in data and ("started_at" in data or "completed_at" in data):
            return [data]
        return []
    if isinstance(data, list):
        return data
    return []


def load_paginated_stdin():
    """Read stdin and handle gh --paginate concatenated JSON output.

    Handles both single JSON objects/arrays and concatenated JSON
    from gh api --paginate. Returns a flat list of job dicts.
    """
    payload = sys.stdin.read()
    if not payload.strip():
        return []

    try:
        # Fast path: valid JSON (single object or array)
        data = json.loads(payload)
        return coerce_jobs_data(data)
    except json.JSONDecodeError:
        # Slow path: concatenated JSON objects from --paginate
        jobs_data = []
        try:
            for obj in parse_json_stream(payload):
                jobs_data.extend(coerce_jobs_data(obj))
        except json.JSONDecodeError:
            pass
        return jobs_data
