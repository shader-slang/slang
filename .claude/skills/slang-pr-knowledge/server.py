#!/usr/bin/env python3
"""
MCP server for the Slang/SlangPy PR Knowledge Base.

Provides full-text search and retrieval tools over merged PR data stored
in SQLite. Runs as a stdio-based MCP server, spawned on demand by
Claude Code or Cursor.

Usage:
    python3 server.py              # Start MCP server (stdio transport)
    python3 server.py --test       # Run a quick self-test
"""

import json
import sqlite3
import sys
from pathlib import Path
from typing import Optional

from mcp.server.fastmcp import FastMCP

DB_PATH = Path(__file__).parent / "pr-knowledge.db"

mcp = FastMCP("slang-pr-knowledge")


def get_db():
    if not DB_PATH.exists():
        return None
    db = sqlite3.connect(str(DB_PATH))
    db.row_factory = sqlite3.Row
    return db


@mcp.tool()
def search_prs(query: str, repo: Optional[str] = None, limit: int = 10) -> str:
    """
    Full-text search across PR titles, descriptions, and review comments.
    Returns matching PRs ranked by relevance.

    Args:
        query: Search terms (e.g. "constraint ordering", "Optional DifferentialPair crash")
        repo: Filter by repo - "slang" or "slangpy". Omit for both.
        limit: Max results to return (default 10)
    """
    db = get_db()
    if not db:
        return "Error: Database not found. Run ingest.py first."

    results = []
    sql = """
        SELECT p.number, p.repo, p.title, p.author, p.merged_at, p.labels,
               snippet(prs_fts, 3, '>>>', '<<<', '...', 40) as snippet
        FROM prs_fts
        JOIN prs p ON p.rowid = prs_fts.rowid
        WHERE prs_fts MATCH ?
    """
    params = [query]
    if repo:
        sql += " AND p.repo = ?"
        params.append(repo)
    sql += f" ORDER BY rank LIMIT {limit}"

    try:
        for row in db.execute(sql, params):
            results.append({
                "number": row["number"],
                "repo": row["repo"],
                "title": row["title"],
                "author": row["author"],
                "merged_at": row["merged_at"],
                "labels": row["labels"],
                "snippet": row["snippet"],
                "url": f"https://github.com/shader-slang/{row['repo']}/pull/{row['number']}",
            })
    except sqlite3.OperationalError as e:
        if "no such table" not in str(e):
            raise

    review_hits = []
    sql2 = """
        SELECT rc.pr_number, rc.repo, rc.reviewer, rc.file_path, rc.line,
               snippet(review_comments_fts, 4, '>>>', '<<<', '...', 40) as snippet
        FROM review_comments_fts rc_fts
        JOIN review_comments rc ON rc.id = rc_fts.rowid
        WHERE review_comments_fts MATCH ?
    """
    params2 = [query]
    if repo:
        sql2 += " AND rc.repo = ?"
        params2.append(repo)
    sql2 += f" ORDER BY rank LIMIT {limit}"

    try:
        for row in db.execute(sql2, params2):
            review_hits.append({
                "pr_number": row["pr_number"],
                "repo": row["repo"],
                "reviewer": row["reviewer"],
                "file": f"{row['file_path']}:{row['line']}",
                "snippet": row["snippet"],
            })
    except sqlite3.OperationalError as e:
        if "no such table" not in str(e):
            raise

    db.close()

    output = []
    if results:
        output.append(f"## PR Matches ({len(results)})\n")
        for r in results:
            output.append(
                f"- **PR #{r['number']}** ({r['repo']}): {r['title']}\n"
                f"  Author: {r['author']} | Merged: {r['merged_at']} | Labels: {r['labels']}\n"
                f"  {r['snippet']}\n"
                f"  {r['url']}\n"
            )
    if review_hits:
        output.append(f"\n## Review Comment Matches ({len(review_hits)})\n")
        for r in review_hits:
            output.append(
                f"- **PR #{r['pr_number']}** ({r['repo']}) — {r['reviewer']} on `{r['file']}`\n"
                f"  {r['snippet']}\n"
            )
    if not output:
        return f"No results found for: {query}"
    return "\n".join(output)


