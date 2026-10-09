#!/usr/bin/env python3
"""
Shared helpers for the bot-CI priority-gate scripts.

`wait-for-priority.py` (the gate) and `retry-yielded-bot-ci.py` (the retry) both
need to identify the throttled bot account, read the actor of a workflow run,
and list active CI runs. Keeping those in one place stops the two scripts from
drifting apart — e.g. one updating the bot-login set or the active-status set
without the other.
"""

import os
import sys
from datetime import datetime

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gh_api import gh_api_list

DEFAULT_REPO = "shader-slang/slang"
DEFAULT_WORKFLOW = "ci.yml"

# Logins whose CI is throttled. Anything ending in "[bot]" also counts (see is_bot).
DEFAULT_BOT_LOGINS = {
    "nv-slang-bot",
    "nv-slang-bot[bot]",
}

# Candidate statuses. Waiting runs need a job-level capacity check below.
ACTIVE_STATUSES = {"queued", "in_progress", "waiting", "requested", "pending"}


def waiting_run_uses_runner_capacity(repo, run):
    """Check jobs before treating an approval-waiting run as a blocker.

    For example, a run can have all its build/test jobs completed while its
    Falcor gate still awaits approval. It consumes no runner capacity and
    must not prevent bot CI or retries. A waiting run with a queued/running
    sibling still blocks. Empty or unknown job state is kept conservatively.
    """
    jobs, err = gh_api_list(
        f"/repos/{repo}/actions/runs/{run['id']}/jobs?per_page=100", "jobs"
    )
    if err:
        raise RuntimeError(f"Failed to list jobs for waiting run {run['id']}: {err}")
    return not jobs or any(
        job.get("status") not in {"completed", "waiting"} for job in jobs
    )


def normalize_bot_logins(extra_logins=None):
    """Return the lower-cased set of throttled bot logins, plus any extra logins."""
    bot_logins = {login.lower() for login in DEFAULT_BOT_LOGINS}
    bot_logins.update((login or "").lower() for login in (extra_logins or []))
    bot_logins.discard("")
    return bot_logins


def is_bot(login, bot_logins):
    """True if login is throttled: any "[bot]" account, or one in bot_logins."""
    if not login:
        return False
    login = login.lower()
    return login.endswith("[bot]") or login in bot_logins


def run_actor_login(run):
    """Best-effort login of whoever caused the run (triggering_actor, then actor)."""
    for key in ("triggering_actor", "actor"):
        actor = run.get(key) or {}
        login = actor.get("login")
        if login:
            return login
    return ""


def fetch_active_runs(repo, workflow, include_run=None):
    """Find sufficient evidence that relevant CI still needs runner capacity.

    The gate and retry scheduler need a busy/quiet decision, not an exhaustive
    inventory. Query runner-active statuses first and stop on a relevant run.
    Only when those are quiet, inspect waiting runs one at a time, stopping at
    the first runnable sibling. For example, an active human build makes it
    unnecessary to inspect a backlog of 100 Falcor approval requests.

    The gate supplies include_run to exclude itself and newer bot runs before
    any job requests. The retry scheduler considers every run. A quiet result
    still requires checking all relevant waiting runs; missing/API-error state
    must never be interpreted as permission to proceed.
    """
    def relevant(run):
        return include_run is None or include_run(run)

    endpoint = f"/repos/{repo}/actions/workflows/{workflow}/runs"
    for status in sorted(ACTIVE_STATUSES - {"waiting"}):
        items, err = gh_api_list(
            f"{endpoint}?status={status}&per_page=100", "workflow_runs"
        )
        if err:
            raise RuntimeError(f"Failed to list {status} runs: {err}")
        active = [run for run in items or [] if relevant(run)]
        if active:
            return active

    waiting, err = gh_api_list(
        f"{endpoint}?status=waiting&per_page=100", "workflow_runs"
    )
    if err:
        raise RuntimeError(f"Failed to list waiting runs: {err}")
    for run in waiting or []:
        if relevant(run) and waiting_run_uses_runner_capacity(repo, run):
            return [run]
    return []


def parse_github_time(value):
    """Parse a GitHub ISO-8601 timestamp (trailing 'Z') into an aware datetime."""
    if not value:
        return None
    return datetime.fromisoformat(value.replace("Z", "+00:00"))
