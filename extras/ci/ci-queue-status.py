#!/usr/bin/env python3
"""
Check current CI queue status for a GitHub Actions repository.

Shows live queue depth by runner group, longest-waiting jobs, active runs,
and self-hosted runner status. Uses the same GitHub API endpoints that
CREMA/KEDA's GitHub runner scaler uses for auto-scaling decisions.

Usage:
    python3 ci-queue-status.py
    python3 ci-queue-status.py --repo OWNER/REPO
    python3 ci-queue-status.py --top-waiting 20

Requires: gh CLI (authenticated)
"""

import argparse
import json
import os
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime, timezone
from collections import defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gh_api import gh_api, gh_api_list

DEFAULT_REPO = "shader-slang/slang"
DEFAULT_TOP_WAITING = 15

# Runner label groups derived from CI workflows.
# Each entry: (required_labels_set, group_name, is_self_hosted)
# Checked top-to-bottom; first match where required_labels is a subset of
# the job's labels wins.
DEFAULT_LABEL_GROUPS = [
    ({"Linux", "self-hosted", "GPU"}, "Linux GPU (GCP)", True),
    ({"Windows", "self-hosted", "GCP-T4"}, "Windows GPU (GCP)", True),
    # These Windows runners share the same physical machines (SLANGWIN*)
    ({"Windows", "self-hosted", "regression-test"}, "Windows", True),
    ({"Windows", "self-hosted", "benchmark"}, "Windows", True),
    ({"Windows", "self-hosted", "falcor"}, "Windows", True),
    ({"Windows", "self-hosted", "perf"}, "Windows", True),
    ({"ubuntu-22.04"}, "Linux (GH)", False),
    ({"ubuntu-latest"}, "Linux (GH)", False),
    ({"ubuntu-24.04-arm"}, "Linux ARM64 (GH)", False),
    ({"macos-latest"}, "macOS (GH)", False),
    ({"windows-latest"}, "Windows (GH)", False),
]


def load_runner_config():
    """Load label groups and name prefixes from analytics/runner_config.json."""
    config_path = os.path.join(
        os.path.dirname(os.path.abspath(__file__)),
        "analytics",
        "runner_config.json",
    )
    if not os.path.exists(config_path):
        return DEFAULT_LABEL_GROUPS, []
    try:
        with open(config_path, "r", encoding="utf-8") as f:
            config = json.load(f)
        groups = []
        for group in config.get("label_groups", []):
            labels = set(group.get("labels", []))
            name = group.get("name", "Other")
            self_hosted = bool(group.get("self_hosted", False))
            groups.append((labels, name, self_hosted))
        prefixes = []
        for p in config.get("runner_name_prefixes", []):
            prefix = p.get("prefix", "")
            if not prefix:
                continue
            prefixes.append((prefix, p.get("name", "Other"), bool(p.get("self_hosted", False))))
        return groups or DEFAULT_LABEL_GROUPS, prefixes
    except (OSError, json.JSONDecodeError):
        return DEFAULT_LABEL_GROUPS, []


LABEL_GROUPS, NAME_PREFIXES = load_runner_config()


def fetch_runs(repo, status):
    """Fetch workflow runs with given status."""
    data, err = gh_api_list(
        f"repos/{repo}/actions/runs?status={status}&per_page=100",
        "workflow_runs",
    )
    if err:
        print(f"Warning: Failed to fetch {status} runs: {err}", file=sys.stderr)
        return []
    return data if isinstance(data, list) else []


def fetch_jobs_for_run(repo, run_id):
    """Fetch all jobs for a specific run."""
    data, err = gh_api_list(
        f"repos/{repo}/actions/runs/{run_id}/jobs?per_page=100",
        "jobs",
    )
    if err:
        return []
    return data if isinstance(data, list) else []