@mcp.tool()
def get_pr(number: int, repo: str = "slang") -> str:
    """
    Get full details of a specific PR including description, files, reviews,
    and comments.

    Args:
        number: PR number
        repo: "slang" or "slangpy" (default "slang")
    """
    db = get_db()
    if not db:
        return "Error: Database not found. Run ingest.py first."

    row = db.execute(
        "SELECT * FROM prs WHERE number = ? AND repo = ?", (number, repo)
    ).fetchone()

    if not row:
        db.close()
        return f"PR #{number} not found in {repo}."

    output = [
        f"# PR #{row['number']}: {row['title']}\n",
        f"**Repo**: {repo} | **Author**: {row['author']} | **Merged**: {row['merged_at']}\n",
        f"**Labels**: {row['labels']}\n",
        f"\n## Description\n\n{row['body'][:5000]}\n",
    ]

    files = json.loads(row["files_json"]) if row["files_json"] else []
    if files:
        output.append(f"\n## Files Changed ({len(files)})\n")
        for f in files[:20]:
            output.append(f"- `{f['filename']}`")

    reviews = db.execute(
        "SELECT * FROM reviews WHERE pr_number = ? AND repo = ?", (number, repo)
    ).fetchall()
    if reviews:
        output.append(f"\n## Reviews ({len(reviews)})\n")
        for r in reviews:
            output.append(f"### {r['reviewer']} ({r['state']})\n{r['body'][:1000]}\n")

    comments = db.execute(
        "SELECT * FROM review_comments WHERE pr_number = ? AND repo = ?", (number, repo)
    ).fetchall()
    if comments:
        output.append(f"\n## Review Comments ({len(comments)})\n")
        for c in comments:
            output.append(f"### {c['reviewer']} on `{c['file_path']}:{c['line']}`\n{c['body'][:500]}\n")

    db.close()
    return "\n".join(output)


@mcp.tool()
def search_reviews(query: str, repo: Optional[str] = None,
                   reviewer: Optional[str] = None, limit: int = 15) -> str:
    """
    Search review comments specifically. Useful for finding reviewer feedback
    patterns.

    Args:
        query: Search terms for review content
        repo: Filter by repo - "slang" or "slangpy"
        reviewer: Filter by reviewer GitHub username
        limit: Max results (default 15)
    """
    db = get_db()
    if not db:
        return "Error: Database not found. Run ingest.py first."

    results = []
    sql = """
        SELECT rc.pr_number, rc.repo, rc.reviewer, rc.file_path, rc.line, rc.body, rc.created_at
        FROM review_comments_fts fts
        JOIN review_comments rc ON rc.id = fts.rowid
        WHERE review_comments_fts MATCH ?
    """
    params = [query]
    if repo:
        sql += " AND rc.repo = ?"
        params.append(repo)
    if reviewer:
        sql += " AND rc.reviewer = ?"
        params.append(reviewer)
    sql += f" ORDER BY rank LIMIT {limit}"

    try:
        for row in db.execute(sql, params):
            results.append({
                "pr_number": row["pr_number"],
                "repo": row["repo"],
                "reviewer": row["reviewer"],
                "file_path": row["file_path"],
                "line": row["line"],
                "body": row["body"][:500],
                "created_at": row["created_at"],
            })
    except sqlite3.OperationalError as e:
        if "no such table" not in str(e):
            raise

    review_bodies = []
    sql2 = """
        SELECT r.pr_number, r.repo, r.reviewer, r.body, r.state, r.submitted_at
        FROM reviews_fts fts
        JOIN reviews r ON r.id = fts.rowid
        WHERE reviews_fts MATCH ?
    """
    params2 = [query]
    if repo:
        sql2 += " AND r.repo = ?"
        params2.append(repo)
    if reviewer:
        sql2 += " AND r.reviewer = ?"
        params2.append(reviewer)
    sql2 += f" ORDER BY rank LIMIT {limit}"

    try:
        for row in db.execute(sql2, params2):
            review_bodies.append({
                "pr_number": row["pr_number"],
                "repo": row["repo"],
                "reviewer": row["reviewer"],
                "body": row["body"][:500],
                "state": row["state"],
            })
    except sqlite3.OperationalError as e:
        if "no such table" not in str(e):
            raise

    db.close()

    output = []
    if results:
        output.append(f"## Inline Review Comments ({len(results)})\n")
        for r in results:
            output.append(
                f"- **PR #{r['pr_number']}** ({r['repo']}) — {r['reviewer']} on "
                f"`{r['file_path']}:{r['line']}`\n"
                f"  {r['body']}\n"
            )
    if review_bodies:
        output.append(f"\n## Review Summaries ({len(review_bodies)})\n")
        for r in review_bodies:
            output.append(
                f"- **PR #{r['pr_number']}** ({r['repo']}) — {r['reviewer']} ({r['state']})\n"
                f"  {r['body']}\n"
            )
    if not output:
        return f"No review comments found for: {query}"
    return "\n".join(output)


