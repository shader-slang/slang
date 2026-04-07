#!/usr/bin/env python3
"""
Ingest merged PRs from shader-slang/slang and shader-slang/slangpy
into a SQLite database with FTS5 full-text search indexes.

Requires: gh CLI authenticated (gh auth login)

Usage:
    python3 ingest.py                  # Full ingestion (skip already-stored PRs)
    python3 ingest.py --incremental    # Only fetch PRs merged after the latest stored date
    python3 ingest.py --repo slang     # Only ingest from one repo
    python3 ingest.py --limit 50       # Limit number of PRs to fetch (for testing)
"""

import argparse
import json
import sqlite3
import subprocess
import sys
import time
from pathlib import Path

DB_PATH = Path(__file__).parent / "pr-knowledge.db"
REPOS = {
    "slang": "shader-slang/slang",
    "slangpy": "shader-slang/slangpy",
}


def gh_api(endpoint, method="GET", params=None):
    """Call GitHub API via gh CLI."""
    cmd = ["gh", "api", endpoint, "--method", method, "--cache=0s"]
    if params:
        for k, v in params.items():
            cmd.extend(["-f", f"{k}={v}"])
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
        if result.returncode != 0:
            print(f"  WARNING: gh api {endpoint} failed: {result.stderr.strip()}", file=sys.stderr)
            return None
        return json.loads(result.stdout) if result.stdout.strip() else None
    except (subprocess.TimeoutExpired, json.JSONDecodeError) as e:
        print(f"  WARNING: gh api {endpoint} error: {e}", file=sys.stderr)
        return None


def gh_api_paginated(endpoint, per_page=100, max_pages=100):
    """Fetch all pages from a paginated GitHub API endpoint."""
    all_items = []
    for page in range(1, max_pages + 1):
        data = gh_api(endpoint, params={"per_page": str(per_page), "page": str(page)})
        if not data or len(data) == 0:
            break
        all_items.extend(data)
        if len(data) < per_page:
            break
        time.sleep(0.2)
    return all_items


def init_db():
    """Initialize SQLite database with tables and FTS5 indexes."""
    db = sqlite3.connect(str(DB_PATH))
    db.execute("PRAGMA journal_mode=WAL")
    db.executescript("""
        CREATE TABLE IF NOT EXISTS prs (
            number INTEGER,
            repo TEXT,
            title TEXT,
            author TEXT,
            body TEXT,
            merged_at TEXT,
            labels TEXT,
            files_json TEXT,
            state TEXT,
            created_at TEXT,
            PRIMARY KEY (number, repo)
        );
        CREATE TABLE IF NOT EXISTS reviews (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            pr_number INTEGER,
            repo TEXT,
            reviewer TEXT,
            body TEXT,
            state TEXT,
            submitted_at TEXT
        );
        CREATE TABLE IF NOT EXISTS review_comments (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            pr_number INTEGER,
            repo TEXT,
            reviewer TEXT,
            file_path TEXT,
            line INTEGER,
            body TEXT,
            created_at TEXT
        );
        CREATE TABLE IF NOT EXISTS issue_comments (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            pr_number INTEGER,
            repo TEXT,
            author TEXT,
            body TEXT,
            created_at TEXT
        );
    """)

    db.execute("""
        CREATE VIRTUAL TABLE IF NOT EXISTS prs_fts USING fts5(
            number, repo, title, body, content='prs', content_rowid='rowid'
        )
    """)
    db.execute("""
        CREATE VIRTUAL TABLE IF NOT EXISTS reviews_fts USING fts5(
            pr_number, repo, reviewer, body, content='reviews', content_rowid='id'
        )
    """)
    db.execute("""
        CREATE VIRTUAL TABLE IF NOT EXISTS review_comments_fts USING fts5(
            pr_number, repo, reviewer, file_path, body,
            content='review_comments', content_rowid='id'
        )
    """)
    db.commit()
    return db


def rebuild_fts(db):
    """Rebuild all FTS5 indexes."""
    print("Rebuilding FTS indexes...")
    db.execute("INSERT INTO prs_fts(prs_fts) VALUES('rebuild')")
    db.execute("INSERT INTO reviews_fts(reviews_fts) VALUES('rebuild')")
    db.execute("INSERT INTO review_comments_fts(review_comments_fts) VALUES('rebuild')")
    db.commit()


