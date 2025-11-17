#!/usr/bin/env python3
"""
Fetch GitHub issues and PRs from shader-slang/slang repository.
Saves data locally for analysis.
"""

import json
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Any
import urllib.request
import urllib.error
import time
import ssl

# Configuration
REPO_OWNER = "shader-slang"
REPO_NAME = "slang"
OUTPUT_DIR = Path(__file__).parent / "data"
GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN", "")

# Create SSL context that works on macOS
def get_ssl_context():
    """Create SSL context for HTTPS requests."""
    try:
        # Try to use certifi's certificates if available
        import certifi
        return ssl.create_default_context(cafile=certifi.where())
    except ImportError:
        # certifi not available, try default
        try:
            return ssl.create_default_context()
        except Exception:
            pass
    except Exception:
        pass

    # Fallback: create unverified context (not ideal but works)
    print("Warning: Using unverified SSL context (certificate verification disabled)")
    return ssl._create_unverified_context()

def make_github_request(url: str) -> Dict[str, Any]:
    """Make a request to GitHub API with authentication if available."""
    headers = {
        "Accept": "application/vnd.github.v3+json",
        "User-Agent": "Slang-Issue-Analyzer"
    }
    if GITHUB_TOKEN:
        headers["Authorization"] = f"token {GITHUB_TOKEN}"

    request = urllib.request.Request(url, headers=headers)
    try:
        ssl_context = get_ssl_context()
        with urllib.request.urlopen(request, context=ssl_context) as response:
            return json.loads(response.read().decode())
    except urllib.error.HTTPError as e:
        print(f"HTTP Error {e.code}: {e.reason}")
        if e.code == 403:
            print("Rate limit likely exceeded. Set GITHUB_TOKEN environment variable.")
        raise

def fetch_all_pages(base_url: str, params: Dict[str, str]) -> List[Dict[str, Any]]:
    """Fetch all pages of results from GitHub API."""
    all_items = []
    page = 1
    per_page = 100

    while True:
        params_with_page = {**params, "page": str(page), "per_page": str(per_page)}
        query_string = "&".join(f"{k}={v}" for k, v in params_with_page.items())
        url = f"{base_url}?{query_string}"

        print(f"Fetching page {page}...", end=" ", flush=True)
        items = make_github_request(url)

        if not items:
            print("Done!")
            break

        all_items.extend(items)
        print(f"Got {len(items)} items (total: {len(all_items)})")

        if len(items) < per_page:
            break

        page += 1
        time.sleep(0.5)  # Be nice to GitHub API

    return all_items

def fetch_issues_and_prs():
    """Fetch all issues and pull requests."""
    base_url = f"https://api.github.com/repos/{REPO_OWNER}/{REPO_NAME}/issues"

    print("\n=== Fetching Issues and Pull Requests ===")
    print("Note: GitHub API returns both issues and PRs in the /issues endpoint")

    # Fetch all (both open and closed)
    all_items = []
    for state in ["open", "closed"]:
        print(f"\nFetching {state} items...")
        items = fetch_all_pages(base_url, {"state": state})
        all_items.extend(items)

    # Separate issues from PRs
    issues = [item for item in all_items if "pull_request" not in item]
    prs = [item for item in all_items if "pull_request" in item]

    print(f"\n=== Summary ===")
    print(f"Total items fetched: {len(all_items)}")
    print(f"Issues: {len(issues)}")
    print(f"Pull Requests: {len(prs)}")

    return issues, prs

def fetch_comments_for_issue(issue_number: int) -> List[Dict[str, Any]]:
    """Fetch all comments for a specific issue."""
    url = f"https://api.github.com/repos/{REPO_OWNER}/{REPO_NAME}/issues/{issue_number}/comments"
    try:
        return fetch_all_pages(url, {})
    except Exception as e:
        print(f"Error fetching comments for issue #{issue_number}: {e}")
        return []

def fetch_timeline_for_issue(issue_number: int) -> List[Dict[str, Any]]:
    """Fetch timeline events for a specific issue to find related PRs."""
    url = f"https://api.github.com/repos/{REPO_OWNER}/{REPO_NAME}/issues/{issue_number}/timeline"
    headers = {
        "Accept": "application/vnd.github.mockingbird-preview+json",  # Required for timeline
        "User-Agent": "Slang-Issue-Analyzer"
    }
    if GITHUB_TOKEN:
        headers["Authorization"] = f"token {GITHUB_TOKEN}"

    request = urllib.request.Request(url, headers=headers)
    try:
        ssl_context = get_ssl_context()
        with urllib.request.urlopen(request, context=ssl_context) as response:
            return json.loads(response.read().decode())
    except Exception as e:
        print(f"Error fetching timeline for issue #{issue_number}: {e}")
        return []

def fetch_pr_files(pr_number: int) -> List[Dict[str, Any]]:
    """Fetch list of files changed in a PR."""
    url = f"https://api.github.com/repos/{REPO_OWNER}/{REPO_NAME}/pulls/{pr_number}/files"
    try:
        files = fetch_all_pages(url, {})
        # Extract key information from each file
        return [{
            "filename": f.get("filename"),
            "status": f.get("status"),  # "added", "removed", "modified", "renamed"
            "additions": f.get("additions", 0),
            "deletions": f.get("deletions", 0),
            "changes": f.get("changes", 0),
        } for f in files]
    except Exception as e:
        print(f"Error fetching files for PR #{pr_number}: {e}")
        return []

