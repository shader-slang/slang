---
name: slang-review-resolve-judgment-calls
description: Resolve Slang clarity review candidates marked as needing a judgment call. Use after consolidation and scope filtering, before posting, to perform focused follow-up analysis and decide whether uncertain candidates should be kept, revised, or dropped.
argument-hint: "<candidate-markdown> <pr-number-or-diff-path>"
allowed-tools:
  - Bash
  - Read
  - Grep
  - Glob
  - Write
  - Edit
---

# Slang Review Judgment-Call Resolution

Review candidates marked `Status: Needs judgment call` or with `Overlap decision: Needs
judgment call`. The goal is to resolve uncertainty through focused analysis, not to wait for a
human reviewer.

Do not post comments. Update the canonical candidate markdown file in place.

## Inputs

Read:

- The canonical candidate markdown file after consolidation and scope filtering.
- `tmp/pr-diff.patch` and `tmp/pr-files.txt`, or the equivalent PR diff and file list.
- PR-version and base-version source around the candidate location.
- Callers, callees, tests, comments, and related declarations needed to answer the specific
  uncertainty recorded in the candidate.

## Decisions

For each judgment-call candidate, spend focused effort to make a decision:

- `Status: Keep` when the concern is credible, in scope, and useful to post.
- `Status: Revise` when the concern is credible but the proposed comment needs a narrower
  location, clearer wording, or a different request.
- `Status: Drop` when the concern is unsupported, duplicate, superseded, out of scope, or too
  speculative after follow-up analysis.
- `Status: Needs judgment call` only when focused analysis still cannot resolve the question.

Update or add:

```md
- Judgment decision: Keep | Revise | Drop | Needs judgment call
- Judgment rationale: <one or two sentences describing the decisive evidence or remaining uncertainty>
```

If keeping, revising, or leaving a candidate as `Needs judgment call`, make sure it still has
postable metadata:

```md
- Scope decision: Direct | Contextual
- Scope rationale: <one sentence>
- Overlap decision: Keep
- Overlap rationale: <one sentence>
```

## Working Process

1. Inventory all candidates marked `Needs judgment call`.
2. For each one, read the recorded uncertainty and identify the smallest source context needed
   to answer it.
3. Follow local call graph, data-flow, type, declaration-reference, test, or documentation
   evidence far enough to decide whether the concern is credible and useful.
4. Revise the proposed comment when the concern is real but the wording overstates confidence
   or points at the wrong location.
5. Move dropped candidates to a later `## Dropped` section when practical, preserving enough
   rationale for auditability.
