---
name: slang-pr-knowledge
description: Build a local MCP server that indexes all merged PRs from shader-slang/slang and shader-slang/slangpy — titles, descriptions, review comments, file changes, reviewer patterns. Provides full-text search tools for AI-assisted development.
argument-hint: "[--incremental] [--repo slang|slangpy] [--limit N]"
allowed-tools:
  - Bash
  - Read
  - Write
  - Edit
  - Grep
  - Glob
---

# PR Knowledge Base — MCP Server Setup

Build a local MCP server that indexes merged PRs from shader-slang/slang and
shader-slang/slangpy into a searchable SQLite database. Once set up, AI agents
can query past PRs, review comments, file change history, and reviewer patterns.

**Prerequisites:**

- Python 3.10+
- `gh` CLI authenticated (`gh auth status`)
- ~2 GB disk for the full database

## Step 1: Create Virtual Environment

```bash
cd .claude/skills/slang-pr-knowledge
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## Step 2: Ingest PR Data

Full ingestion fetches all merged PRs (takes 2-4 hours for initial run due to
GitHub API rate limits). Subsequent runs skip already-stored PRs.

```bash
source .venv/bin/activate
python3 ingest.py
```

Options:

- `--incremental` — Only fetch PRs merged after the latest stored date (fast)
- `--repo slang` or `--repo slangpy` — Only ingest from one repo
- `--limit 50` — Limit number of PRs (for testing)

For a quick test run:

```bash
python3 ingest.py --repo slang --limit 50
```

## Step 3: Verify

```bash
python3 server.py --test
```

This prints database statistics (PR count, review count, date range).

## Step 4: Register MCP Server

### For Cursor

Add to your Cursor MCP settings (`.cursor/mcp.json` in your workspace or
`~/.cursor/mcp.json` globally):

```json
{
  "mcpServers": {
    "slang-pr-knowledge": {
      "command": "<repo-root>/.claude/skills/slang-pr-knowledge/.venv/bin/python3",
      "args": ["<repo-root>/.claude/skills/slang-pr-knowledge/server.py"]
    }
  }
}
```

Replace `<repo-root>` with the absolute path to your slang repository.

### For Claude Code CLI

Run this from the repository root:

```bash
claude mcp add slang-pr-knowledge \
  .claude/skills/slang-pr-knowledge/.venv/bin/python3 \
  .claude/skills/slang-pr-knowledge/server.py
```

## Available MCP Tools

Once registered, the following tools are available to AI agents:

| Tool                  | Description                                                               |
| --------------------- | ------------------------------------------------------------------------- |
| `search_prs`          | Full-text search across PR titles, descriptions, and review comments      |
| `get_pr`              | Get full details of a specific PR (description, files, reviews, comments) |
| `search_reviews`      | Search review comments by content, repo, or reviewer                      |
| `search_files`        | Find PRs that touched specific files                                      |
| `list_prs_by_author`  | List PRs by a specific GitHub author                                      |
| `get_review_patterns` | Find common reviewer feedback patterns                                    |
| `get_stats`           | Show database statistics                                                  |

## Keeping Up to Date

Run incremental ingestion periodically to pick up new PRs:

```bash
cd .claude/skills/slang-pr-knowledge
source .venv/bin/activate
python3 ingest.py --incremental
```

Or set up a cron job:

```bash
0 */12 * * * cd /path/to/slang/.claude/skills/slang-pr-knowledge && .venv/bin/python3 ingest.py --incremental
```

## Interactive Workflow

If `$ARGUMENTS` contains flags, pass them to `ingest.py`. Otherwise:

1. Check if `.venv/` exists; if not, create it (Step 1)
2. Check if `pr-knowledge.db` exists; if not, run initial ingestion (Step 2)
3. If `--incremental` is passed, run incremental update
4. Verify the database (Step 3)
5. Show the user how to register the MCP server (Step 4)
