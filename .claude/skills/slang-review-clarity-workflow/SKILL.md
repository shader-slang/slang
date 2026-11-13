---
name: slang-review-clarity-workflow
description: "Apply clarity acceptance requirements when authoring or revising Slang changes, including reassessment after review feedback. Also coordinate end-to-end clarity review: generate candidates, consolidate overlap, filter for PR-author scope, and optionally post one GitHub PR review."
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

Apply the clarity criteria while authoring Slang changes and coordinate clarity-focused reviews of other contributors' changes.
Authored work must satisfy the acceptance procedure below before it is ready to deliver.
For a review request, use the candidate-generation and posting workflow that follows that procedure.

The ordinary `REVIEW.md` bug-review process does not replace clarity review.
Do not apply its severity or confidence threshold to waive a naming, contract, structure, or explanation requirement.
Candidate generation and posting decisions are distinct from accepting authored work: producing or filtering candidates does not show that the change meets the requirements.

## Authoring and Self-Review

Use [slang-review-clarity](../slang-review-clarity/SKILL.md) for the problem statement, decomposition, contracts, and invariants, and [slang-review-fine-grained-clarity](../slang-review-fine-grained-clarity/SKILL.md) for consistency at the level of individual declarations, names, expressions, conditions, and comment sentences.
Read both criteria in full before implementation and apply them throughout revision.
Read `AGENTS.md` and `CLAUDE.md` from the checkout being changed.

### Establish the complete change and responsibility boundary

Record the assigned task, comparison base, branch or PR revision, and pre-existing local changes.
For a PR, use its actual base; for a branch or local patch, identify the corresponding comparison revision.
Review the cumulative change against that base, including changes carried forward from inherited commits within the assignment, new files, intended staged and unstaged edits, and fixes made during review.
The latest commit or latest response to feedback is insufficient as the review input.
Read any other local changes needed for context while preserving contributions outside the assignment.

Read complete declarations and the callers, producers, and consumers needed to understand an affected contract.
Apply [slang-review-scope-filter](../slang-review-scope-filter/SKILL.md) before acting on each finding.
New declarations, comments, tests, and documentation are in scope, as are existing contracts the change alters or makes misleading.
Calling an existing helper, changing a few lines of a large function, or discovering a nearby weakness does not authorize unrelated cleanup.
A review suggestion does not itself authorize a broader redesign.
When a root-cause fix requires an additional change, record its causal relationship to the assigned task.
Review iterations must preserve this boundary rather than use an incidental edit to justify more cleanup.

### Review and record coverage

Examine every in-scope declaration, name, expression, condition, and sentence in comments and documentation, including apparently trivial changes.
Do not sample or treat an overall impression of readability as examination of individual items.
For each condition, establish which semantic cases it distinguishes and why that distinction is valid.
For each comment or documentation sentence, identify its proposition and referents and check that the implementation supports the claim.
Exhaustive examination does not require a comment on every line; it requires the code and its explanation to make each line's task, necessity, and correctness clear.

Keep a compact review record outside the committed change, using task-specific filenames when sessions share a checkout.
Identify the comparison base, reviewed revision, and diff snapshot; include working-tree changes in that snapshot when present.
Inventory changed files, declarations, regions, and affected contracts, including headers, tests, diagnostics, and documentation.
For every inventoried region, record a finding, a reference to a finding covering the same concern, or a concrete reason the applicable criteria are satisfied.
For example, a predicate's name, documented true/false contract, and separately justified cases may supply its correctness argument.
A blanket statement that the code is "obvious" or "reviewed" is insufficient.
The record may group related items, but examination may not omit them.

Repeat both review perspectives and the following steps until the delivered change satisfies the completion requirements:

1. Save the cumulative diff and refresh the inventory.
   Read new files in order and changed regions with enough surrounding context to understand them.
2. Perform the high-level and fine-grained reviews and record concerns immediately.
   Audit coverage against the inventory before filtering findings.