@mcp.tool()
def search_files(file_path: str, repo: Optional[str] = None, limit: int = 20) -> str:
    """
    Find PRs that touched specific files. Useful for understanding change
    history of a file.

    Args:
        file_path: File path or partial path to search (e.g. "slang-ir-lower-optional-type.cpp")
        repo: Filter by repo - "slang" or "slangpy"
        limit: Max results (default 20)
    """
    db = get_db()
    if not db:
        return "Error: Database not found. Run ingest.py first."

    sql = "SELECT number, repo, title, author, merged_at, files_json FROM prs WHERE 1=1"
    params = []
    if repo:
        sql += " AND repo = ?"
        params.append(repo)
    sql += " ORDER BY merged_at DESC LIMIT 2000"

    results = []
    for row in db.execute(sql, params):
        files = json.loads(row["files_json"]) if row["files_json"] else []
        matching_files = [f["filename"] for f in files if file_path.lower() in f["filename"].lower()]
        if matching_files:
            results.append({
                "number": row["number"],
                "repo": row["repo"],
                "title": row["title"],
                "author": row["author"],
                "merged_at": row["merged_at"],
                "matching_files": matching_files,
            })
            if len(results) >= limit:
                break

    db.close()

    if not results:
        return f"No PRs found that touched files matching: {file_path}"

    output = [f"## PRs touching `{file_path}` ({len(results)} found)\n"]
    for r in results:
        files_str = ", ".join(f"`{f}`" for f in r["matching_files"][:5])
        output.append(
            f"- **PR #{r['number']}** ({r['repo']}): {r['title']}\n"
            f"  Author: {r['author']} | Merged: {r['merged_at']}\n"
            f"  Files: {files_str}\n"
        )
    return "\n".join(output)


@mcp.tool()
def list_prs_by_author(author: str, repo: Optional[str] = None, limit: int = 30) -> str:
    """
    List PRs by a specific author.

    Args:
        author: GitHub username
        repo: Filter by repo - "slang" or "slangpy"
        limit: Max results (default 30)
    """
    db = get_db()
    if not db:
        return "Error: Database not found. Run ingest.py first."

    sql = "SELECT number, repo, title, merged_at, labels FROM prs WHERE author = ?"
    params = [author]
    if repo:
        sql += " AND repo = ?"
        params.append(repo)
    sql += f" ORDER BY merged_at DESC LIMIT {limit}"

    results = db.execute(sql, params).fetchall()
    db.close()

    if not results:
        return f"No PRs found by author: {author}"

    output = [f"## PRs by {author} ({len(results)} shown)\n"]
    for r in results:
        output.append(
            f"- **PR #{r['number']}** ({r['repo']}): {r['title']}\n"
            f"  Merged: {r['merged_at']} | Labels: {r['labels']}\n"
        )
    return "\n".join(output)


