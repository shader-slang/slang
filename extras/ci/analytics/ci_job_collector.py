#!/usr/bin/env python3
"""
GitHub Actions CI Job Collector

Collects completed workflow run and job data from GitHub Actions API
and saves it as JSON. Supports incremental updates — if the output file
exists, only newer jobs are fetched and merged.

Usage:
    python3 ci_job_collector.py                          # last 7 days, shader-slang/slang
    python3 ci_job_collector.py --repo OWNER/REPO
    python3 ci_job_collector.py --days 30
    python3 ci_job_collector.py --days all
    python3 ci_job_collector.py --output ci_data.json

Requires: gh CLI (authenticated)
"""

import argparse
import datetime
import json
import os
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import timezone

# Import shared helpers from parent ci/ directory
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from gh_api import gh_api_list

DEFAULT_REPO = "shader-slang/slang"
DEFAULT_DAYS = 7
DEFAULT_OUTPUT = "ci_jobs.json"


def parse_args():
    parser = argparse.ArgumentParser(
        description="Collect GitHub Actions CI job data and save as JSON."
    )
    parser.add_argument(
        "--repo",
        default=DEFAULT_REPO,
        help=f"Repository in OWNER/REPO format (default: {DEFAULT_REPO})",
    )
    parser.add_argument(
        "--days",
        default=str(DEFAULT_DAYS),
        help=f"Days to look back if no existing data (default: {DEFAULT_DAYS}). Use 'all' for everything.",
    )
    parser.add_argument(
        "--output",
        default=DEFAULT_OUTPUT,
        help=f"Output JSON file (default: {DEFAULT_OUTPUT})",
    )
    parser.add_argument(
        "--workflow",
        help="Only collect runs for this workflow name (e.g. 'CI'). Reduces API calls significantly.",
    )
    parser.add_argument(
        "--verbose", action="store_true", help="Enable verbose output"
    )
    return parser.parse_args()


def load_existing_data(file_path, verbose=False):
    """Load existing job data from file if it exists."""
    if not os.path.exists(file_path):
        if verbose:
            print(f"No existing data at {file_path}")
        return []
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            data = json.load(f)
            if verbose:
                print(f"Loaded {len(data)} existing jobs from {file_path}")
            return data
    except (json.JSONDecodeError, OSError) as e:
        print(f"Warning: Could not load existing data: {e}", file=sys.stderr)
        return []


def get_start_date(days_str, existing_data, verbose=False):
    """Determine the start date for fetching.

    If existing data is available, use the most recent job's created_at.
    Otherwise, use the --days parameter.
    """
    if existing_data:
        latest = None
        for job in existing_data:
            created = job.get("created_at")
            if not created:
                continue
            try:
                dt = datetime.datetime.fromisoformat(
                    created.replace("Z", "+00:00")
                )
                if latest is None or dt > latest:
                    latest = dt
            except (ValueError, TypeError):
                continue

        if latest:
            # Start from the beginning of the day of the latest job
            start = latest.replace(hour=0, minute=0, second=0, microsecond=0)
            if verbose:
                print(f"Incremental update from {start.isoformat()}")
            return start

    # No existing data or no valid dates — use days parameter
    if days_str == "all":
        if verbose:
            print("Collecting all available jobs")
        # GitHub Actions launched Nov 2019; no data before that
        return datetime.datetime(2019, 11, 1, tzinfo=timezone.utc)

    try:
        days = int(days_str)
    except ValueError:
        print(f"Error: --days must be a number or 'all', got '{days_str}'", file=sys.stderr)
        sys.exit(1)
    start = datetime.datetime.now(timezone.utc) - datetime.timedelta(days=days)
    start = start.replace(hour=0, minute=0, second=0, microsecond=0)
    if verbose:
        print(f"Collecting jobs from past {days} days (since {start.isoformat()})")
    return start


API_PAGINATION_CAP = 1000


