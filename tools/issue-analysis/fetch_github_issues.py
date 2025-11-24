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
from typing import Dict, List, Any, Tuple
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

def fetch_all_pages(base_url: str, params: Dict[str, str], since: str = None) -> List[Dict[str, Any]]:
    """Fetch all pages of results from GitHub API."""
    all_items = []
    page = 1
    per_page = 100

    while True:
        params_with_page = {**params, "page": str(page), "per_page": str(per_page)}
        if since:
            params_with_page["since"] = since
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

def fetch_issues_and_prs(since: str = None):
    """Fetch all issues and pull requests, optionally since a specific date."""
    base_url = f"https://api.github.com/repos/{REPO_OWNER}/{REPO_NAME}/issues"

    if since:
        print("\n=== Fetching Updated Issues and Pull Requests (Incremental) ===")
        print(f"Fetching items updated since: {since}")
    else:
        print("\n=== Fetching Issues and Pull Requests ===")
        print("Note: GitHub API returns both issues and PRs in the /issues endpoint")

    # Fetch all (both open and closed)
    all_items = []
    for state in ["open", "closed"]:
        print(f"\nFetching {state} items...")
        items = fetch_all_pages(base_url, {"state": state}, since=since)
        all_items.extend(items)

    # Separate issues from PRs
    issues = [item for item in all_items if "pull_request" not in item]
    prs = [item for item in all_items if "pull_request" in item]

    print(f"\n=== Summary ===")
    print(f"Total items fetched: {len(all_items)}")
    print(f"Issues: {len(issues)}")
    print(f"Pull Requests: {len(prs)}")

    return issues, prs

def extract_issues_from_pr(pr: Dict[str, Any]) -> List[int]:
    """
    Extract issue numbers referenced in PR title and body.
    Looks for patterns like: fixes #123, closes #456, resolves #789

    This matches GitHub's own issue linking behavior.
    """
    import re

    title = pr.get("title", "")
    body = pr.get("body") or ""
    text = f"{title} {body}"

    # Pattern matches: fix/fixes/fixed/close/closes/closed/resolve/resolves/resolved #NUMBER
    # Also matches bare #NUMBER references
    pattern = r'(?:fix(?:es|ed)?|close(?:s|d)?|resolve(?:s|d)?)\s*#(\d+)|(?:^|\s)#(\d+)'

    issue_numbers = []
    for match in re.finditer(pattern, text, re.IGNORECASE):
        # match.group(1) is from fix/close/resolve pattern
        # match.group(2) is from bare # pattern
        num = match.group(1) or match.group(2)
        if num:
            issue_numbers.append(int(num))

    # Remove duplicates and sort
    return sorted(list(set(issue_numbers)))

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


def load_existing_data() -> Tuple[List[Dict], List[Dict], Dict]:
    """Load existing data if available."""
    issues_file = OUTPUT_DIR / "issues.json"
    prs_file = OUTPUT_DIR / "pull_requests.json"
    metadata_file = OUTPUT_DIR / "metadata.json"

    issues = []
    prs = []
    metadata = {}

    if issues_file.exists():
        with open(issues_file, "r") as f:
            issues = json.load(f)

    if prs_file.exists():
        with open(prs_file, "r") as f:
            prs = json.load(f)

    if metadata_file.exists():
        with open(metadata_file, "r") as f:
            metadata = json.load(f)

    return issues, prs, metadata

def merge_data(existing: List[Dict], new: List[Dict]) -> List[Dict]:
    """Merge new data with existing data, avoiding duplicates."""
    # Create a dictionary keyed by issue/PR number for fast lookup
    merged = {item["number"]: item for item in existing}

    # Update or add new items
    for item in new:
        merged[item["number"]] = item

    # Return as list, sorted by number
    return sorted(merged.values(), key=lambda x: x["number"])

