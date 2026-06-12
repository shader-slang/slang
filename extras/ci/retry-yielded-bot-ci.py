#!/usr/bin/env python3
"""
Rerun bot CI runs that intentionally yielded to higher-priority CI.

This script is designed for a short-lived workflow_run/scheduled workflow. It
does not wait or poll. Each invocation checks whether the CI workflow is quiet;
if so, it reruns at most one recent bot-authored pull-request run whose priority
gate failed in the dedicated "Stop yielded bot CI" marker step.
"""

import argparse
import os
import subprocess
import sys
from datetime import datetime, timedelta, timezone

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gh_api import gh_api_list

DEFAULT_REPO = "shader-slang/slang"
DEFAULT_WORKFLOW = "ci.yml"

DEFAULT_BOT_LOGINS = {
    "nv-slang-bot",
    "nv-slang-bot[bot]",
}

ACTIVE_STATUSES = {"queued", "in_progress", "waiting", "requested", "pending"}


GATE_JOB_NAME = "wait-for-human-priority"
YIELDED_STEP_NAME = "Stop yielded bot CI"
CHECK_CI_JOB_NAME = "check-ci"
RERUNNABLE_CONCLUSIONS = {"failure", "cancelled"}


def normalize_bot_logins(extra_logins=None):
    bot_logins = {login.lower() for login in DEFAULT_BOT_LOGINS}
    bot_logins.update((login or "").lower() for login in (extra_logins or []))
    bot_logins.discard("")
    return bot_logins


def is_bot(login, bot_logins):
    if not login:
        return False
    login = login.lower()
    return login.endswith("[bot]") or login in bot_logins


def run_actor_login(run):
    for key in ("triggering_actor", "actor"):
        actor = run.get(key) or {}
        login = actor.get("login")
        if login:
            return login
    return ""


def fetch_active_runs(repo, workflow):
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


def any_active_ci(runs):
    return [run for run in runs if run.get("status") in ACTIVE_STATUSES]


def parse_github_time(value):
    if not value:
        return None
    return datetime.fromisoformat(value.replace("Z", "+00:00"))


def fetch_recent_completed_runs(repo, workflow):
    runs, err = gh_api_list(
        f"/repos/{repo}/actions/workflows/{workflow}/runs"
        "?status=completed&event=pull_request&per_page=100",
        "workflow_runs",
    )
    if err:
        raise RuntimeError(f"Failed to list completed pull-request runs: {err}")
    return runs or []


def fetch_jobs(repo, run_id):
    jobs, err = gh_api_list(
        f"/repos/{repo}/actions/runs/{run_id}/jobs?per_page=100",
        "jobs",
    )
    if err:
        raise RuntimeError(f"Failed to list jobs for run {run_id}: {err}")
    return jobs or []


def yielded_marker_failed(job):
    if job.get("name") != GATE_JOB_NAME:
        return False
    if job.get("conclusion") not in RERUNNABLE_CONCLUSIONS:
        return False
    for step in job.get("steps") or []:
        if (
            step.get("name") == YIELDED_STEP_NAME
            and step.get("conclusion") in RERUNNABLE_CONCLUSIONS
        ):
            return True
    return False


def failed_only_because_priority_gate(jobs):
    """Return true only when the yielded marker caused the run failure."""
    found_yielded_marker = False
    for job in jobs:
        name = job.get("name")
        conclusion = job.get("conclusion")
        if yielded_marker_failed(job):
            found_yielded_marker = True
            continue
        if name == CHECK_CI_JOB_NAME and conclusion in RERUNNABLE_CONCLUSIONS:
            continue
        if conclusion in RERUNNABLE_CONCLUSIONS:
            return False
    return found_yielded_marker


def has_newer_run_for_branch(run, runs):
    branch = run.get("head_branch")
    run_number = run.get("run_number", 0)
    if not branch:
        return False
    for other in runs:
        if other.get("id") == run.get("id"):
            continue
        if other.get("head_branch") != branch:
            continue
        if other.get("run_number", 0) > run_number:
            return True
    return False


def yielded_bot_candidates(runs, repo, bot_logins, lookback_hours, max_attempts):
    now = datetime.now(timezone.utc)
    cutoff = now - timedelta(hours=lookback_hours)
    candidates = []
    for run in runs:
        if run.get("conclusion") not in RERUNNABLE_CONCLUSIONS:
            continue
        if not is_bot(run_actor_login(run), bot_logins):
            continue
        if int(run.get("run_attempt") or 1) >= max_attempts:
            continue
        created_at = parse_github_time(run.get("created_at"))
        if created_at and created_at < cutoff:
            continue
        if has_newer_run_for_branch(run, runs):
            continue
        jobs = fetch_jobs(repo, run["id"])
        if failed_only_because_priority_gate(jobs):
            candidates.append(run)
    return sorted(candidates, key=lambda run: run.get("run_number", 0))


def rerun_run(repo, run_id):
    result = subprocess.run(
        ["gh", "api", "-X", "POST", f"/repos/{repo}/actions/runs/{run_id}/rerun"],
        capture_output=True,
        text=True,
        timeout=120,
    )
    if result.returncode != 0:
        raise RuntimeError(f"Failed to rerun {run_id}: {result.stderr.strip()}")


def main():
    parser = argparse.ArgumentParser(
        description="Rerun bot CI runs that intentionally yielded to human CI."
    )
    parser.add_argument(
        "--repo", default=os.environ.get("GITHUB_REPOSITORY", DEFAULT_REPO)
    )
    parser.add_argument("--workflow", default=DEFAULT_WORKFLOW)
    parser.add_argument("--lookback-hours", type=int, default=12)
    parser.add_argument("--max-attempts", type=int, default=3)
    parser.add_argument("--max-reruns", type=int, default=1)
    parser.add_argument(
        "--bot-login",
        action="append",
        default=[],
        help="Additional exact bot login to treat as low priority. May be repeated.",
    )
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    active_runs = any_active_ci(fetch_active_runs(args.repo, args.workflow))
    if active_runs:
        print(f"CI is still active ({len(active_runs)} run(s)); not rerunning bot CI.")
        for run in active_runs[:10]:
            print(
                f"  active #{run.get('run_number')} "
                f"({run.get('event')}, {run.get('status')}, by {run_actor_login(run)})"
            )
        return 0

    bot_logins = normalize_bot_logins(args.bot_login)
    completed_runs = fetch_recent_completed_runs(args.repo, args.workflow)
    candidates = yielded_bot_candidates(
        completed_runs,
        args.repo,
        bot_logins,
        args.lookback_hours,
        args.max_attempts,
    )
    if not candidates:
        print("No yielded bot CI runs are eligible for rerun.")
        return 0

    rerun_count = 0
    for run in candidates[: args.max_reruns]:
        print(
            f"Rerunning yielded bot CI run #{run.get('run_number')} "
            f"(id={run.get('id')}, branch={run.get('head_branch')}, "
            f"attempt={run.get('run_attempt')})."
        )
        if not args.dry_run:
            rerun_run(args.repo, run["id"])
        rerun_count += 1

    if args.dry_run:
        print(f"Dry run: would rerun {rerun_count} run(s).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
