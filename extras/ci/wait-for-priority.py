#!/usr/bin/env python3
"""
Yield a low-priority (bot-authored) CI run when human-authored CI is active.

GitHub Actions assigns queued jobs to self-hosted runners roughly FIFO, with no
native notion of priority. This script is meant to run as the very first job of
a bot-authored CI run. It checks once whether:

  * no human-authored CI run for the same workflow is queued or in progress, and
  * no *older* bot-authored CI run is still queued or in progress.

If either check fails, it writes `yielded=true` to `$GITHUB_OUTPUT` and exits 0.
The workflow has a separate marker step that fails only for this intentional
yield, so a scheduled retry workflow can safely distinguish priority yields from
script or API failures. The effect is:

  * human PRs never wait behind a bot PR's CI (the bot yields the runners), and
  * at most one bot CI consumes the contended runners at a time (older first).

A running bot job is never preempted: this gate only stops bot runs that have
not yet started consuming the expensive build/test runners.

Usage (in a workflow step):
    python3 extras/ci/wait-for-priority.py --workflow ci.yml

Requires: gh CLI (authenticated via the workflow token).
"""

import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gh_api import gh_api, gh_api_list

DEFAULT_REPO = "shader-slang/slang"
DEFAULT_WORKFLOW = "ci.yml"

DEFAULT_BOT_LOGINS = {
    "nv-slang-bot",
    "nv-slang-bot[bot]",
}

# Run statuses that mean a run still holds, or is waiting for, runner capacity.
ACTIVE_STATUSES = {"queued", "in_progress", "waiting", "requested", "pending"}


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
    """Best-effort login of whoever caused the run."""
    for key in ("triggering_actor", "actor"):
        actor = run.get(key) or {}
        login = actor.get("login")
        if login:
            return login
    return ""


def fetch_active_runs(repo, workflow):
    """Return active CI runs for the workflow across all active statuses."""
    runs = {}
    for status in sorted(ACTIVE_STATUSES):
        endpoint = (
            f"/repos/{repo}/actions/workflows/{workflow}/runs"
            f"?status={status}&per_page=100"
        )
        items, err = gh_api_list(endpoint, "workflow_runs")
        if err:
            raise RuntimeError(f"Failed to list {status} runs: {err}")
        for run in items or []:
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


def write_output(name, value):
    output_path = os.environ.get("GITHUB_OUTPUT")
    if not output_path:
        return
    with open(output_path, "a", encoding="utf-8") as output:
        output.write(f"{name}={value}\n")


def main():
    parser = argparse.ArgumentParser(
        description="Stop a bot-authored CI run when higher-priority CI is active."
    )
    parser.add_argument(
        "--repo", default=os.environ.get("GITHUB_REPOSITORY", DEFAULT_REPO)
    )
    parser.add_argument("--workflow", default=DEFAULT_WORKFLOW)
    parser.add_argument(
        "--bot-login",
        action="append",
        default=[],
        help="Additional exact bot login to treat as low priority. May be repeated.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Report the current decision without writing GITHUB_OUTPUT.",
    )
    args = parser.parse_args()

    bot_logins = normalize_bot_logins(args.bot_login)

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

    runs = fetch_active_runs(args.repo, args.workflow)
    human, older_bot = classify_blockers(
        runs, self_run_id, self_run_number, bot_logins
    )

    yielded = bool(human or older_bot)
    if not args.dry_run:
        write_output("yielded", "true" if yielded else "false")

    if not yielded:
        print("No higher-priority CI is active. Proceeding.")
        return 0

    for run in human:
        print(f"Yielding to human/merge CI {describe(run)}")
    for run in older_bot:
        print(f"Yielding behind earlier bot CI {describe(run)}")
    print("Higher-priority CI is active. Marking this bot run for retry.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