def save_data(issues: List[Dict], prs: List[Dict], enrichments: Dict[str, bool] = None):
    """Save fetched data to JSON files."""
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    timestamp = datetime.now().isoformat()
    metadata = {
        "fetched_at": timestamp,
        "repo": f"{REPO_OWNER}/{REPO_NAME}",
        "issue_count": len(issues),
        "pr_count": len(prs),
        "enrichments": enrichments or {}
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

def add_issue_references_to_prs(prs: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
    """
    Extract issue references from PR titles and bodies.
    This is fast (no API calls) and matches GitHub's linking behavior.
    """
    print("\n=== Extracting Issue References from PRs ===")

    total_refs = 0
    for pr in prs:
        issue_refs = extract_issues_from_pr(pr)
        pr["referenced_issues"] = issue_refs
        total_refs += len(issue_refs)

    print(f"✓ Found {total_refs} issue references across {len(prs)} PRs")
    return prs

def enrich_prs_with_files(prs: List[Dict[str, Any]], only_new: bool = False) -> List[Dict[str, Any]]:
    """
    Enrich PRs with file change information.
    WARNING: This makes additional API calls and can be slow!
    Only run this for detailed analysis.

    Args:
        prs: List of PRs to enrich
        only_new: If True, only enrich PRs that don't already have files_changed
    """
    # Filter to only PRs that need enrichment
    if only_new:
        prs_to_enrich = [pr for pr in prs if "files_changed" not in pr]
        already_enriched = len(prs) - len(prs_to_enrich)
    else:
        prs_to_enrich = prs
        already_enriched = 0

    if not prs_to_enrich:
        print("\n=== All PRs Already Have File Changes ===")
        return prs

    print("\n=== Enriching PRs with File Changes ===")
    print(f"PRs to enrich: {len(prs_to_enrich)}")
    if already_enriched > 0:
        print(f"Already enriched: {already_enriched}")
    print("This may take a while and consume API rate limit.")

    # Create a mapping for quick lookup
    pr_map = {pr["number"]: pr for pr in prs}

    for i, pr in enumerate(prs_to_enrich):
        if (i + 1) % 50 == 0:
            print(f"Processed {i + 1}/{len(prs_to_enrich)} PRs...")

        # Fetch files changed in this PR
        files = fetch_pr_files(pr["number"])
        pr_map[pr["number"]]["files_changed"] = files

        # Rate limiting protection
        if (i + 1) % 100 == 0:
            print("Pausing to respect rate limits...")
            time.sleep(2)

    print(f"✓ Enriched {len(prs_to_enrich)} PRs (skipped {already_enriched} already enriched)")
    return list(pr_map.values())

def main():
    """Main entry point."""
    import argparse

    parser = argparse.ArgumentParser(description="Fetch Slang GitHub issues and PRs")
    parser.add_argument(
        "--incremental",
        action="store_true",
        help="Incremental update: fetch only items updated since last fetch"
    )
    args = parser.parse_args()

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
        # Check for incremental update
        since = None
        existing_issues = []
        existing_prs = []
        previous_enrichments = {}

        if args.incremental:
            existing_issues, existing_prs, metadata = load_existing_data()
            if metadata and "fetched_at" in metadata:
                since = metadata["fetched_at"]
                previous_enrichments = metadata.get("enrichments", {})

                print(f"=== Incremental Update Mode ===")
                print(f"Existing data: {len(existing_issues)} issues, {len(existing_prs)} PRs")
                print(f"Last fetch: {since}")

                # Show previous enrichments info
                if previous_enrichments:
                    print(f"Previous enrichments detected: {', '.join(k for k, v in previous_enrichments.items() if v)}")

                print(f"Fetching updates since then...\n")
            else:
                print("No existing data found. Performing full fetch.")
                args.incremental = False

        # Fetch new/updated data
        new_issues, new_prs = fetch_issues_and_prs(since=since if args.incremental else None)

        # Merge with existing data if incremental
        if args.incremental and existing_issues:
            print(f"\n=== Merging Data ===")
            print(f"New items fetched: {len(new_issues)} issues, {len(new_prs)} PRs")
            issues = merge_data(existing_issues, new_issues)
            prs = merge_data(existing_prs, new_prs)
            print(f"After merge: {len(issues)} issues, {len(prs)} PRs")
        else:
            issues = new_issues
            prs = new_prs

        # Always add issue references from PRs (fast, no API calls!)
        prs = add_issue_references_to_prs(prs)

        # Always enrich PRs with file changes (only new ones in incremental mode)
        if args.incremental and existing_prs:
            # In incremental mode, only enrich PRs that don't have files yet
            print(f"\n→ Smart enrichment: Only fetching files for new/updated PRs")
            prs = enrich_prs_with_files(prs, only_new=True)
        else:
            prs = enrich_prs_with_files(prs, only_new=False)

        # Track which enrichments were applied
        enrichments = {
            "pr_files": True,  # Always included now
            "issue_references": True  # Always included now
        }

        save_data(issues, prs, enrichments)
        print("\n✓ Data fetch complete!")

        if args.incremental:
            print("\nIncremental update successful! Data merged with existing.")

        print("\nData enrichments:")
        print("  ✓ PRs include 'referenced_issues' field (issue numbers from PR title/body)")
        print("  ✓ PRs include 'files_changed' field (detailed file change information)")

    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main()

