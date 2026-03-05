#!/usr/bin/env python3
"""
GitHub Merged PR Collector

Collects merged pull request data from the GitHub API and saves it as JSON.
Supports incremental updates — if the output file exists, only newer PRs
are fetched and merged.

Usage:
    python3 pr_collector.py                          # last 30 days, shader-slang/slang
    python3 pr_collector.py --repo OWNER/REPO
    python3 pr_collector.py --days 90
    python3 pr_collector.py --output pr_merges.json

Requires: gh CLI (authenticated)
"""

import argparse
import datetime
import json
import os
import sys
from datetime import timezone

# Import shared helpers from parent ci/ directory
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from gh_api import gh_api_list

DEFAULT_REPO = "shader-slang/slang"
DEFAULT_DAYS = 30
DEFAULT_OUTPUT = "pr_merges.json"


def parse_args():
    parser = argparse.ArgumentParser(
        description="Collect merged PR data from GitHub and save as JSON."
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
        "--verbose", action="store_true", help="Enable verbose output"
    )
    return parser.parse_args()


def load_existing_data(file_path, verbose=False):
    """Load existing PR data from file if it exists."""
    if not os.path.exists(file_path):
        if verbose:
            print(f"No existing data at {file_path}")
        return []
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            data = json.load(f)
            if verbose:
                print(f"Loaded {len(data)} existing PRs from {file_path}")
            return data
    except (json.JSONDecodeError, OSError) as e:
        print(f"Warning: Could not load existing data: {e}", file=sys.stderr)
        return []


def get_start_date(days_str, existing_data, verbose=False):
    """Determine the start date for fetching."""
    if existing_data:
        latest = None
        for pr in existing_data:
            merged = pr.get("merged_at")
            if not merged:
                continue
            try:
                dt = datetime.datetime.fromisoformat(
                    merged.replace("Z", "+00:00")
                )
                if latest is None or dt > latest:
                    latest = dt
            except (ValueError, TypeError):
                continue

        if latest:
            start = latest.replace(hour=0, minute=0, second=0, microsecond=0)
            if verbose:
                print(f"Incremental update from {start.isoformat()}")
            return start

    if days_str == "all":
        if verbose:
            print("Collecting all available PRs")
        return datetime.datetime(2019, 1, 1, tzinfo=timezone.utc)

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
        print(f"Collecting PRs from past {days} days (since {start.isoformat()})")
    return start


def fetch_merged_prs(repo, since_date, verbose=False):
    """Fetch merged PRs using the GitHub search API.

    Walks week by week to stay under API pagination limits.
    """
    now = datetime.datetime.now(timezone.utc)
    all_prs = []
    seen_numbers = set()

    current = since_date
    while current < now:
        next_window = current + datetime.timedelta(days=7)
        if next_window > now:
            next_window = now

        from_str = current.strftime("%Y-%m-%d")
        to_str = next_window.strftime("%Y-%m-%d")

        print(
            f"Fetching merged PRs for {from_str}..{to_str}...",
            file=sys.stderr,
            end="",
            flush=True,
        )

        prs, err = gh_api_list(
            f"search/issues?q=repo:{repo}+is:pr+is:merged"
            f"+merged:{from_str}..{to_str}&per_page=100&sort=updated",
            "items",
        )

        if err:
            print(f" error: {err}", file=sys.stderr)
            current = next_window
            continue

        new_count = 0
        for pr in prs or []:
            number = pr.get("number")
            if number and number not in seen_numbers:
                seen_numbers.add(number)
                all_prs.append(pr)
                new_count += 1

        print(f" {new_count} PRs", file=sys.stderr)
        current = next_window

    if verbose:
        print(f"Total: {len(all_prs)} merged PRs")
    return all_prs


def extract_pr_data(pr):
    """Extract relevant fields from a PR search result."""
    pr_info = pr.get("pull_request", {})
    merged_at = pr_info.get("merged_at") or pr.get("closed_at")

    return {
        "number": pr.get("number"),
        "title": pr.get("title", ""),
        "user": (pr.get("user") or {}).get("login", ""),
        "merged_at": merged_at,
        "created_at": pr.get("created_at", ""),
        "closed_at": pr.get("closed_at", ""),
        "html_url": pr.get("html_url", ""),
        "labels": [label.get("name", "") for label in (pr.get("labels") or [])],
    }


def save_data(data, file_path):
    """Save PR data as JSON."""
    directory = os.path.dirname(file_path)
    if directory:
        os.makedirs(directory, exist_ok=True)
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, default=str)


def main():
    args = parse_args()

    existing = load_existing_data(args.output, args.verbose)
    existing_count = len(existing)
    if existing_count > 0:
        print(f"Loaded {existing_count} existing PRs from {args.output}")

    start_date = get_start_date(args.days, existing, args.verbose)
    print(f"Collecting merged PRs since {start_date.isoformat()}")

    raw_prs = fetch_merged_prs(args.repo, start_date, args.verbose)
    if not raw_prs:
        print("No merged PRs found in the specified time range.")
        if existing_count > 0:
            print(f"Existing dataset contains {existing_count} PRs.")
        return

    existing_numbers = {pr["number"] for pr in existing}
    new_prs = []
    for pr in raw_prs:
        extracted = extract_pr_data(pr)
        if extracted["number"] not in existing_numbers:
            new_prs.append(extracted)
            existing_numbers.add(extracted["number"])

    merged = existing + new_prs
    merged.sort(key=lambda p: p.get("merged_at") or "")

    save_data(merged, args.output)
    print(f"Added {len(new_prs)} new PRs (total: {len(merged)})")
    print(f"Data saved to: {args.output}")


if __name__ == "__main__":
    main()
