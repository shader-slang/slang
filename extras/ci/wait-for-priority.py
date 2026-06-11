#!/usr/bin/env python3
"""
Hold a low-priority (bot-authored) CI run until human-authored CI has cleared.

GitHub Actions assigns queued jobs to self-hosted runners roughly FIFO, with no
native notion of priority. This script is meant to run as the very first job of
a bot-authored CI run. It blocks (polling the Actions API) until:

  * no human-authored CI run for the same workflow is queued or in progress, and
  * no *older* bot-authored CI run is still queued or in progress.

Once both hold, it exits 0 and the rest of the pipeline proceeds. The effect is:

  * human PRs never wait behind a bot PR's CI (the bot yields the runners), and
  * at most one bot CI consumes the contended runners at a time (older first).

A running bot job is never preempted: this gate only delays bot runs that have
not yet started consuming runners. If a human PR appears while a bot run is
already executing, the human simply queues behind it (one job's wait).

To avoid starving the bot forever when humans submit continuously, the gate
gives up after --max-wait-minutes and proceeds anyway.

Usage (in a workflow step):
    python3 extras/ci/wait-for-priority.py --workflow ci.yml

Requires: gh CLI (authenticated via the workflow token).
"""

import argparse
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gh_api import gh_api

DEFAULT_REPO = "shader-slang/slang"
DEFAULT_WORKFLOW = "ci.yml"
DEFAULT_POLL_INITIAL_SECONDS = 15
DEFAULT_POLL_MAX_SECONDS = 300
DEFAULT_MAX_WAIT_MINUTES = 300

DEFAULT_BOT_LOGINS = {
    "nv-slang-bot",
    "nv-slang-bot[bot]",
}

# Run statuses that mean a run still holds (or is waiting for) runners.
ACTIVE_STATUSES = {"queued", "in_progress", "waiting", "requested", "pending"}


def is_bot(login, bot_logins):
    if not login:
        return False
    login = login.lower()
    return login.endswith("[bot]") or login in bot_logins


def run_actor_login(run):
    """Best-effort login of whoever caused the run."""
    for key in ("triggering_actor", "actor"):
        actor = run.get(key) or {}
        login = actor.get("login")
        if login:
            return login
    return ""


def fetch_active_runs(repo, workflow):
    """Return active CI runs for the workflow across the relevant statuses."""
    runs = {}
    for status in ("queued", "in_progress"):
        endpoint = (
            f"/repos/{repo}/actions/workflows/{workflow}/runs"
            f"?status={status}&per_page=100"
        )
        data, err = gh_api(endpoint)
        if err:
            raise RuntimeError(f"Failed to list {status} runs: {err}")
        for run in (data or {}).get("workflow_runs", []):
            runs[run["id"]] = run
    return list(runs.values())


def fetch_self_run(repo, run_id):
    data, err = gh_api(f"/repos/{repo}/actions/runs/{run_id}")
    if err:
        raise RuntimeError(f"Failed to fetch self run {run_id}: {err}")
    return data


def classify_blockers(runs, self_run_id, self_run_number, bot_logins):
    """Return (human_blockers, older_bot_blockers) among active runs."""
    human = []
    older_bot = []
    for run in runs:
        if run.get("id") == self_run_id:
            continue
        if run.get("status") not in ACTIVE_STATUSES:
            continue
        # merge_group runs are post-approval merges; always yield to them.
        if run.get("event") == "merge_group" or not is_bot(
            run_actor_login(run), bot_logins
        ):
            human.append(run)
            continue
        # Bot run: only older ones (lower run_number) take precedence over us.
        if run.get("run_number", 0) < self_run_number:
            older_bot.append(run)
    return human, older_bot


def describe(run):
    return (
        f"#{run.get('run_number')} "
        f"({run.get('event')}, {run.get('status')}, "
        f"by {run_actor_login(run) or 'unknown'})"
    )


def main():
    parser = argparse.ArgumentParser(
        description="Delay a bot-authored CI run until human CI has cleared."
    )
    parser.add_argument(
        "--repo", default=os.environ.get("GITHUB_REPOSITORY", DEFAULT_REPO)
    )
    parser.add_argument("--workflow", default=DEFAULT_WORKFLOW)
    parser.add_argument(
        "--poll-initial", type=int, default=DEFAULT_POLL_INITIAL_SECONDS,
        help="Starting poll interval in seconds (doubles each round, capped at --poll-max).",
    )
    parser.add_argument(
        "--poll-max", type=int, default=DEFAULT_POLL_MAX_SECONDS,
        help="Maximum poll interval in seconds.",
    )
    parser.add_argument(
        "--max-wait-minutes", type=int, default=DEFAULT_MAX_WAIT_MINUTES
    )
    parser.add_argument(
        "--bot-login",
        action="append",
        default=[],
        help="Additional exact bot login to treat as low priority. May be repeated.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Report the current decision once and exit without waiting.",
    )
    args = parser.parse_args()

    bot_logins = {login.lower() for login in DEFAULT_BOT_LOGINS}
    bot_logins.update(login.lower() for login in args.bot_login)

    self_run_id = int(os.environ.get("GITHUB_RUN_ID", "0"))
    self_run_number = None
    if self_run_id:
        try:
            self_run = fetch_self_run(args.repo, self_run_id)
            self_run_number = self_run.get("run_number")
        except RuntimeError as exc:
            print(f"::warning::{exc}")
    if self_run_number is None:
        self_run_number = int(os.environ.get("GITHUB_RUN_NUMBER", "0"))

    print(
        f"Priority gate for run #{self_run_number} (id={self_run_id}) "
        f"on {args.repo} workflow {args.workflow}."
    )

    deadline = time.monotonic() + args.max_wait_minutes * 60
    interval = args.poll_initial
    while True:
        try:
            runs = fetch_active_runs(args.repo, args.workflow)
        except RuntimeError as exc:
            # Be conservative: a transient API failure should not let the bot
            # jump the queue, but it also should not hang forever.
            print(f"::warning::{exc}; retrying after {interval}s")
            if args.dry_run:
                return 0
            time.sleep(interval)
            interval = min(interval * 2, args.poll_max)
            continue

        human, older_bot = classify_blockers(
            runs, self_run_id, self_run_number, bot_logins
        )

        if not human and not older_bot:
            print("No higher-priority CI is active. Proceeding.")
            return 0

        for run in human:
            print(f"Yielding to human/merge CI {describe(run)}")
        for run in older_bot:
            print(f"Waiting behind earlier bot CI {describe(run)}")

        if args.dry_run:
            print("Dry run: would wait.")
            return 0

        if time.monotonic() >= deadline:
            print(
                f"::warning::Waited {args.max_wait_minutes} min; "
                "proceeding anyway to avoid starvation."
            )
            return 0

        print(f"Next check in {interval}s.")
        time.sleep(interval)
        interval = min(interval * 2, args.poll_max)


if __name__ == "__main__":
    sys.exit(main())