def fetch_runners(repo):
    """Fetch runners available to this repo. Returns (runners, success).

    Combines repo-level runners with org-level runners from runner groups
    that are shared with this repo (visibility "all" or explicitly shared).
    Falls back to repo-level only if org API is not accessible.
    """
    runners = []
    seen_ids = set()
    success = False

    # Repo-level runners (always the ground truth)
    data, err = gh_api_list(f"repos/{repo}/actions/runners?per_page=100", "runners")
    if not err and isinstance(data, list):
        for r in data:
            rid = r.get("id")
            if rid is None:
                continue
            runners.append(r)
            seen_ids.add(rid)
        success = True

    # Try org-level: find runner groups shared with this repo
    org, repo_name = repo.split("/", 1) if "/" in repo else (None, repo)
    if org:
        groups, err = gh_api_list(
            f"orgs/{org}/actions/runner-groups?per_page=100", "runner_groups"
        )
        if not err and isinstance(groups, list):

            def _check_group_shared(group):
                """Check if a runner group is shared with repo_name."""
                if group.get("visibility") == "all":
                    return True
                if group.get("selected_repositories_url"):
                    gid = group["id"]
                    data, rerr = gh_api_list(
                        f"orgs/{org}/actions/runner-groups/{gid}/repositories?per_page=100",
                        "repositories",
                    )
                    if not rerr and isinstance(data, list):
                        return any(
                            r.get("name") == repo_name
                            for r in data
                        )
                return False

            def _fetch_group_runners(group):
                """Fetch runners for a group if shared with this repo."""
                if not _check_group_shared(group):
                    return []
                gid = group["id"]
                data, rerr = gh_api_list(
                    f"orgs/{org}/actions/runner-groups/{gid}/runners?per_page=100",
                    "runners",
                )
                if not rerr and isinstance(data, list):
                    return data
                return []

            with ThreadPoolExecutor(max_workers=6) as executor:
                for group_runners in executor.map(_fetch_group_runners, groups):
                    for r in group_runners:
                        rid = r.get("id")
                        if rid is not None and rid not in seen_ids:
                            runners.append(r)
                            seen_ids.add(rid)
            success = True

    return runners, success


# --- Data processing ---


def classify_group(labels, runner_name=""):
    """Map runner name or job labels to a (group_name, is_self_hosted) tuple.

    Checks runner name prefixes first (scale set runners have empty labels),
    then falls back to label matching for legacy/static runners.
    """
    if runner_name:
        for prefix, name, self_hosted in NAME_PREFIXES:
            if runner_name.startswith(prefix):
                return name, self_hosted
    label_set = set(labels) if labels else set()
    for required, name, self_hosted in LABEL_GROUPS:
        if required <= label_set:
            return name, self_hosted
    return "Other", False


def parse_dt(s):
    """Parse ISO 8601 datetime string to timezone-aware datetime."""
    if not s:
        return None
    return datetime.fromisoformat(s.replace("Z", "+00:00"))


def format_duration(seconds):
    """Format seconds as human-readable duration."""
    if seconds < 0:
        return "0s"
    seconds = int(seconds)
    if seconds < 60:
        return f"{seconds}s"
    minutes = seconds // 60
    secs = seconds % 60
    if minutes < 60:
        return f"{minutes}m {secs:02d}s"
    hours = minutes // 60
    mins = minutes % 60
    return f"{hours}h {mins:02d}m"


def format_ago(dt, now):
    """Format a past datetime as 'Xm ago' or 'Xh Ym ago'."""
    if not dt:
        return "?"
    seconds = (now - dt).total_seconds()
    if seconds < 60:
        return f"{int(seconds)}s ago"
    minutes = int(seconds) // 60
    if minutes < 60:
        return f"{minutes}m ago"
    hours = minutes // 60
    mins = minutes % 60
    return f"{hours}h {mins:02d}m ago"


# --- Output ---


def print_section(title):
    """Print a section header."""
    print("\n" + "=" * 100)
    print(title)
    print("=" * 100)


def print_summary(queued_runs, inprogress_runs, all_jobs, now, repo):
    """Print snapshot summary."""
    print("=" * 100)
    print("CI QUEUE STATUS SNAPSHOT")
    print("=" * 100)
    print(f"\nFetched at: {now.strftime('%Y-%m-%d %H:%M:%S UTC')}  |  Repo: {repo}")

    queued_jobs = [j for j in all_jobs if j["status"] in ("queued", "waiting")]
    running_jobs = [j for j in all_jobs if j["status"] == "in_progress"]

    print(f"\nRuns queued:       {len(queued_runs):3d}")
    print(f"Runs in progress:  {len(inprogress_runs):3d}")
    print(
        f"\nJobs queued (need runner): {len(queued_jobs):3d}"
    )
    print(f"Jobs running:             {len(running_jobs):3d}")


