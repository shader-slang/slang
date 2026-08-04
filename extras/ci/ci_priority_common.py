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

# Run statuses that mean a run still holds, or is waiting for, runner capacity.
ACTIVE_STATUSES = {"queued", "in_progress", "waiting", "requested", "pending"}


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


def fetch_active_runs(repo, workflow):
    """Return active CI runs for the workflow across all active statuses.

    Queries every status in ACTIVE_STATUSES and paginates each query, so a
    higher-priority run is never missed because it sits in a less-common state
    or on a later page.
    """
    runs = {}
    for status in sorted(ACTIVE_STATUSES):
        items, err = gh_api_list(
            f"/repos/{repo}/actions/workflows/{workflow}/runs"
            f"?status={status}&per_page=100",
            "workflow_runs",
        )
        if err:
            raise RuntimeError(f"Failed to list {status} runs: {err}")
        for run in items or []:
            runs[run["id"]] = run
    return list(runs.values())


def parse_github_time(value):
    """Parse a GitHub ISO-8601 timestamp (trailing 'Z') into an aware datetime."""
    if not value:
        return None
    return datetime.fromisoformat(value.replace("Z", "+00:00"))
