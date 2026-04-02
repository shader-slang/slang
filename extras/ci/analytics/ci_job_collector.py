#!/usr/bin/env python3
"""
GitHub Actions CI Job Collector

Collects completed workflow run and job data from GitHub Actions API
and saves it as JSON. Supports incremental updates and monthly file splitting.

Usage:
    python3 ci_job_collector.py --output-dir ./data          # monthly files in ./data
    python3 ci_job_collector.py --output-dir ./data --workflow CI
    python3 ci_job_collector.py --output ci_jobs.json        # legacy single-file mode
    python3 ci_job_collector.py --migrate ci_jobs.json --output-dir ./data

Requires: gh CLI (authenticated)
"""

import argparse
import datetime
import glob
import json
import os
import sys
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import timezone

# Import shared helpers from parent ci/ directory
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from gh_api import gh_api_list

DEFAULT_REPO = "shader-slang/slang"
DEFAULT_DAYS = 7
MONTHLY_PREFIX = "ci_jobs_"


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
        "--output-dir",
        help="Output directory for monthly JSON files (e.g. ci_jobs_2026-03.json).",
    )
    parser.add_argument(
        "--output",
        help="Legacy: output as a single JSON file.",
    )
    parser.add_argument(
        "--migrate",
        metavar="FILE",
        help="Migrate a single ci_jobs.json into monthly files in --output-dir.",
    )
    parser.add_argument(
        "--workflow",
        help="Only collect runs for this workflow name (e.g. 'CI'). Reduces API calls significantly.",
    )
    parser.add_argument(
        "--verbose", action="store_true", help="Enable verbose output"
    )
    parser.add_argument(
        "--max-job-fetch-error-rate",
        type=float,
        default=0.05,
        help="Maximum tolerated fraction of run job-fetch failures before exiting non-zero (default: 0.05).",
    )
    return parser.parse_args()


# --- Data I/O helpers ---


def load_existing_data(file_path, verbose=False):
    """Load existing job data from a single file if it exists."""
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


def load_monthly_file(directory, month_str, verbose=False):
    """Load a single monthly file. Returns list of jobs."""
    path = os.path.join(directory, f"{MONTHLY_PREFIX}{month_str}.json")
    return load_existing_data(path, verbose)


def find_monthly_files(directory):
    """Find all monthly files in directory, return sorted list of (month_str, path)."""
    pattern = os.path.join(directory, f"{MONTHLY_PREFIX}*.json")
    files = sorted(glob.glob(pattern))
    result = []
    for path in files:
        basename = os.path.basename(path)
        # Extract YYYY-MM from ci_jobs_YYYY-MM.json
        month_str = basename[len(MONTHLY_PREFIX) : -len(".json")]
        if len(month_str) == 7 and month_str[4] == "-":
            result.append((month_str, path))
    return result


def load_all_monthly_data(directory, verbose=False):
    """Load all monthly files and return a flat list of all jobs."""
    monthly_files = find_monthly_files(directory)
    if not monthly_files:
        if verbose:
            print(f"No monthly data files in {directory}")
        return []
    all_jobs = []
    for month_str, path in monthly_files:
        jobs = load_existing_data(path, verbose=False)
        all_jobs.extend(jobs)
    if verbose:
        print(
            f"Loaded {len(all_jobs)} jobs from {len(monthly_files)} monthly files in {directory}"
        )
    return all_jobs


def load_recent_monthly_data(directory, verbose=False):
    """Load only the most recent monthly file to determine incremental start date."""
    monthly_files = find_monthly_files(directory)
    if not monthly_files:
        return []
    # Last file is the most recent month
    month_str, path = monthly_files[-1]
    return load_existing_data(path, verbose)


def job_month(job):
    """Extract YYYY-MM from a job's created_at timestamp."""
    created = job.get("created_at") or ""
    if len(created) >= 7:
        return created[:7]
    return "unknown"


def save_data(data, file_path):
    """Save job data as compact JSON."""
    directory = os.path.dirname(file_path)
    if directory:
        os.makedirs(directory, exist_ok=True)
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(data, f, separators=(",", ":"), default=str)


def save_monthly_data(all_jobs, output_dir, changed_months=None):
    """Save jobs split into monthly files. Only writes months in changed_months if given."""
    os.makedirs(output_dir, exist_ok=True)
    by_month = defaultdict(list)
    for job in all_jobs:
        by_month[job_month(job)].append(job)
    written = 0
    for month_str in sorted(by_month):
        if changed_months is not None and month_str not in changed_months:
            continue
        path = os.path.join(output_dir, f"{MONTHLY_PREFIX}{month_str}.json")
        save_data(by_month[month_str], path)
        written += 1
    return written


def months_in_jobs(jobs):
    """Return the set of month strings represented by the jobs' created_at values."""
    return {month for month in (job_month(job) for job in jobs) if month != "unknown"}