def get_stored_pr_numbers(db, repo):
    cursor = db.execute("SELECT number FROM prs WHERE repo = ?", (repo,))
    return {row[0] for row in cursor.fetchall()}


def get_latest_merged_at(db, repo):
    cursor = db.execute(
        "SELECT MAX(merged_at) FROM prs WHERE repo = ? AND merged_at IS NOT NULL",
        (repo,),
    )
    row = cursor.fetchone()
    return row[0] if row and row[0] else None


def fetch_pr_list(full_repo, since=None, limit=None):
    """Fetch list of merged PRs using gh pr list."""
    cmd = [
        "gh", "pr", "list",
        "--repo", full_repo,
        "--state", "merged",
        "--json", "number,title,author,mergedAt,createdAt,state,labels",
        "--limit", str(limit) if limit else "10000",
    ]
    if since:
        cmd.extend(["--search", f"merged:>={since[:10]}"])

    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
        if result.returncode != 0:
            print(f"  ERROR listing PRs: {result.stderr.strip()}", file=sys.stderr)
            return []
        return json.loads(result.stdout) if result.stdout.strip() else []
    except (subprocess.TimeoutExpired, json.JSONDecodeError) as e:
        print(f"  ERROR listing PRs: {e}", file=sys.stderr)
        return []


def fetch_pr_details(full_repo, pr_number):
    """Fetch full details for a single PR (body, files, reviews, comments)."""
    details = {}

    pr_data = gh_api(f"repos/{full_repo}/pulls/{pr_number}")
    if pr_data:
        details["body"] = pr_data.get("body", "") or ""
        details["state"] = pr_data.get("state", "")
        details["merged_at"] = pr_data.get("merged_at", "")
        details["created_at"] = pr_data.get("created_at", "")

    files = gh_api_paginated(f"repos/{full_repo}/pulls/{pr_number}/files")
    details["files"] = [
        {"filename": f.get("filename", ""), "patch": f.get("patch", "")[:2000]}
        for f in (files or [])
    ]

    reviews = gh_api_paginated(f"repos/{full_repo}/pulls/{pr_number}/reviews")
    details["reviews"] = [
        {
            "reviewer": r.get("user", {}).get("login", ""),
            "state": r.get("state", ""),
            "body": r.get("body", "") or "",
            "submitted_at": r.get("submitted_at", ""),
        }
        for r in (reviews or [])
        if r.get("body")
    ]

    review_comments = gh_api_paginated(f"repos/{full_repo}/pulls/{pr_number}/comments")
    details["review_comments"] = [
        {
            "reviewer": c.get("user", {}).get("login", ""),
            "file_path": c.get("path", ""),
            "line": c.get("original_line") or c.get("line") or 0,
            "body": c.get("body", "") or "",
            "created_at": c.get("created_at", ""),
        }
        for c in (review_comments or [])
    ]

    issue_comments = gh_api_paginated(f"repos/{full_repo}/issues/{pr_number}/comments")
    details["issue_comments"] = [
        {
            "author": c.get("user", {}).get("login", ""),
            "body": c.get("body", "") or "",
            "created_at": c.get("created_at", ""),
        }
        for c in (issue_comments or [])
    ]

    return details


