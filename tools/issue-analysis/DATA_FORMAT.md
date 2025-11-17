# Data Format Reference

This document describes the JSON data structure returned by GitHub API and stored locally.

## Basic Fetch (Default)

### issues.json

Array of issue objects with standard GitHub fields:

```json
[
  {
    "number": 9030,
    "title": "Implement cooperative-matrix-2 support",
    "state": "open",
    "created_at": "2025-11-17T12:34:56Z",
    "updated_at": "2025-11-17T14:20:00Z",
    "closed_at": null,
    "user": {
      "login": "username",
      "id": 12345,
      "avatar_url": "..."
    },
    "labels": [
      {
        "name": "Bug",
        "color": "d73a4a",
        "description": "Something isn't working"
      }
    ],
    "assignees": [...],
    "milestone": {...},
    "comments": 5,
    "body": "Full issue description in markdown...",
    "html_url": "https://github.com/shader-slang/slang/issues/9030"
  }
]
```

### pull_requests.json

Array of PR objects (similar to issues but with PR-specific fields):

```json
[
  {
    "number": 8999,
    "title": "Fix SPIRV emission bug",
    "state": "closed",
    "created_at": "2025-11-10T10:00:00Z",
    "merged_at": "2025-11-11T15:30:00Z",
    "user": {...},
    "labels": [...],
    "pull_request": {
      "url": "...",
      "html_url": "...",
      "diff_url": "...",
      "patch_url": "..."
    },
    "changed_files": 5,
    "additions": 120,
    "deletions": 45,
    "body": "Description of changes..."
  }
]
```

## With --enrich

Issues gain a `related_prs` field:

```json
{
  "number": 9030,
  "title": "Some bug",
  "state": "closed",
  "related_prs": [8999, 9001],  // ← Added
  "...": "other fields..."
}
```

**How it's populated:**
- Fetches timeline events for each issue
- Finds "closed" events that reference a PR
- Extracts PR numbers that closed the issue

**Limitations:**
- Only captures PRs that explicitly closed the issue via GitHub's auto-close
- May miss PRs that partially fixed the issue
- Won't catch issues closed manually without PR reference

## With --pr-files

PRs gain a `files_changed` field with detailed file information:

```json
{
  "number": 8999,
  "title": "Fix SPIRV emission bug",
  "files_changed": [  // ← Added
    {
      "filename": "source/slang/slang-emit-spirv.cpp",
      "status": "modified",
      "additions": 25,
      "deletions": 10,
      "changes": 35
    },
    {
      "filename": "tests/spirv/new-test.slang",
      "status": "added",
      "additions": 50,
      "deletions": 0,
      "changes": 50
    },
    {
      "filename": "docs/old-doc.md",
      "status": "removed",
      "additions": 0,
      "deletions": 100,
      "changes": 100
    },
    {
      "filename": "tools/old-name.cpp",
      "status": "renamed",
      "previous_filename": "tools/new-name.cpp",
      "additions": 5,
      "deletions": 5,
      "changes": 10
    }
  ],
  "...": "other fields..."
}
```

**File status values:**
- `"modified"` - File was changed
- `"added"` - New file
- `"removed"` - File deleted
- `"renamed"` - File renamed (includes `previous_filename`)
- `"changed"` - Generic change

**Metrics per file:**
- `additions` - Lines added
- `deletions` - Lines removed
- `changes` - Total lines changed (additions + deletions)

## With --full

Both enrichments applied:
- Issues have `related_prs`
- PRs have `files_changed`

This enables full traceability: **Issue → PR → Files**

## metadata.json

```json
{
  "fetched_at": "2025-11-17T15:30:00.123456",
  "repo": "shader-slang/slang",
  "issue_count": 1847,
  "pr_count": 234
}
```

## Usage Examples

### Find which files are involved in bug fixes

```python
import json

# Load data
with open('data/issues.json') as f:
    issues = json.load(f)
with open('data/pull_requests.json') as f:
    prs = json.load(f)

# Build PR lookup
pr_by_number = {pr['number']: pr for pr in prs}

# Find bug-related files
bug_files = {}
for issue in issues:
    # Check if it's a bug
    is_bug = any(label['name'].lower() == 'bug' for label in issue.get('labels', []))
    if not is_bug:
        continue

    # Get related PRs
    for pr_num in issue.get('related_prs', []):
        pr = pr_by_number.get(pr_num)
        if not pr or 'files_changed' not in pr:
            continue

        # Count files
        for file_info in pr['files_changed']:
            filename = file_info['filename']
            bug_files[filename] = bug_files.get(filename, 0) + 1

# Top 20 files with most bug fixes
top_files = sorted(bug_files.items(), key=lambda x: x[1], reverse=True)[:20]
for filename, count in top_files:
    print(f"{count:3} bugs: {filename}")
```

### Analyze IR passes vs emitters

```python
ir_pass_bugs = 0
emitter_bugs = 0

for issue in issues:
    is_bug = any(label['name'].lower() == 'bug' for label in issue.get('labels', []))
    if not is_bug:
        continue

    for pr_num in issue.get('related_prs', []):
        pr = pr_by_number.get(pr_num)
        if not pr or 'files_changed' not in pr:
            continue

        for file_info in pr['files_changed']:
            filename = file_info['filename']
            if 'slang-ir-' in filename:
                ir_pass_bugs += 1
            elif 'slang-emit-' in filename:
                emitter_bugs += 1

print(f"IR pass bugs: {ir_pass_bugs}")
print(f"Emitter bugs: {emitter_bugs}")
```

## API Rate Limits

- **Unauthenticated**: 60 requests/hour
- **Authenticated** (with token): 5000 requests/hour

Fetch modes and approximate API calls:
- Basic: ~40 calls (just paginated issue/PR fetches)
- `--enrich`: +2000 calls (timeline for each issue)
- `--pr-files`: +250 calls (files for each PR)
- `--full`: +2250 calls total

**Recommendation**: Always use `GITHUB_TOKEN` for any enriched fetches!