def print_queue_by_group(all_jobs, runners, runners_available):
    """Print queue depth broken down by runner label group."""
    print_section("QUEUE DEPTH BY RUNNER GROUP")

    # Count jobs per group
    group_queued = defaultdict(int)
    group_running = defaultdict(int)
    group_self_hosted = {}
    group_names_seen = []

    for job in all_jobs:
        group_name, is_self_hosted = classify_group(job.get("labels", []), job.get("runner_name", ""))
        if group_name not in group_queued and group_name not in group_running:
            group_names_seen.append(group_name)
        group_self_hosted[group_name] = is_self_hosted
        if job["status"] == "queued":
            group_queued[group_name] += 1
        elif job["status"] == "in_progress":
            group_running[group_name] += 1

    # Count runners per group
    group_runners_total = defaultdict(int)
    group_runners_idle = defaultdict(int)
    for runner in runners:
        labels = [l["name"] for l in runner.get("labels", [])]
        group_name, is_self_hosted = classify_group(labels, runner.get("name", ""))
        if is_self_hosted and runner.get("status") == "online":
            group_runners_total[group_name] += 1
            if not runner.get("busy", False):
                group_runners_idle[group_name] += 1

    # Stable ordering: groups in the order they appear in LABEL_GROUPS, then "Other"
    ordered_groups = []
    seen = set()
    for _, name, _ in LABEL_GROUPS:
        if name not in seen and (name in group_queued or name in group_running):
            ordered_groups.append(name)
            seen.add(name)
    for name in group_names_seen:
        if name not in seen:
            ordered_groups.append(name)
            seen.add(name)

    header = f"  {'Runner Group':<35s} {'Queued':>8s} {'Running':>9s}"
    if runners_available:
        header += f"  {'Runners':>18s}"
    print(f"\n{header}")
    print(f"  {'-' * 35} {'-' * 8} {'-' * 9}", end="")
    if runners_available:
        print(f"  {'-' * 18}", end="")
    print()

    for group_name in ordered_groups:
        queued = group_queued.get(group_name, 0)
        running = group_running.get(group_name, 0)
        line = f"  {group_name:<35s} {queued:>8d} {running:>9d}"

        if runners_available:
            total = group_runners_total.get(group_name, 0)
            idle = group_runners_idle.get(group_name, 0)
            if total > 0:
                line += f"  {idle:>2d} idle / {total:>2d} total"
            elif group_self_hosted.get(group_name, False):
                line += f"  {'(org-level)':>18s}"
            else:
                line += f"  {'(cloud)':>18s}"
        print(line)


def print_longest_waiting(all_jobs, top_n, now):
    """Print the longest-waiting queued jobs."""
    print_section(f"LONGEST-WAITING QUEUED JOBS (top {top_n})")

    queued = []
    for job in all_jobs:
        if job["status"] != "queued":
            continue
        created = parse_dt(job.get("created_at"))
        if not created:
            continue
        wait_secs = (now - created).total_seconds()
        queued.append((wait_secs, job))

    queued.sort(key=lambda x: x[0], reverse=True)

    if not queued:
        print("\n  No jobs currently queued.")
        return

    print(
        f"\n  {'Wait':>9s}   {'Job':50s}   {'Run / Branch'}"
    )
    print(
        f"  {'-' * 9}   {'-' * 50}   {'-' * 40}"
    )

    for wait_secs, job in queued[:top_n]:
        name = job["name"][:50]
        run_branch = job.get("_branch", "")
        run_workflow = job.get("_workflow", "")
        context = f"{run_workflow} / {run_branch}" if run_workflow else run_branch
        print(
            f"  {format_duration(wait_secs):>9s}   {name:50s}   {context}"
        )


def print_inprogress_runs(inprogress_runs, now):
    """Print currently active runs."""
    print_section("IN-PROGRESS RUNS")

    if not inprogress_runs:
        print("\n  No runs currently in progress.")
        return

    # Sort by start time (oldest first)
    runs_sorted = sorted(
        inprogress_runs,
        key=lambda r: r.get("run_started_at") or r.get("created_at", ""),
    )

    print(
        f"\n  {'Started':>12s}   {'Branch':30s}   {'Workflow':20s}   {'Event':15s}   {'Actor'}"
    )
    print(
        f"  {'-' * 12}   {'-' * 30}   {'-' * 20}   {'-' * 15}   {'-' * 20}"
    )

    for run in runs_sorted:
        started = parse_dt(run.get("run_started_at") or run.get("created_at"))
        branch = (run.get("head_branch") or "")[:30]
        workflow = (run.get("name") or "")[:20]
        event = (run.get("event") or "")[:15]
        actor = (run.get("actor", {}) or {}).get("login", "")
        print(
            f"  {format_ago(started, now):>12s}   {branch:30s}   {workflow:20s}   {event:15s}   {actor}"
        )