3. Consolidate overlapping findings, apply the responsibility boundary, and resolve uncertainty through focused code, contract, or test investigation.
   The [consolidation](../slang-review-consolidate-candidates/SKILL.md) and [judgment-call](../slang-review-resolve-judgment-calls/SKILL.md) procedures supply the corresponding decisions.
   Preserve the reasons for dropped or narrowed findings.
4. Correct supported in-scope concerns at the appropriate level: design, decomposition, names, contracts, comments, or tests.
   When explanation remains difficult, revisit the structure and representation rather than accumulate prose.
5. Inspect the revised diff for unnecessary churn and newly introduced inconsistencies.
   Refresh coverage to describe the exact revision being delivered, including new helpers and revised explanations.
   Run validation appropriate to the actual changes.

### Reassess after review feedback

Feedback about clarity, quality, or style reopens the application of the relevant expectation across the cumulative assigned change before deciding whether the cited concern is supported, unsupported, or out of scope.
Apply the following process to every such feedback item:

1. Re-read the applicable requirement and re-examine every place in the cumulative assigned change where the expectation applies, including changes carried forward from earlier commits, tests, comments, and documentation.
   Inspect the concepts, contracts, decomposition, and implementation; searching for repetitions of the criticized words cannot establish compliance.
   For example, feedback that a predicate's name promises more than its conservative analysis proves requires checking other predicates and callers for the same mismatch, even when their names use different words.
   Reconsider the judgments that previously accepted those locations before deciding the cited concern's disposition.
   Do not dispose of feedback as a preference without examining the requirement and implementation.
2. Determine whether the concern is supported and within the responsibility boundary.
   Unsupported or out-of-scope feedback can be rejected with evidence; feedback does not expand the assignment.
   Explicit clarifications from the contributor directing the task are requirements for that task.
   Apply a genuinely new requirement throughout the current assignment without claiming that previous work violated it.
   Do not classify a requirement as new merely because the earlier review overlooked it; distinguish the cases using the instructions that applied to the reviewed work.
3. Correct every supported violation within the responsibility boundary.
   Reassess the fixes themselves: a renamed helper, extracted predicate, or replacement sentence can introduce another inconsistency.
4. When feedback demonstrates a violation of an already applicable requirement, the earlier self-review was insufficient.
   Record the expectation clarified by the feedback and the prior acceptance judgment that the evidence invalidates.
   Use concrete code and criteria rather than speculate about an agent's internal reasoning.
   Perform fresh high-level and fine-grained acceptance reviews of the full cumulative change.
   Prior "reviewed" dispositions cannot substitute for re-examination.
   Refresh the revision, inventory, coverage evidence, and finding dispositions before reporting the feedback addressed or the work ready.

New requirements and other addressing revisions still follow the ordinary iterative acceptance procedure above.
Unsupported feedback requires the expectation-wide examination and an evidence-backed disposition, but does not itself trigger fresh acceptance reviews of the entire change.
Fixing the cited location alone does not restore readiness.
Preserve the clarified expectation, pending reassessment, reviewed revision, and remaining findings in the task record so a resumed session continues the same obligation.

### Check what an independent reader can recover

Read each in-scope declaration and body without using the issue, PR discussion, or authoring conversation to supply its task, contract, or rationale.
When delegation is available and permitted by the task and harness, obtain an independent reader's check for substantial changes, difficult explanations, and feedback revealing a missed expectation about explaining the code.
Use a fresh reader context that excludes the authoring conversation and inherited task history.
Give the reader the Slang code, relevant referenced declarations, and established term definitions needed to assess it.
Withhold the author's intended interpretation and PR rationale: those would supply the explanation the code must establish.
Ask the reader what the code does and which facts or referents cannot be recovered.
Assess findings against the original responsibility boundary and revise supported deficiencies.
When delegation is unavailable or prohibited, perform this check directly and record that limitation.
After revisions, repeat the independent check for the explanations it found insufficient, or repeat the direct check when delegation is unavailable or prohibited.
Record the checked revision and result so an earlier reader pass cannot establish acceptance of revised explanations.