def extract_related_prs_from_issue(issue: Dict[str, Any]) -> List[int]:
    """Extract PR numbers that are related to this issue."""
    related_prs = []

    # Method 1: Check if closed by a PR via timeline
    if issue.get("state") == "closed":
        timeline = fetch_timeline_for_issue(issue["number"])
        for event in timeline:
            if event.get("event") == "closed" and event.get("source"):
                source = event["source"]
                if source.get("type") == "issue" and "pull_request" in source.get("issue", {}):
                    pr_number = source["issue"]["number"]
                    related_prs.append(pr_number)

    # Method 2: Parse body and comments for "fixes #123" patterns
    body = issue.get("body") or ""
    # Common patterns: "fixes #123", "closes #123", "resolves #123"
    import re
    # Look for PR references (issues that reference this issue)
    # This is a bit backwards - we're looking for mentions in the body

    return list(set(related_prs))  # Remove duplicates

def save_data(issues: List[Dict], prs: List[Dict]):
    """Save fetched data to JSON files."""
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    timestamp = datetime.now().isoformat()
    metadata = {
        "fetched_at": timestamp,
        "repo": f"{REPO_OWNER}/{REPO_NAME}",
        "issue_count": len(issues),
        "pr_count": len(prs)
    }

    # Save issues
    issues_file = OUTPUT_DIR / "issues.json"
    with open(issues_file, "w") as f:
        json.dump(issues, f, indent=2)
    print(f"\nSaved {len(issues)} issues to {issues_file}")

    # Save PRs
    prs_file = OUTPUT_DIR / "pull_requests.json"
    with open(prs_file, "w") as f:
        json.dump(prs, f, indent=2)
    print(f"Saved {len(prs)} pull requests to {prs_file}")

    # Save metadata
    metadata_file = OUTPUT_DIR / "metadata.json"
    with open(metadata_file, "w") as f:
        json.dump(metadata, f, indent=2)
    print(f"Saved metadata to {metadata_file}")

def enrich_issues_with_pr_links(issues: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
    """
    Enrich issues with related PR information.
    WARNING: This makes additional API calls and can be slow!
    Only run this for detailed analysis.
    """
    print("\n=== Enriching Issues with PR Links ===")
    print(f"This will fetch timeline data for {len(issues)} issues...")
    print("This may take a while and consume API rate limit.")

    enriched_issues = []
    for i, issue in enumerate(issues):
        if (i + 1) % 50 == 0:
            print(f"Processed {i + 1}/{len(issues)} issues...")

        # Add related PRs to issue
        related_prs = extract_related_prs_from_issue(issue)
        issue_copy = issue.copy()
        issue_copy["related_prs"] = related_prs
        enriched_issues.append(issue_copy)

        # Rate limiting protection
        if (i + 1) % 100 == 0:
            print("Pausing to respect rate limits...")
            time.sleep(2)

    print(f"✓ Enriched all {len(enriched_issues)} issues")
    return enriched_issues

def enrich_prs_with_files(prs: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
    """
    Enrich PRs with file change information.
    WARNING: This makes additional API calls and can be slow!
    Only run this for detailed analysis.
    """
    print("\n=== Enriching PRs with File Changes ===")
    print(f"This will fetch file changes for {len(prs)} PRs...")
    print("This may take a while and consume API rate limit.")

    enriched_prs = []
    for i, pr in enumerate(prs):
        if (i + 1) % 50 == 0:
            print(f"Processed {i + 1}/{len(prs)} PRs...")

        # Fetch files changed in this PR
        files = fetch_pr_files(pr["number"])
        pr_copy = pr.copy()
        pr_copy["files_changed"] = files
        enriched_prs.append(pr_copy)

        # Rate limiting protection
        if (i + 1) % 100 == 0:
            print("Pausing to respect rate limits...")
            time.sleep(2)

    print(f"✓ Enriched all {len(enriched_prs)} PRs")
    return enriched_prs

def main():
    """Main entry point."""
    import argparse

    parser = argparse.ArgumentParser(description="Fetch Slang GitHub issues and PRs")
    parser.add_argument(
        "--enrich",
        action="store_true",
        help="Enrich issues with PR relationships (slow, uses more API calls)"
    )
    parser.add_argument(
        "--pr-files",
        action="store_true",
        help="Fetch file changes for each PR (slow, uses more API calls)"
    )
    parser.add_argument(
        "--full",
        action="store_true",
        help="Full enrichment: both issue-PR links and PR files (very slow!)"
    )
    args = parser.parse_args()

    # --full implies both --enrich and --pr-files
    if args.full:
        args.enrich = True
        args.pr_files = True

    if not GITHUB_TOKEN:
        print("WARNING: GITHUB_TOKEN environment variable not set.")
        print("You may hit rate limits quickly without authentication.")
        print("Create a token at: https://github.com/settings/tokens")
        response = input("\nContinue anyway? (y/N): ")
        if response.lower() != 'y':
            sys.exit(1)

    print(f"\nFetching data from {REPO_OWNER}/{REPO_NAME}")
    print(f"Output directory: {OUTPUT_DIR}\n")

    try:
        issues, prs = fetch_issues_and_prs()

        # Optionally enrich with PR links
        if args.enrich:
            issues = enrich_issues_with_pr_links(issues)

        # Optionally enrich PRs with file changes
        if args.pr_files:
            prs = enrich_prs_with_files(prs)

        save_data(issues, prs)
        print("\n✓ Data fetch complete!")

        if args.enrich:
            print("\nNote: Issues have been enriched with 'related_prs' field")
        if args.pr_files:
            print("Note: PRs have been enriched with 'files_changed' field")

    except Exception as e:
        print(f"\n✗ Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()