def migrate_single_to_monthly(single_file, output_dir, verbose=False):
    """Migrate a single ci_jobs.json into monthly files."""
    data = load_existing_data(single_file, verbose)
    if not data:
        print(f"No data to migrate from {single_file}")
        return
    by_month = defaultdict(list)
    for job in data:
        by_month[job_month(job)].append(job)
    os.makedirs(output_dir, exist_ok=True)
    for month_str in sorted(by_month):
        path = os.path.join(output_dir, f"{MONTHLY_PREFIX}{month_str}.json")
        save_data(by_month[month_str], path)
        print(f"  {month_str}: {len(by_month[month_str])} jobs -> {os.path.basename(path)}")
    print(f"Migrated {len(data)} jobs into {len(by_month)} monthly files in {output_dir}")


# --- GitHub API helpers ---


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
        print(
            f"Error: --days must be a number or 'all', got '{days_str}'",
            file=sys.stderr,
        )
        sys.exit(1)
    start = datetime.datetime.now(timezone.utc) - datetime.timedelta(days=days)
    start = start.replace(hour=0, minute=0, second=0, microsecond=0)
    if verbose:
        print(f"Collecting jobs from past {days} days (since {start.isoformat()})")
    return start


API_PAGINATION_CAP = 1000


class DataCompletenessError(RuntimeError):
    """Raised when API pagination limits prevent complete data collection."""


def resolve_workflow_id(repo, workflow_name):
    """Resolve a workflow name to workflow id."""
    workflows, err = gh_api_list(
        f"repos/{repo}/actions/workflows?per_page=100",
        "workflows",
    )
    if err:
        print(
            f"Warning: Could not resolve workflow '{workflow_name}': {err}",
            file=sys.stderr,
        )
        return None
    for wf in workflows or []:
        if wf.get("name") == workflow_name:
            return wf.get("id")
    return None