def _fetch_runs_window(repo, date_from, date_to, seen_ids, depth=0):
    """Fetch runs for a time window, subdividing if the API cap is hit.

    The GitHub API returns at most 1000 results per paginated query.
    When a window hits this cap, we split it in half and fetch both
    halves recursively. Stops subdividing below 1-hour windows.
    """
    from_str = date_from.strftime("%Y-%m-%dT%H:%M:%SZ")
    to_str = date_to.strftime("%Y-%m-%dT%H:%M:%SZ")

    runs, err = gh_api_list(
        f"repos/{repo}/actions/runs?status=completed&per_page=100"
        f"&created={from_str}..{to_str}",
        "workflow_runs",
    )

    if err:
        print(
            f"  Warning: API error for {from_str}..{to_str}: {err}",
            file=sys.stderr,
        )
        return []

    if not runs:
        return []

    # Deduplicate against already-seen IDs
    new_runs = [r for r in runs if r["id"] not in seen_ids]

    if len(runs) >= API_PAGINATION_CAP:
        span = date_to - date_from
        if span > datetime.timedelta(hours=1):
            # Split in half and recurse
            mid = date_from + span / 2
            indent = "  " * depth
            print(
                f"\n{indent}  Window {from_str}..{to_str} hit {len(runs)} cap, splitting...",
                file=sys.stderr,
            )
            for r in new_runs:
                seen_ids.add(r["id"])
            left = _fetch_runs_window(repo, date_from, mid, seen_ids, depth + 1)
            right = _fetch_runs_window(repo, mid, date_to, seen_ids, depth + 1)
            return new_runs + left + right

    for r in new_runs:
        seen_ids.add(r["id"])
    return new_runs


def fetch_completed_runs(repo, since_date, verbose=False):
    """Fetch completed workflow runs since a given date.

    Walks day by day, subdividing any day that hits the GitHub API's
    1000-result pagination cap into smaller windows.
    """
    now = datetime.datetime.now(timezone.utc)
    all_runs = []
    seen_ids = set()

    current = since_date
    while current < now:
        next_day = current + datetime.timedelta(days=1)
        if next_day > now:
            next_day = now

        print(
            f"Fetching runs for {current.strftime('%Y-%m-%d')}...",
            file=sys.stderr,
            end="",
            flush=True,
        )

        day_runs = _fetch_runs_window(repo, current, next_day, seen_ids)
        all_runs.extend(day_runs)

        print(f" {len(day_runs)} runs", file=sys.stderr)
        current = next_day

    if verbose:
        print(f"Total: {len(all_runs)} completed runs")
    return all_runs


def fetch_jobs_for_run(repo, run_id):
    """Fetch all jobs for a specific run."""
    jobs, err = gh_api_list(
        f"repos/{repo}/actions/runs/{run_id}/jobs?per_page=100",
        "jobs",
    )
    if err:
        return []
    return jobs if isinstance(jobs, list) else []


def extract_job_data(job, run):
    """Extract relevant fields from a job + run pair."""
    created = job.get("created_at")
    started = job.get("started_at")
    completed = job.get("completed_at")

    # Compute durations
    duration_seconds = None
    queued_seconds = None
    try:
        if started and completed:
            s = datetime.datetime.fromisoformat(started.replace("Z", "+00:00"))
            c = datetime.datetime.fromisoformat(completed.replace("Z", "+00:00"))
            duration_seconds = (c - s).total_seconds()
        if created and started:
            cr = datetime.datetime.fromisoformat(created.replace("Z", "+00:00"))
            s = datetime.datetime.fromisoformat(started.replace("Z", "+00:00"))
            queued_seconds = (s - cr).total_seconds()
    except (ValueError, TypeError):
        pass

    return {
        "id": job["id"],
        "run_id": run["id"],
        "name": job.get("name", ""),
        "workflow_name": run.get("name", ""),
        "workflow_path": run.get("path", ""),
        "status": job.get("status", ""),
        "conclusion": job.get("conclusion", ""),
        "created_at": created,
        "started_at": started,
        "completed_at": completed,
        "duration_seconds": duration_seconds,
        "queued_seconds": queued_seconds,
        "runner_name": job.get("runner_name", ""),
        "runner_id": job.get("runner_id", 0),
        "runner_group_name": job.get("runner_group_name", ""),
        "labels": job.get("labels", []),
        "head_branch": run.get("head_branch", ""),
        "event": run.get("event", ""),
        "actor": (run.get("actor") or {}).get("login", ""),
        "html_url": job.get("html_url", ""),
        "run_created_at": run.get("created_at", ""),
    }


