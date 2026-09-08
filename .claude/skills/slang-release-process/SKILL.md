---
name: slang-release-process
description: Push a new Slang release. Triggers CI, determines the version from the sprint board, generates release notes, creates an annotated tag, and pushes it to upstream.
allowed-tools:
  - Bash
  - Read
  - Grep
  - Glob
---

# Slang Release Process

## Prerequisites

- **GitHub CLI (`gh`)**: Install from <https://cli.github.com>
- **Token scopes**: The `gh` token must include `read:project` (for querying sprint info from the project board). Run `gh auth refresh --scopes read:project` to add it.

## Step 1: Trigger Release CI

Fetch `upstream/master` and inspect the user-skills gitlink before starting the preflight:

```bash
git fetch upstream master
git ls-tree upstream/master external/slang-user-skills
```

Confirm that the full commit SHA is the reviewed `slang-user-skills` revision intended for this Slang release.

Manually trigger the Release workflow on the `master` branch from:
<https://github.com/shader-slang/slang/actions/workflows/release.yml>

Using GitHub CLI:

```bash
gh workflow run release.yml --ref master
```

The workflow takes approximately 30 minutes. Monitor it with:

```bash
gh run list --workflow=release.yml --limit 1 --json status,conclusion,databaseId
```

Wait until the run completes successfully before proceeding.
The release configuration requires the exact `external/slang-user-skills` commit recorded by `master`, and every binary-package job verifies the bundled files and provenance before upload.
Confirm that the `Verify bundled user skills` steps passed; a missing, modified, or mismatched skills checkout is a release blocker.

## Step 2: Determine the Version

Tag nomenclature: `vYYYY.N` where YYYY is the year and N is the sprint number within that year.

- Find the current sprint number from the project board: <https://github.com/orgs/shader-slang/projects/10/views/23>
- Or query it programmatically (requires `read:project` scope):

```bash
gh api graphql -f query='{
  organization(login: "shader-slang") {
    projectV2(number: 10) {
      fields(first: 20) {
        nodes {
          ... on ProjectV2IterationField {
            name
            configuration {
              iterations { title startDate duration }
              completedIterations { title startDate duration }
            }
          }
        }
      }
    }
  }
}'
```

- The first active iteration whose date range covers today is the current sprint.
- Subtract the starting sprint of the year to get N. The first sprint of the year is the earliest one with a startDate in January of that year.
- For 2026, sprint 45 was the first sprint (Jan 06), so `N = current_sprint - 45`.
- Example: sprint 52 → `52 - 45 = 7` → version `v2026.7`

Hotfix nomenclature: `vYYYY.N.H` where H increments from 1. The next sprint release note continues from the latest hotfix, not the previous sprint release.

Respin nomenclature (rare): `vYYYY.N.H.R` where R is the respin number, for re-spinning an old release with minimal changes.

## Step 3: Generate Release Notes

```bash
# Auto-detects previous release tag and generates notes
# Requires GitHub CLI (gh) for PR label lookup; takes ~1-2 minutes
bash docs/scripts/release-note.sh
```

The script prints breaking changes (PRs labeled "pr: breaking change") followed by all changes.

## Step 4: Create and Push the Tag

Create an annotated tag on `upstream/master` with the release notes as the message:

```bash
# Make sure local master is up to date
git fetch upstream && git checkout master && git merge --ff-only upstream/master

# Create the annotated tag (paste the release-note.sh output as the message body)
git tag -a vYYYY.N -m "<tag message with release notes>"

# Push to upstream — this triggers the Release CI which creates the release packages
git push upstream vYYYY.N
```

The tag message format:

```
vYYYY.N

=== Breaking changes ===
<one-line git log entries for breaking PRs>

=== All changes for this release ===
<one-line git log entries for all PRs, with [BREAKING] prefix on breaking ones>
```

## Interactive Workflow

1. Check prerequisites: verify `gh` is installed and has `read:project` scope (`gh auth status`)
2. Inspect the `external/slang-user-skills` gitlink on `upstream/master` and confirm it is the intended revision
3. Trigger the Release CI on master (`gh workflow run release.yml --ref master`)
4. Monitor the CI run until it passes (~30 minutes), including every `Verify bundled user skills` step
5. Confirm the verification logs report the intended skills commit for `share/slang/agent-skills/PROVENANCE.json`
6. Query the project board to determine the current sprint and compute the version number
7. Run `docs/scripts/release-note.sh` to generate release notes
8. Create the annotated tag on `upstream/master` with the release notes
9. Push the tag to `upstream` to trigger release packaging
10. Verify the release CI was triggered for the new tag