def _fetch_runs_window(
    repo, date_from, date_to, seen_ids, workflow_id=None, depth=0
):
    """Fetch runs for a time window, subdividing if the API cap is hit.

    The GitHub API returns at most 1000 results per paginated query.
    When a window hits this cap, we split it in half and fetch both
    halves recursively. Stops subdividing below 1-hour windows.
    """
    from_str = date_from.strftime("%Y-%m-%dT%H:%M:%SZ")
    to_str = date_to.strftime("%Y-%m-%dT%H:%M:%SZ")

    endpoint = (
        f"repos/{repo}/actions/workflows/{workflow_id}/runs"
        if workflow_id
        else f"repos/{repo}/actions/runs"
    )
    runs, err = gh_api_list(
        f"{endpoint}?status=completed&per_page=100&created={from_str}..{to_str}",
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
            left = _fetch_runs_window(
                repo, date_from, mid, seen_ids, workflow_id, depth + 1
            )
            right = _fetch_runs_window(
                repo, mid, date_to, seen_ids, workflow_id, depth + 1
            )
            return new_runs + left + right
        raise DataCompletenessError(
            f"Window {from_str}..{to_str} returned {len(runs)} runs and cannot be split further "
            f"without risking missing data."
        )

    for r in new_runs:
        seen_ids.add(r["id"])
    return new_runs


def fetch_completed_runs(repo, since_date, workflow_id=None, verbose=False):
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

        day_runs = _fetch_runs_window(
            repo, current, next_day, seen_ids, workflow_id
        )
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
        return [], err
    return (jobs if isinstance(jobs, list) else []), None


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


def collect_jobs(
    repo, runs, existing=None, output_dir=None, output_path=None, verbose=False
):
    """Fetch jobs for all runs in parallel and extract data.

    Saves incrementally every 500 runs to avoid losing progress on
    rate limit hits or interruptions.
    """
    all_jobs = list(existing) if existing else []
    existing_ids = {j["id"] for j in all_jobs}
    total = len(runs)
    initial_count = len(all_jobs)

    print(f"Fetching jobs for {total} runs...", file=sys.stderr)

    failed_job_fetches = 0
    failed_run_ids = []
    with ThreadPoolExecutor(max_workers=8) as executor:
        futures = {
            executor.submit(fetch_jobs_for_run, repo, run["id"]): run
            for run in runs
        }

        done = 0
        for future in as_completed(futures):
            run = futures[future]
            jobs, err = future.result()
            if err:
                failed_job_fetches += 1
                if len(failed_run_ids) < 10:
                    failed_run_ids.append(run["id"])
                if verbose:
                    print(
                        f"  Warning: failed to fetch jobs for run {run['id']}: {err}",
                        file=sys.stderr,
                    )
                done += 1
                continue
            for job in jobs:
                if (
                    job.get("status") == "completed"
                    and job["id"] not in existing_ids
                ):
                    all_jobs.append(extract_job_data(job, run))
                    existing_ids.add(job["id"])

            done += 1
            if done % 50 == 0 or done == total:
                print(
                    f"  Progress: {done}/{total} runs ({len(all_jobs) - initial_count} new jobs)",
                    file=sys.stderr,
                )
            # Save incrementally every 500 runs
            if done % 500 == 0:
                if output_dir:
                    save_monthly_data(all_jobs, output_dir)
                elif output_path:
                    save_data(all_jobs, output_path)
                print(
                    f"  Checkpoint saved: {len(all_jobs)} jobs",
                    file=sys.stderr,
                )

    return all_jobs, failed_job_fetches, failed_run_ids


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


# --- Main ---


def main():
    args = parse_args()

    if not args.output_dir and not args.output:
        args.output_dir = "."

    # Handle migration mode
    if args.migrate:
        if not args.output_dir:
            print("Error: --migrate requires --output-dir", file=sys.stderr)
            sys.exit(1)
        migrate_single_to_monthly(args.migrate, args.output_dir, args.verbose)
        return

    use_monthly = args.output_dir is not None

    # Load existing data
    if use_monthly:
        recent = load_recent_monthly_data(args.output_dir, args.verbose)
        existing_count = len(recent)
        if existing_count > 0:
            print(f"Loaded {existing_count} jobs from latest monthly file")
    else:
        recent = load_existing_data(args.output, args.verbose)
        existing_count = len(recent)
        if existing_count > 0:
            print(f"Loaded {existing_count} existing jobs from {args.output}")

    # Determine start date
    start_date = get_start_date(args.days, recent, args.verbose)
    if args.days == "all" and not recent:
        print("Collecting all available CI jobs")
    else:
        print(f"Collecting CI jobs since {start_date.isoformat()}")

    # Fetch completed runs
    workflow_id = None
    if args.workflow:
        workflow_id = resolve_workflow_id(args.repo, args.workflow)
        if workflow_id:
            print(f"Resolved workflow '{args.workflow}' to ID {workflow_id}")
        else:
            print(
                f"Warning: workflow '{args.workflow}' was not resolved; falling back to repository-wide run fetch.",
                file=sys.stderr,
            )

    try:
        runs = fetch_completed_runs(
            args.repo, start_date, workflow_id, args.verbose
        )
    except DataCompletenessError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(2)
    if not runs:
        print("No completed runs found in the specified time range.")
        if existing_count > 0:
            print(f"Existing dataset contains {existing_count} jobs.")
        return

    # Keep name-based filter for safety (in case workflow resolution fails)
    if args.workflow:
        before = len(runs)
        runs = [r for r in runs if r.get("name") == args.workflow]
        print(
            f"Filtered to '{args.workflow}' workflow: {len(runs)} of {before} runs"
        )

    if use_monthly:
        fetched_jobs, failed_job_fetches, failed_run_ids = collect_jobs(
            args.repo,
            runs,
            verbose=args.verbose,
        )
        changed_months = months_in_jobs(fetched_jobs)
        existing_for_merge = []
        for month in changed_months:
            existing_for_merge.extend(
                load_monthly_file(args.output_dir, month, args.verbose)
            )
        if existing_for_merge:
            print(
                f"Loaded {len(existing_for_merge)} existing jobs from {len(changed_months)} overlapping months"
            )

        merged = merge_data(existing_for_merge, fetched_jobs, args.verbose)
        new_count = len(merged) - len(existing_for_merge)

        written = save_monthly_data(merged, args.output_dir, changed_months)
        print(f"Added {new_count} new jobs (wrote {written} monthly files)")
    else:
        # Legacy single-file mode
        merged, failed_job_fetches, failed_run_ids = collect_jobs(
            args.repo,
            runs,
            existing=recent,
            output_path=args.output,
            verbose=args.verbose,
        )
        new_count = len(merged) - existing_count
        save_data(merged, args.output)
        print(f"Added {new_count} new jobs (total: {len(merged)})")
        print(f"Data saved to: {args.output}")

    total_runs = len(runs)
    error_rate = (failed_job_fetches / total_runs) if total_runs else 0
    if failed_job_fetches:
        print(
            f"Warning: failed to fetch jobs for {failed_job_fetches}/{total_runs} runs "
            f"({error_rate * 100:.1f}%).",
            file=sys.stderr,
        )
        if failed_run_ids:
            print(
                f"Sample failed run IDs: {', '.join(str(i) for i in failed_run_ids)}",
                file=sys.stderr,
            )
    if error_rate > args.max_job_fetch_error_rate:
        print(
            f"Error: job fetch error rate {error_rate * 100:.1f}% exceeds allowed "
            f"{args.max_job_fetch_error_rate * 100:.1f}%",
            file=sys.stderr,
        )
        sys.exit(2)


if __name__ == "__main__":
    main()
