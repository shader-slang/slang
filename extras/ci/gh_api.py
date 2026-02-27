"""
Shared GitHub API and JSON helpers for CI scripts.

Uses the `gh` CLI for authenticated API access. Handles pagination
correctly for GitHub's concatenated JSON output format.
"""

import json
import subprocess
import sys
import time

MAX_RETRIES = 3
RETRY_BACKOFF_SECONDS = 2
GH_COMMAND_TIMEOUT = 120  # seconds per attempt


def _is_retryable_error(stderr):
    text = (stderr or "").lower()
    retry_markers = [
        "timed out",
        "timeout",
        "connection reset",
        "temporary failure",
        "service unavailable",
        "bad gateway",
        "gateway timeout",
        "secondary rate limit",
        "api rate limit exceeded",
    ]
    return any(marker in text for marker in retry_markers)


def _run_gh_command(cmd):
    """Run a gh command with retry for transient failures."""
    last_result = None
    for attempt in range(MAX_RETRIES):
        try:
            result = subprocess.run(
                cmd, capture_output=True, text=True, timeout=GH_COMMAND_TIMEOUT
            )
        except subprocess.TimeoutExpired:
            # Treat timeout as retryable
            if attempt == MAX_RETRIES - 1:
                # Return a fake failed result
                return subprocess.CompletedProcess(
                    cmd, 1, stdout="", stderr="Command timed out"
                )
            time.sleep(RETRY_BACKOFF_SECONDS * (attempt + 1))
            continue
        last_result = result
        if result.returncode == 0:
            return result
        if not _is_retryable_error(result.stderr) or attempt == MAX_RETRIES - 1:
            return result
        time.sleep(RETRY_BACKOFF_SECONDS * (attempt + 1))
    return last_result


def gh_api(endpoint):
    """Call gh api (single request) and return (parsed_json, error_string)."""
    cmd = ["gh", "api", endpoint]
    result = _run_gh_command(cmd)
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
    result = _run_gh_command(cmd)
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
