---
name: slang-review-clarity
description: Review Slang changes for high-level clarity. Use whenever reviewing PRs or diffs for code quality or correctness. Produces candidate review comments in a markdown file.
argument-hint: "<pr-number-or-diff-path>"
allowed-tools:
  - Bash
  - Read
  - Grep
  - Glob
  - Write
  - Edit
---

# Slang Clarity Review

Generate high-level candidate comments to be used as review feedback, focused on the clarity and explainability of code.
This skill is intentionally broader than the normal bug-review protocol in `REVIEW.md`.
Code need not be demonstrably wrong to warrant a comment.
If changed code does not clearly communicate its intent and approach, or does not explain why it is correct, that is a valid candidate finding.

Do not directly post comments to PRs; write candidate comments to a markdown file under
`tmp/review-candidates/`.
If the caller provides an existing candidate file, append new candidates to that file instead
of creating a fresh file. This is useful for sequential workflows. When multiple review passes
may run concurrently, write separate raw files and let `slang-review-consolidate-candidates`
merge them.
When appending, inspect existing candidate IDs and continue numbering for this skill's `C`
prefix instead of restarting at `C001`.
If requested to post comments, write the candidates to a file, then run the consolidation and
filtering skills before posting the filtered results.

## Inputs

Prefer reviewing from a saved diff:

```bash
mkdir -p tmp/review-candidates
gh.exe pr diff <number> -R shader-slang/slang > tmp/pr-diff.patch
gh.exe pr view <number> -R shader-slang/slang --json files -q '.files[].path' > tmp/pr-files.txt
```

If the user provides a local branch or patch instead, save the equivalent diff and file list
under `tmp/` first.

Read:

- `CLAUDE.md` for Slang project context.
- `AGENTS.md` for cross-harness repository guidance.
- `tmp/pr-files.txt` for the changed file list.
- `tmp/pr-diff.patch` for the actual changes.
- The PR version of each changed source file, using grep first for large files.

## Output File

If the caller provides an output file, append candidates there.
Otherwise, write candidates to:

```text
tmp/review-candidates/pr-<number>-clarity.md
```

If no PR number is known, use:

```text
tmp/review-candidates/clarity-review.md
```

## Review Philosophy

Treat clarity issues as evidence of correctness risk. The desired standard is code that is
clear enough for a future maintainer to understand the problem, the chosen decomposition, the
important invariants, and why the implementation should be trusted.
"Obviously no bugs" rather than "no obvious bugs."

The review protects maintainer attention. PR authors are responsible for making changes easy
to review, not merely for producing code that might be correct after prolonged reconstruction.
Generate a candidate unless you can confidently say a careful reader would find the changed
code, comments, names, and tests obviously necessary and correct.

Strong clarity comments do not say "make change X." They say:

- the current code/comment/name combination is unclear or internally inconsistent;
- the author needs to make the model, contract, or invariant explicit;
- one possible direction for improvement is identifiable, but the essential request is clarity and consistency.

Proposed comments should lead with the understanding or certainty the reader currently lacks.
Only prescribe a specific remedy when there is only one credible way to resolve the confusion.
Otherwise, identify the missing clarity and let the PR author decide how best to supply it.

Code that is not commented should be held to the highest possible standard of clarity.
If it is not manifestly obvious why a line of code is both correct and necessary, there must be a comment that speaks to it.

## What To Look For

Prioritize high-level concerns in and around changed code:

- Missing file-level or section-level comments providing a problem statement, mental model, and overview of the solution strategy for new or substantially changed algorithms or data structures.
- Comments that narrate the sequence of what code is doing, without explaining the decomposition or approach that motivates and justifies that sequence.
- New terminology used before it is defined, or without any definition given.
- Inconsistency between the name, comments, and implementation of a declaration (type, function, etc.).
- APIs (including helper APIs) whose names, signatures, return values, or side effects do not match.
- Comments that describe intent or what "should" happen but do not specify a precise contract, invariant, or failure mode.
- Stateful algorithms (state machines, work lists, caches, or dependency logic) without stated invariants and termination or convergence conditions.
- Invariants that are stated or assumed, but not clearly justified in comments or validated via assertions or test cases.
- Improperly "defensive" code that uses early-outs to gloss over unexpected/corner cases rather than confidently asserting its invariants/expectations.
- Code that drops semantic context or provenance, such as conversion from a `DeclRef` to plain `Decl*`, without providing a solid proof of why it is valid to do so.
- Conditional branching logic where the validity of the conditions or case analysis is not clearly explained and fully justified.
- Comments that justify a piece of code by appealing to a corner or special case, but that do not provide a clear example of input that would trigger that case, nor a test that would fail if the case were not handled correctly.

When reviewing compiler code, pay special attention to consistent use of established Slang terminology. E.g.:

- "argument" (call-site) versus "parameter" (declaration site)
- "constraint" (on a generic parameter) versus "requirement" (of an interface) versus "witness" (of a conformance)
- "declaration" (unspecialized) versus "declaration reference" (specialized, with substitution context)

## Scope Tagging

Generate candidates liberally, but tag the apparent PR ownership scope:

- `Scope: Direct` for changed lines, newly added declarations, or comments changed by the PR.
- `Scope: Contextual` for nearby pre-existing code whose contract is changed, invalidated, or
  made important by the PR.
- `Scope: Probably out-of-scope` for concerns discovered while reading that may be real but
  are not clearly the PR author's responsibility.

Do not discard `Probably out-of-scope` candidates in this skill. The separate scope filter
will decide what is postable.

## Candidate Format

Use this exact structure:

````markdown
### C001: Short Title

- Status: Proposed
- Confidence: High | Medium | Low
- Scope: Direct | Contextual | Probably out-of-scope
- Category: comments | terminology | organization | invariant | API shape | decl-ref/substitution | correctness smell
- Location: `path:line`
- GitHub link:

Context:

```cpp
<small PR-version snippet>
```

Proposed comment:

> <comment text suitable for GitHub after filtering>

Notes:
<why this candidate was raised, uncertainty, related candidates>
````

Keep context snippets short enough to identify the issue, but include the relevant comment,
name, signature, or control-flow fragment. Prefer PR-version source context over only a
GitHub link.

## Working Process

1. Inventory the changed files and group them by feature or subsystem.
2. For each changed source file, read the surrounding type/function/section, not only the
   diff hunk.
3. Identify the new or changed concepts the PR introduces.
4. Ask whether the code teaches those concepts clearly enough for a future maintainer.
5. Write candidates as you go. Do not wait until the end and summarize from memory.
6. After the first pass, perform a coverage audit: revisit each changed file, changed
   declaration, and changed conceptual section and ask what candidate would have been written
   if the review were maximally strict. Add any credible missing candidates.
7. Deduplicate candidates that ask for the same clarification at the same conceptual level.
8. Leave borderline candidates in the file with `Scope: Probably out-of-scope` or
   `Confidence: Low` rather than silently discarding them.

## Not This Skill

Do not run the normal `REVIEW.md` severity filter here. Do not require a concrete crash,
miscompile, or missing test. Do not post to GitHub. Do not rewrite the PR.