@mcp.tool()
def get_review_patterns(reviewer: Optional[str] = None,
                        keyword: Optional[str] = None, limit: int = 20) -> str:
    """
    Find common reviewer feedback patterns. Shows what reviewers frequently
    comment on.

    Args:
        reviewer: Filter by reviewer username (optional)
        keyword: Filter by keyword in comments (optional)
        limit: Max results (default 20)
    """
    db = get_db()
    if not db:
        return "Error: Database not found. Run ingest.py first."

    output = []

    if reviewer:
        sql = """
            SELECT pr_number, repo, file_path, line, body, created_at
            FROM review_comments WHERE reviewer = ?
        """
        params = [reviewer]
        if keyword:
            sql += " AND body LIKE ?"
            params.append(f"%{keyword}%")
        sql += f" ORDER BY created_at DESC LIMIT {limit}"

        comments = db.execute(sql, params).fetchall()
        output.append(f"## Review comments by {reviewer} ({len(comments)} shown)\n")
        for c in comments:
            output.append(
                f"- **PR #{c['pr_number']}** ({c['repo']}) — `{c['file_path']}:{c['line']}`\n"
                f"  {c['body'][:300]}\n"
            )
    else:
        sql = ("SELECT reviewer, COUNT(*) as cnt FROM review_comments "
               "GROUP BY reviewer ORDER BY cnt DESC LIMIT 15")
        output.append("## Top Reviewers by Comment Count\n")
        for row in db.execute(sql):
            output.append(f"- **{row['reviewer']}**: {row['cnt']} comments")

        if keyword:
            sql2 = f"""
                SELECT rc.pr_number, rc.repo, rc.reviewer, rc.file_path, rc.body
                FROM review_comments rc WHERE rc.body LIKE ?
                ORDER BY rc.created_at DESC LIMIT {limit}
            """
            comments = db.execute(sql2, [f"%{keyword}%"]).fetchall()
            output.append(f"\n## Comments containing '{keyword}' ({len(comments)} shown)\n")
            for c in comments:
                output.append(
                    f"- **PR #{c['pr_number']}** ({c['repo']}) — {c['reviewer']} on `{c['file_path']}`\n"
                    f"  {c['body'][:300]}\n"
                )

    db.close()
    return "\n".join(output) if output else "No review patterns found."


@mcp.tool()
def get_stats() -> str:
    """Get statistics about the PR knowledge base."""
    db = get_db()
    if not db:
        return "Error: Database not found. Run ingest.py first."

    stats = {}
    stats["total_prs"] = db.execute("SELECT COUNT(*) FROM prs").fetchone()[0]
    stats["slang_prs"] = db.execute("SELECT COUNT(*) FROM prs WHERE repo='slang'").fetchone()[0]
    stats["slangpy_prs"] = db.execute("SELECT COUNT(*) FROM prs WHERE repo='slangpy'").fetchone()[0]
    stats["total_reviews"] = db.execute("SELECT COUNT(*) FROM reviews").fetchone()[0]
    stats["total_review_comments"] = db.execute("SELECT COUNT(*) FROM review_comments").fetchone()[0]
    stats["total_issue_comments"] = db.execute("SELECT COUNT(*) FROM issue_comments").fetchone()[0]

    latest = db.execute("SELECT MAX(merged_at) FROM prs").fetchone()[0]
    earliest = db.execute(
        "SELECT MIN(merged_at) FROM prs WHERE merged_at IS NOT NULL AND merged_at != ''"
    ).fetchone()[0]

    db.close()

    return (
        f"## PR Knowledge Base Stats\n\n"
        f"| Metric | Count |\n"
        f"|--------|-------|\n"
        f"| Total PRs | {stats['total_prs']} |\n"
        f"| Slang PRs | {stats['slang_prs']} |\n"
        f"| SlangPy PRs | {stats['slangpy_prs']} |\n"
        f"| Reviews | {stats['total_reviews']} |\n"
        f"| Review Comments | {stats['total_review_comments']} |\n"
        f"| Discussion Comments | {stats['total_issue_comments']} |\n"
        f"| Date Range | {earliest} to {latest} |\n"
    )


if __name__ == "__main__":
    if "--test" in sys.argv:
        print("Running self-test...")
        db = get_db()
        if db:
            count = db.execute("SELECT COUNT(*) FROM prs").fetchone()[0]
            print(f"Database has {count} PRs")
            db.close()
            print(get_stats())
        else:
            print("No database found. Run ingest.py first.")
    else:
        mcp.run()
