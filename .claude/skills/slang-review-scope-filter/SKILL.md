---
name: slang-review-scope-filter
description: Filter candidate review comments in place for PR-author ownership. Use after slang-review-consolidate-candidates, slang-review-clarity, or slang-review-fine-grained-clarity to conservatively keep only comments about code the PR author is justifiably responsible for addressing. Updates candidate status metadata in the markdown file.
argument-hint: "<candidate-markdown> <pr-number-or-diff-path>"
allowed-tools:
  - Bash
  - Read
  - Grep
  - Glob
  - Write
  - Edit
---

# Slang Review Scope Filter

Filter candidate review feedback/comments so that posted feedback is fair to the PR author.
This skill is intentionally stricter than the skills that generate candidates.
You should drop comments for which the author could justifiably answer: "that is out of scope for this PR," even if they agreed with the underlying concern.

Do not post comments.
Update the candidate markdown file in place. Do not create a separate filtered file unless the
user explicitly asks for one.
Preserve dropped candidates in the file so a human can audit the decision.

## Inputs

Read:

- One candidate markdown file produced by `slang-review-consolidate-candidates`,
  `slang-review-clarity`, or `slang-review-fine-grained-clarity`.
- `tmp/pr-diff.patch` and `tmp/pr-files.txt`, or the equivalent PR diff and file list.
- The PR-version source around candidate locations.
- The base-version source when needed to decide whether a concern predates the PR.

## In-Place Updates

Preserve the original candidate IDs. Add or update:

```md
- Status: Keep | Revise | Needs judgment call | Drop
- Scope decision: Direct | Contextual | Out-of-scope
- Scope rationale: <one sentence>
```

For `Drop` candidates, keep the title, proposed comment, and scope rationale in the file so a
human can audit the decision. Prefer moving dropped candidates under a later `## Dropped`
section within the same file so they do not interrupt review of the postable candidates.

If a previous consolidation pass already set `Status: Drop` for a duplicate, superseded, or
merged candidate, preserve that decision unless it is clearly wrong. Do not revive dropped
overlap candidates during scope filtering.

## Keep Rules

Keep or revise a candidate when at least one condition holds:

- It is anchored on a line added or modified by the PR.
- It concerns a new type, function, field, enum case, comment, test, or document introduced by
  the PR.
- It concerns a declaration whose signature changed in the PR.
- It concerns a declaration whose body changed in the PR, in a way that affects its behavior, usage, or semantic contract.
- It concerns front matter, overview comments, or documentation that the PR made stale or
  misleading.
- It concerns pre-existing code where the PR changed that code's de facto contract,
  visibility, invariants, or semantic role.
- It asks for explanation of a design choice, invariant, or terminology introduced by the PR.
- It identifies a code/comment/name mismatch created or exposed by changed lines.
- The candidate pertains to a declaration that was not introduced by the PR, but the PR touches an overwhelming majority of the lines of code that use that declaration, such that folding a cleanup or fix into the PR will not drastically increase its scope.

Revise rather than drop when the core issue is in scope but the comment text directs blame at the wrong location in code, or asks for a cleanup that is overly broad when a more focused change would suffice.

## Drop Rules

Drop or mark `Needs judgment call` when:

- The concern is a pre-existing naming, comment, or design problem not made worse by the PR.
  E.g., if a PR adds a field to a struct that already had a bad name, it is inappropriate to ask the PR author to fix it.
- The concern would require a broad refactor of unrelated code to address, and potentially lead to a cascade of additional concerns/requests.
- The candidate is focused on a piece of code the PR only calls, reads, or moves through, without
  changing its contract.
  E.g., adding a few `case`s to handle a new instruction in a pre-existing pass with structural issues does not put the PR author on the hook to fix those issues.
- The candidate is redundant with another candidate that is more narrowly scoped and actionable.
  Prefer to ask for local and specific changes over broad ones unless the clarity issue is truly large in scope.

When uncertain, prefer `Needs judgment call` over `Keep`.

## Filtering Is Not Bug Triage

Do not apply the normal `REVIEW.md` bug/gap/question confidence filter here. A clarity comment
can be worth posting even when it does not prove a concrete bug. The scope filter asks only
whether the PR author can reasonably be asked to own and address the clarity problem as part of their PR.
Do not drop a candidate merely because it is fine-grained, pedantic, or asks the author to do
additional explanatory work. Drop only for scope, duplication, supersession, unsupported
analysis, or a concrete lack of usefulness.

## Revision Guidelines

For kept comments:

- Keep the request centered on making the changed code clear and internally consistent.
- Lead with what the reader cannot currently understand or verify with confidence. Only keep
  prescriptive remedy text when there is only one credible way to resolve that uncertainty.
- Avoid saying the author must perform a non-local refactor unless there are no alternatives that could satisfy the concerns.
  Prefer to identify clarity issues that must be resolved without dictating a singular way to resolve them, and allow the author to exercise their judgment in how best to address concerns.
- When broad concerns are identified, frame them as requests for a clear contract, invariant, or explanation at the smallest useful scale.
- If a candidate points at code not directly touched by the PR, ensure that the comment explains why it is appropriate for the PR author to address it in their work.
  For example, point out how the PR changed the de facto contract of a pre-existing abstraction by virtue of changing utility code used in the implementation of that abstraction.
- Preserve source context code blocks, or revise them to show the correct lines if you have revised the candidate to direct its request at a different location.

For dropped comments:

- Record the shortest fair rationale, for example "pre-existing helper not touched by this PR"
  or "broad cleanup not required to understand changed lines."

For `Needs judgment call` comments:

- Clearly identify the source of uncertainty and the focused follow-up analysis needed to make
  the decision. Do not assume a human must decide; the judgment-call resolution pass should try
  to answer the question before posting.

## Suggested File Structure

```md
# Scope-Filtered Review Candidates

## Kept

### FG001: ...

...

## Needs Judgment Call

### FG007: ...

...

## Dropped

### FG014: ...

- Status: Drop
- Scope decision: Out-of-scope
- Scope rationale: ...
```