def print_runner_status(runners, all_jobs, runners_available):
    """Print self-hosted runner status with current job info."""
    print_section("SELF-HOSTED RUNNER STATUS")

    if not runners_available:
        print(
            "\n  Runners endpoint not accessible (requires admin access). Skipping."
        )
        return

    # Build runner_name -> job mapping for in-progress jobs
    runner_to_job = {}
    for job in all_jobs:
        if job["status"] == "in_progress" and job.get("runner_name"):
            runner_to_job[job["runner_name"]] = job

    # Group self-hosted runners by group
    groups = defaultdict(list)
    for runner in runners:
        labels = [l["name"] for l in runner.get("labels", [])]
        group_name, is_self_hosted = classify_group(labels, runner.get("name", ""))
        if is_self_hosted:
            groups[group_name].append(runner)

    if not groups:
        print("\n  No self-hosted runners found.")
        return

    for group_name, group_runners in sorted(groups.items()):
        online = sum(1 for r in group_runners if r.get("status") == "online")
        print(f"\n  {group_name} ({online} online / {len(group_runners)} total):")

        for runner in sorted(group_runners, key=lambda r: r["name"]):
            name = runner["name"]
            status = "ONLINE" if runner.get("status") == "online" else "OFFLINE"
            busy = runner.get("busy", False)

            if status == "OFFLINE":
                print(f"    {name:40s} {status}")
                continue

            state = "BUSY" if busy else "IDLE"
            line = f"    {name:40s} {status:7s} {state:5s}"

            if busy and name in runner_to_job:
                job = runner_to_job[name]
                job_name = job["name"]
                branch = job.get("_branch", "")
                if branch:
                    line += f"  -> {job_name} ({branch})"
                else:
                    line += f"  -> {job_name}"

            print(line)


def build_json_output(queued_runs, inprogress_runs, all_jobs, runners, runners_available, now, repo):
    """Build JSON output structure for programmatic consumption."""
    queued_jobs = [j for j in all_jobs if j["status"] in ("queued", "waiting")]
    running_jobs = [j for j in all_jobs if j["status"] == "in_progress"]

    # Queue depth by group
    group_queued = defaultdict(int)
    group_running = defaultdict(int)
    group_self_hosted = {}
    group_names_seen = []

    for job in all_jobs:
        group_name, is_self_hosted = classify_group(job.get("labels", []), job.get("runner_name", ""))
        if group_name not in group_queued and group_name not in group_running:
            group_names_seen.append(group_name)
        group_self_hosted[group_name] = is_self_hosted
        if job["status"] == "queued":
            group_queued[group_name] += 1
        elif job["status"] == "in_progress":
            group_running[group_name] += 1

    group_runners_total = defaultdict(int)
    group_runners_idle = defaultdict(int)
    if runners_available:
        for runner in runners:
            labels = [l["name"] for l in runner.get("labels", [])]
            group_name, is_self_hosted = classify_group(labels, runner.get("name", ""))
            if is_self_hosted and runner.get("status") == "online":
                group_runners_total[group_name] += 1
                if not runner.get("busy", False):
                    group_runners_idle[group_name] += 1

    ordered_groups = []
    seen = set()
    for _, name, _ in LABEL_GROUPS:
        if name not in seen and (name in group_queued or name in group_running):
            ordered_groups.append(name)
            seen.add(name)
    for name in group_names_seen:
        if name not in seen:
            ordered_groups.append(name)
            seen.add(name)

    groups = []
    for group_name in ordered_groups:
        entry = {
            "name": group_name,
            "queued": group_queued.get(group_name, 0),
            "running": group_running.get(group_name, 0),
            "self_hosted": bool(group_self_hosted.get(group_name, False)),
        }
        if runners_available:
            entry["runners"] = {
                "idle": group_runners_idle.get(group_name, 0),
                "total": group_runners_total.get(group_name, 0),
            }
        groups.append(entry)

    # Longest waiting jobs (queued)
    queued = []
    for job in queued_jobs:
        created = parse_dt(job.get("created_at"))
        if not created:
            continue
        wait_secs = (now - created).total_seconds()
        queued.append((wait_secs, job))
    queued.sort(key=lambda x: x[0], reverse=True)

    longest_waiting = []
    for wait_secs, job in queued:
        longest_waiting.append(
            {
                "wait_seconds": int(wait_secs),
                "name": job.get("name", ""),
                "labels": job.get("labels", []),
                "branch": job.get("_branch", ""),
                "workflow": job.get("_workflow", ""),
                "created_at": job.get("created_at"),
                "runner_name": job.get("runner_name"),
                "html_url": job.get("html_url", ""),
            }
        )

    inprogress = []
    for run in sorted(
        inprogress_runs,
        key=lambda r: r.get("run_started_at") or r.get("created_at", ""),
    ):
        inprogress.append(
            {
                "id": run.get("id"),
                "branch": run.get("head_branch"),
                "workflow": run.get("name"),
                "event": run.get("event"),
                "actor": (run.get("actor", {}) or {}).get("login"),
                "run_started_at": run.get("run_started_at"),
                "created_at": run.get("created_at"),
            }
        )

    runner_status = []
    if runners_available:
        runner_to_job = {}
        for job in all_jobs:
            if job["status"] == "in_progress" and job.get("runner_name"):
                runner_to_job[job["runner_name"]] = job

        for runner in runners:
            labels = [l["name"] for l in runner.get("labels", [])]
            group_name, is_self_hosted = classify_group(labels, runner.get("name", ""))
            if not is_self_hosted:
                continue
            entry = {
                "name": runner.get("name"),
                "status": runner.get("status"),
                "busy": runner.get("busy", False),
                "group": group_name,
                "labels": labels,
            }
            job = runner_to_job.get(runner.get("name"))
            if job:
                entry["job"] = {
                    "name": job.get("name"),
                    "branch": job.get("_branch", ""),
                    "workflow": job.get("_workflow", ""),
                    "html_url": job.get("html_url", ""),
                }
            runner_status.append(entry)

    return {
        "fetched_at": now.strftime("%Y-%m-%d %H:%M:%S UTC"),
        "repo": repo,
        "summary": {
            "runs_queued": len(queued_runs),
            "runs_in_progress": len(inprogress_runs),
            "jobs_queued": len(queued_jobs),
            "jobs_running": len(running_jobs),
            "jobs_waiting": 0,
        },
        "queue_by_group": groups,
        "longest_waiting_jobs": longest_waiting,
        "in_progress_runs": inprogress,
        "self_hosted_runners": runner_status,
        "runners_available": runners_available,
    }