### Completion requirements

Before publishing or reporting completion, confirm that:

- Both review perspectives cover the actual cumulative delivered change, with inspectable reasons for acceptance.
- Every required feedback reassessment is complete for that reviewed diff snapshot.
- Required reader checks cover the revised explanations in the delivered diff.
- No supported, actionable, in-scope clarity concern remains unresolved, including concerns introduced by corrections.
- Scope and judgment decisions have evidence; volume, effort, lack of a runtime bug, or a posting threshold did not excuse a violation.
- Names, declarations, implementation, diagnostics, tests, and documentation state consistent contracts.
- The diff contains no unrelated cleanup accumulated during review.
- Appropriate validation is complete, with pending CI, unavailable checks, and concrete blockers reported accurately.

A single pass, successful tests, or a statement that self-review passed does not establish completion.
If a blocker prevents meeting a requirement, preserve the work and report the unresolved concern and evidence instead of declaring readiness.
Self-review does not authorize posting a GitHub review or other external communication.

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

The conventional input paths above are shared within a checkout.
Use an isolated checkout if concurrent sessions could overwrite those inputs.
Preserve a task-specific copy of the reviewed snapshot with the review record so later overwrites cannot replace its coverage evidence.

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
5. Before consolidation, audit generation coverage against the complete cumulative diff and file list.
   Confirm that every in-scope declaration, name, expression, condition, and sentence in comments and documentation was examined, including apparently trivial changes.
   For each region, record a candidate, a reference to a candidate covering the same concern, or concrete reasons the applicable clarity criteria are satisfied.
   Group related items in the record without omitting examination of individual items.
   If coverage is incomplete, extend the relevant generation pass before filtering.
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
review, local project policy for the account being used permits automated blocking reviews, and
the workflow is not using `--acting-as-bot-user`.

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

Evaluate behavior in isolated fixtures with a fixed comparison base, a revision captured before feedback, and the raw feedback available at that point.
Run paired baseline and revised-guidance trials with the same model, settings, task, and code in fresh contexts.
Withhold later discussion, subsequent fixes, and the grading rubric from the acting agent.
Grade the delivered code and recorded coverage of the full diff against independently prepared criteria; a claim that self-review passed is not evidence of success.
Measure missed applicable violations, corrections introducing new violations, and changes outside the responsibility boundary.
Include initial-authoring trials without feedback and unchanged-code controls, repeat paired runs, and report the sample size and variation.
A small replay can test review behavior but cannot establish compiler correctness or replace normal validation of implementation changes.

Use these scenarios when checking whether the workflow still behaves correctly:

- Initial authoring: without review feedback, examine every in-scope declaration, condition, and comment sentence and correct supported violations before submission.
- Feedback reassessment: feedback names one location, but other declarations violate the same expectation in different words; find and correct all applicable violations and repeat both acceptance reviews.
- Cumulative review: the latest commit satisfies a requirement while earlier assigned commits do not; use the full change against its base and catch the earlier violations.
- Correction review: a feedback fix introduces a misleading name or comment; detect it while reviewing the revised cumulative change.
- Responsibility boundary: an unchanged neighboring helper has a pre-existing clarity issue; leave it alone unless the assigned change affects its contract.
- Generation-only: given a PR number, produce high-level and fine-grained raw candidate files
  without posting.
- Generation coverage audit: given an in-scope declaration or region with neither a candidate nor concrete acceptance reasons, revisit generation before consolidation.
- Duplicate consolidation: given overlapping high-level and fine-grained candidates, keep the
  clearer comment and mark the duplicate or superseded candidate as dropped.
- Judgment-call resolution: given a candidate marked `Needs judgment call`, follow the local
  code context deeply enough to keep, revise, or drop it when possible.
- Scope filtering: given a pre-existing clarity issue not made worse by the PR, move it to the
  dropped section with a short rationale.
- Posting validation: given a postable-looking candidate missing scope or overlap metadata,
  the posting script must fail before making a GitHub API call.
