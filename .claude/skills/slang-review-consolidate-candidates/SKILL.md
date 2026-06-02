---
name: slang-review-consolidate-candidates
description: Merge Slang clarity review candidate files and resolve overlapping, duplicate, or superseded comments. Use after high-level and fine-grained clarity review passes and before scope filtering or GitHub posting. Produces or updates one canonical candidate markdown file.
argument-hint: "<candidate-markdown> [more-candidate-markdown...]"
allowed-tools:
  - Bash
  - Read
  - Grep
  - Glob
  - Write
  - Edit
---

# Slang Review Candidate Consolidation

Merge clarity-review candidates into one canonical markdown file and resolve overlap before
scope filtering.

Prefer one canonical file after raw generation:

```text
tmp/review-candidates/pr-<number>-clarity-workflow.md
```

If given multiple raw candidate files, create or overwrite the canonical file by copying their
candidate entries in source order. If given one canonical file, update it in place.

Do not post comments.

## Inputs

Read:

- Candidate files produced by `slang-review-clarity` and
  `slang-review-fine-grained-clarity`.
- `tmp/pr-diff.patch` and `tmp/pr-files.txt`, or the equivalent PR diff and file list.
- PR-version source around overlapping candidate locations when needed.

## Candidate Metadata

Preserve candidate IDs such as `C001` and `FG014`. Add or update:

```md
- Overlap decision: Keep | Duplicate of <id> | Superseded by <id> | Merged into <id> | Needs judgment call
- Overlap rationale: <one sentence>
```

Every candidate must receive overlap metadata, including candidates kept without changes. Use
`Overlap decision: Keep` for candidates that do not overlap another candidate.

For duplicates, superseded candidates, and candidates merged into another comment, set or
update:

```md
- Status: Drop
```

When merging two candidates, revise the retained candidate's proposed comment so it covers the
best version of the concern. Do not include candidate IDs or process metadata in the proposed
comment text.

## Overlap Rules

Treat candidates as overlapping when they point at the same line range, nearby lines in the
same function/type/section, or the same conceptual clarity problem.

Drop a candidate as a duplicate when it asks for the same clarification at the same conceptual
level as another candidate. Prefer the clearer, more actionable, better-scoped comment.

Mark a fine-grained candidate as superseded when a high-level candidate asks for enough
restructuring, recasting, or contract explanation that the fine-grained code may no longer
exist after the high-level concern is addressed.

Merge candidates when each has part of the same concern and one combined comment would be
clearer than multiple separate comments.

Keep overlapping candidates separate when they identify genuinely different problems, such as
a high-level missing invariant and a separate local naming contradiction.

Use `Needs judgment call` when overlap depends on author intent, likely implementation
strategy, or whether a broader rewrite will happen. This does not mean the workflow must stop
for a human; it means the judgment-call resolution pass should spend focused analysis before
posting.

Prefer moving dropped, duplicate, superseded, and merged candidates to a later `## Dropped`
section in the same file. Preserve enough text for auditability and to avoid reintroducing a
previously rejected comment.

## Working Process

1. Inventory candidates by ID, location, function/type/section, and category.
2. Cluster nearby and conceptually related candidates.
3. For each cluster, decide whether candidates should be kept, dropped as duplicates,
   superseded, merged, or marked as needing a judgment call.
4. Update the canonical file in place with overlap metadata and any revised proposed comments.
5. Preserve enough dropped-candidate text for auditability.