def main():
    parser = argparse.ArgumentParser(
        description="Check current CI queue status for a GitHub Actions repository."
    )
    parser.add_argument(
        "--repo",
        default=DEFAULT_REPO,
        help=f"Repository in OWNER/REPO format (default: {DEFAULT_REPO})",
    )
    parser.add_argument(
        "--top-waiting",
        type=int,
        default=DEFAULT_TOP_WAITING,
        metavar="N",
        help=f"Number of longest-waiting jobs to show (default: {DEFAULT_TOP_WAITING})",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Output JSON instead of human-readable text",
    )
    args = parser.parse_args()

    repo = args.repo
    now = datetime.now(timezone.utc)

    # Fetch runs and runners in parallel
    with ThreadPoolExecutor(max_workers=3) as executor:
        queued_future = executor.submit(fetch_runs, repo, "queued")
        inprogress_future = executor.submit(fetch_runs, repo, "in_progress")
        runners_future = executor.submit(fetch_runners, repo)

        queued_runs = queued_future.result()
        inprogress_runs = inprogress_future.result()
        runners, runners_available = runners_future.result()

    all_runs = queued_runs + inprogress_runs

    if not all_runs:
        print("No queued or in-progress runs found.")
        return

    # Fetch jobs for all runs in parallel
    print(
        f"Fetching jobs for {len(all_runs)} active runs...",
        file=sys.stderr,
        flush=True,
    )

    with ThreadPoolExecutor(max_workers=8) as executor:
        futures = {
            executor.submit(fetch_jobs_for_run, repo, r["id"]): r
            for r in all_runs
            if r.get("id") is not None
        }
        for future in as_completed(futures):
            run = futures[future]
            jobs = future.result()
            # Annotate jobs with parent run info for display
            for job in jobs:
                job["_branch"] = run.get("head_branch", "")
                job["_workflow"] = run.get("name", "")
            run["jobs"] = jobs

    # Flatten all jobs (exclude completed to keep things relevant)
    all_jobs = []
    for run in all_runs:
        for job in run.get("jobs", []):
            if job.get("status") != "completed":
                all_jobs.append(job)

    # Output
    if args.json:
        payload = build_json_output(
            queued_runs,
            inprogress_runs,
            all_jobs,
            runners,
            runners_available,
            now,
            repo,
        )
        payload["longest_waiting_jobs"] = payload["longest_waiting_jobs"][
            : args.top_waiting
        ]
        print(json.dumps(payload, indent=2))
        return

    print_summary(queued_runs, inprogress_runs, all_jobs, now, repo)
    print_queue_by_group(all_jobs, runners, runners_available)
    print_longest_waiting(all_jobs, args.top_waiting, now)
    print_inprogress_runs(inprogress_runs, now)
    print_runner_status(runners, all_jobs, runners_available)
    print("\n" + "=" * 100)


if __name__ == "__main__":
    main()
