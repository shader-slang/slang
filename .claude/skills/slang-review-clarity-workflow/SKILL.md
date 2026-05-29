---
name: slang-review-clarity-workflow
description: "Run the full Slang clarity review workflow: generate high-level and fine-grained candidates, consolidate overlap, filter for PR-author scope, and optionally post one GitHub PR review. Use when asked to perform an end-to-end clarity-focused review."
argument-hint: "<pr-number-or-diff-path>"
allowed-tools:
  - Bash
  - Read
  - Grep
  - Glob
  - Write
  - Edit
---

# Slang Clarity Review Workflow

Coordinate the full clarity-review process.

Do not integrate this with the ordinary `REVIEW.md` bug-review process unless the user
explicitly asks. This workflow has a different purpose: produce and post clarity-focused
review feedback.

## Files

For PR `<number>`, use:

```text
tmp/pr-diff.patch
tmp/pr-files.txt
tmp/pr-view.json
tmp/review-candidates/pr-<number>-clarity.md
tmp/review-candidates/pr-<number>-fine-grained-clarity.md
tmp/review-candidates/pr-<number>-clarity-workflow.md
```

The first two candidate files are raw generation outputs. The workflow file is the canonical
file after consolidation. After consolidation, update the canonical file in place. The
canonical file may also contain top-level `## PR Summary` and `## Review Body` sections before
candidate entries.

## Workflow

1. Save the PR diff and file list under `tmp/` using Windows-native `gh.exe`:

   ```bash
   mkdir -p tmp/review-candidates
   gh.exe pr diff <number> -R shader-slang/slang > tmp/pr-diff.patch
   gh.exe pr view <number> -R shader-slang/slang --json files -q '.files[].path' > tmp/pr-files.txt
   gh.exe pr view <number> -R shader-slang/slang --json title,body,files,additions,deletions > tmp/pr-view.json
   ```

2. Create a short PR summary. Compare what the PR title/description says with what the diff
   appears to do. Note mismatches and the PR's apparent value proposition.
3. Run `slang-review-clarity` to produce high-level candidates.
4. Run `slang-review-fine-grained-clarity` to produce fine-grained candidates.
5. Before consolidation, do a generation coverage audit. Re-read `tmp/pr-files.txt`, the raw
   candidate files, and the changed hunks. Confirm that every changed file, touched
   declaration, and non-trivial changed code/comment region is either covered by a candidate or
   is obviously clear, necessary, and correct. If there are gaps, rerun or extend the relevant
   generation pass before filtering.
6. Run `slang-review-consolidate-candidates` to merge raw outputs into the canonical workflow
   file and resolve duplicates, overlaps, and superseded comments.
7. Run `slang-review-scope-filter` on the canonical workflow file in place.
8. Run `slang-review-resolve-judgment-calls` on candidates that need a judgment call.
9. Create or update the canonical file's `## Review Body`. Write it like a competent human
   reviewer: summarize the state of the PR relative to its size and value, call out whether it
   feels rough or close to merge-ready, and name the key areas the author should focus on.
   Write the section content as a strict Markdown blockquote: after leading/trailing blank
   lines, every line before the next top-level section or candidate entry must start with `>`,
   blank lines must be written as `>`, and headings inside the review body must be quoted,
   e.g. `> ## Main Concerns`. Do not use lazy continuation lines.
   Unless the workflow is posting through a GitHub account that already identifies the agent,
   the first line must identify the review as agent-authored. Use the form
   `<agent name>-authored <optional review type> review:`, such as
   `GPT-4.1-authored clarity review:` or `Claude 3.7 Sonnet authored review:`. The separator
   before `authored` may be whitespace instead of `-`. The agent name may be any non-newline
   text up to 50 Unicode scalar values.
10. Validate the canonical file before posting.

    The posting script enforces the mechanical blocking checks:

    - no duplicate candidate IDs;
    - no postable candidate remains `Status: Proposed` or another unfiltered status;
    - every postable candidate has `Scope decision`, `Scope rationale`, `Overlap decision`,
      and `Overlap rationale`;
    - every postable candidate has a `Location` that names a line/range present in the GitHub
      diff;
    - every postable candidate has a non-empty strict-blockquote `Proposed comment:`;
    - the `## Review Body` section, if present, uses strict blockquote formatting for every
      content line;
    - unless `--acting-as-bot-user` will be used, the review body starts with the required
      agent-authorship attribution.

    The agent must also check the judgment-based posting policy:

    - dropped, duplicate, superseded, and merged candidates are preserved later in the file for
      auditability;
    - candidates about code outside the diff are attached to the closest or most logical
      commentable diff line, and their proposed comment clearly names the actual code or
      contract they concern;
    - proposed comments do not include candidate IDs, status, confidence, scope, notes, or
      other process metadata.
11. If the user asked to post, run `slang-review-post-github` on the canonical workflow file.

If a harness cannot invoke repository-local skills by name, read and apply the corresponding
`SKILL.md` files under `.claude/skills/` in the sequence above.

Raw generation may write separate files so the two review passes can run independently without
append races. Sequential workflows may instead pass the canonical file as an append target to
the review skills, but consolidation still needs to run before scope filtering.

## Posting Defaults

Default posted review result is `COMMENT`. This repository's automated-review policy treats
bot-authored reviews as non-blocking advisory reviews, and may reject or dismiss automated
`REQUEST_CHANGES` reviews.

Use `REQUEST_CHANGES` only when the initial user prompt explicitly asks for a blocking clarity
review and local project policy for the account being used permits automated blocking reviews.

Never use `APPROVE` for this workflow.

Include `Needs judgment call` candidates by default. The workflow is expected to run mostly
without a human in the loop, so uncertain-but-credible comments should not disappear just
because they required extra focused analysis.

Never post individual PR thread comments for this workflow. Posting must create one proper
GitHub PR review with comments attached to diff lines/ranges.

Do not omit the agent-authorship label unless the workflow is running under GitHub credentials
that already identify a bot or agent account. In that case, pass `--acting-as-bot-user` to the
posting script.

## Evaluation Scenarios

Use these scenarios when checking whether the workflow still behaves correctly:

- Generation-only: given a PR number, produce high-level and fine-grained raw candidate files
  without posting.
- Generation coverage audit: given raw candidates that leave a changed non-trivial function or
  file with no candidate and no explicit obvious-clarity rationale, the workflow must revisit
  generation before consolidation.
- Duplicate consolidation: given overlapping high-level and fine-grained candidates, keep the
  clearer comment and mark the duplicate or superseded candidate as dropped.
- Judgment-call resolution: given a candidate marked `Needs judgment call`, follow the local
  code context deeply enough to keep, revise, or drop it when possible.
- Scope filtering: given a pre-existing clarity issue not made worse by the PR, move it to the
  dropped section with a short rationale.
- Posting validation: given a postable-looking candidate missing scope or overlap metadata,
  the posting script must fail before making a GitHub API call.