def collect_jobs(repo, runs, output_path=None, existing=None, verbose=False):
    """Fetch jobs for all runs in parallel and extract data.

    Saves incrementally every 500 runs to avoid losing progress on
    rate limit hits or interruptions.
    """
    all_jobs = list(existing) if existing else []
    existing_ids = {j["id"] for j in all_jobs}
    total = len(runs)
    initial_count = len(all_jobs)

    print(f"Fetching jobs for {total} runs...", file=sys.stderr)

    with ThreadPoolExecutor(max_workers=8) as executor:
        futures = {
            executor.submit(fetch_jobs_for_run, repo, run["id"]): run
            for run in runs
        }

        done = 0
        for future in as_completed(futures):
            run = futures[future]
            jobs = future.result()
            for job in jobs:
                if job.get("status") == "completed" and job["id"] not in existing_ids:
                    all_jobs.append(extract_job_data(job, run))
                    existing_ids.add(job["id"])

            done += 1
            if done % 50 == 0 or done == total:
                print(
                    f"  Progress: {done}/{total} runs ({len(all_jobs) - initial_count} new jobs)",
                    file=sys.stderr,
                )
            # Save incrementally every 500 runs
            if output_path and done % 500 == 0:
                save_data(all_jobs, output_path)
                print(
                    f"  Checkpoint saved: {len(all_jobs)} jobs",
                    file=sys.stderr,
                )

    return all_jobs


def merge_data(existing, new_data, verbose=False):
    """Merge new job data with existing, deduplicating by job ID."""
    if not existing:
        return new_data

    existing_ids = {job["id"] for job in existing}
    added = 0
    for job in new_data:
        if job["id"] not in existing_ids:
            existing.append(job)
            existing_ids.add(job["id"])
            added += 1

    if verbose:
        print(f"Added {added} new jobs (total: {len(existing)})")
    return existing


def save_data(data, file_path):
    """Save job data as JSON."""
    directory = os.path.dirname(file_path)
    if directory:
        os.makedirs(directory, exist_ok=True)
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, default=str)


def main():
    args = parse_args()

    # Load existing data
    existing = load_existing_data(args.output, args.verbose)
    existing_count = len(existing)
    if existing_count > 0:
        print(f"Loaded {existing_count} existing jobs from {args.output}")

    # Determine start date
    start_date = get_start_date(args.days, existing, args.verbose)
    if args.days == "all" and not existing:
        print("Collecting all available CI jobs")
    else:
        print(f"Collecting CI jobs since {start_date.isoformat()}")

    # Fetch completed runs
    runs = fetch_completed_runs(args.repo, start_date, args.verbose)
    if not runs:
        print("No completed runs found in the specified time range.")
        if existing_count > 0:
            print(f"Existing dataset contains {existing_count} jobs.")
        return

    # Filter by workflow name if specified
    if args.workflow:
        before = len(runs)
        runs = [r for r in runs if r.get("name") == args.workflow]
        print(f"Filtered to '{args.workflow}' workflow: {len(runs)} of {before} runs")

    # Fetch jobs for all runs (with incremental saves)
    merged = collect_jobs(
        args.repo, runs, args.output, existing, args.verbose
    )
    new_count = len(merged) - existing_count
    save_data(merged, args.output)

    print(f"Added {new_count} new jobs (total: {len(merged)})")
    print(f"Data saved to: {args.output}")


if __name__ == "__main__":
    main()