def store_pr(db, repo, pr_meta, pr_details):
    """Store a PR and its associated reviews/comments in the database."""
    number = pr_meta["number"]
    title = pr_meta.get("title", "")
    author = pr_meta.get("author", {})
    if isinstance(author, dict):
        author = author.get("login", "")
    else:
        author = str(author)
    labels = ",".join(
        l.get("name", "") if isinstance(l, dict) else str(l)
        for l in (pr_meta.get("labels") or [])
    )

    db.execute(
        """INSERT OR REPLACE INTO prs
           (number, repo, title, author, body, merged_at, labels, files_json, state, created_at)
           VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)""",
        (
            number, repo, title, author,
            pr_details.get("body", ""),
            pr_details.get("merged_at") or pr_meta.get("mergedAt", ""),
            labels,
            json.dumps(pr_details.get("files", [])),
            pr_details.get("state", "merged"),
            pr_details.get("created_at") or pr_meta.get("createdAt", ""),
        ),
    )

    for r in pr_details.get("reviews", []):
        db.execute(
            """INSERT INTO reviews (pr_number, repo, reviewer, body, state, submitted_at)
               VALUES (?, ?, ?, ?, ?, ?)""",
            (number, repo, r["reviewer"], r["body"], r["state"], r.get("submitted_at", "")),
        )

    for c in pr_details.get("review_comments", []):
        db.execute(
            """INSERT INTO review_comments
               (pr_number, repo, reviewer, file_path, line, body, created_at)
               VALUES (?, ?, ?, ?, ?, ?, ?)""",
            (number, repo, c["reviewer"], c["file_path"], c["line"], c["body"], c.get("created_at", "")),
        )

    for c in pr_details.get("issue_comments", []):
        db.execute(
            """INSERT INTO issue_comments (pr_number, repo, author, body, created_at)
               VALUES (?, ?, ?, ?, ?)""",
            (number, repo, c["author"], c["body"], c.get("created_at", "")),
        )

    db.commit()


def ingest_repo(db, repo_key, full_repo, incremental=False, limit=None):
    print(f"\n{'='*60}")
    print(f"Ingesting: {full_repo}")
    print(f"{'='*60}")

    stored = get_stored_pr_numbers(db, repo_key)
    print(f"Already stored: {len(stored)} PRs")

    since = None
    if incremental:
        since = get_latest_merged_at(db, repo_key)
        if since:
            print(f"Incremental mode: fetching PRs merged after {since}")
        else:
            print("No prior data found, doing full ingestion")

    print("Fetching PR list...")
    pr_list = fetch_pr_list(full_repo, since=since, limit=limit)
    print(f"Found {len(pr_list)} merged PRs")

    new_prs = [p for p in pr_list if p["number"] not in stored]
    print(f"New PRs to ingest: {len(new_prs)}")

    for i, pr_meta in enumerate(new_prs):
        number = pr_meta["number"]
        title = pr_meta.get("title", "")
        print(f"  [{i+1}/{len(new_prs)}] PR #{number}: {title[:60]}...")

        db.execute("DELETE FROM reviews WHERE pr_number = ? AND repo = ?", (number, repo_key))
        db.execute("DELETE FROM review_comments WHERE pr_number = ? AND repo = ?", (number, repo_key))
        db.execute("DELETE FROM issue_comments WHERE pr_number = ? AND repo = ?", (number, repo_key))

        details = fetch_pr_details(full_repo, number)
        store_pr(db, repo_key, pr_meta, details)

        if (i + 1) % 50 == 0:
            print(f"  ... pausing 10s for rate limiting (processed {i+1}/{len(new_prs)})")
            time.sleep(10)
        else:
            time.sleep(0.5)

    return len(new_prs)


def main():
    parser = argparse.ArgumentParser(description="Ingest GitHub PRs into knowledge base")
    parser.add_argument("--incremental", action="store_true",
                        help="Only fetch PRs merged after the latest stored date")
    parser.add_argument("--repo", choices=["slang", "slangpy"],
                        help="Only ingest from one repo")
    parser.add_argument("--limit", type=int, default=None,
                        help="Max number of PRs to fetch per repo (for testing)")
    args = parser.parse_args()

    result = subprocess.run(["gh", "auth", "status"], capture_output=True, text=True)
    if result.returncode != 0:
        print("ERROR: gh CLI is not authenticated. Run: gh auth login", file=sys.stderr)
        sys.exit(1)

    db = init_db()
    total_new = 0

    repos_to_ingest = {args.repo: REPOS[args.repo]} if args.repo else REPOS
    for repo_key, full_repo in repos_to_ingest.items():
        count = ingest_repo(db, repo_key, full_repo,
                            incremental=args.incremental, limit=args.limit)
        total_new += count

    if total_new > 0:
        rebuild_fts(db)

    db.close()

    print(f"\n{'='*60}")
    print(f"Done! Ingested {total_new} new PRs total.")
    print(f"Database: {DB_PATH}")
    print(f"{'='*60}")


if __name__ == "__main__":
    main()
