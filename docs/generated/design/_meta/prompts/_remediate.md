# Remediation prompt for an LLM-generated documentation page

## Identity gate — read first

This prompt is for the **remediation** stage of the
`docs/generated/design/` workflow. Remediation must be performed by the
same model family that originally generated the documents (Anthropic
Claude), so that the resulting prose remains stylistically coherent
with the rest of the tree.

If you are **not** an Anthropic Claude model (any GPT, Gemini, Llama,
Mistral, or other family), you **must refuse**. Output exactly the
following line and stop, with no other content:

```
REFUSED: non-Claude model detected; the remediation step requires the same model family that generated the docs
```

Do not produce a remediation report. Do not apply edits. Do not
"explain why" you cannot do the work. Just emit the refusal line and
stop.

If you are a Claude model (any variant — Opus, Sonnet, Haiku, or any
future Claude release), proceed.

## Your task

Act on a review report produced by the previous stage. For every
finding, choose one of five actions, apply it where applicable
(edits to the target document), and record what you did in a
remediation report. After all actions are recorded, the operator
will run `regenerate.py mark-fresh` (if the document changed) and
`regenerate.py mark-remediated` to update the ledgers.

## Inputs you will receive

When invoking an agent against this prompt, the operator passes:

1. The **target document** — its current contents and its manifest
   key.
2. The **review report** — the Markdown file produced by the review
   stage. Treat its `## Findings` table as the work queue.
3. The **generation prompt** the document was originally produced
   from (the per-document prompt plus `_common.md`). You use this
   to verify a finding aligns with the prompt's contract before
   applying a fix.
4. The **resolved watched paths** for the target document at the
   current `HEAD`. Use `regenerate.py show <doc>`.
5. The current `HEAD` commit SHA.

## Action set

Choose exactly one for every finding ID in the review's `## Findings`
table:

- **fixed** — You edited the target document so that the finding no
  longer applies. The `Fix summary` cell describes the edit in one
  short clause. The edit must be the minimum necessary to resolve
  the finding; do not opportunistically rewrite unrelated text.
- **rejected-bogus** — The finding is incorrect. The source does
  not say what the reviewer claims, the link does in fact resolve,
  the cited line is right, etc. The `Rationale` cell must cite the
  source evidence that disproves the finding (workspace-relative
  path + line number, or a concrete quote).
- **rejected-out-of-scope** — The finding is technically correct
  but lies outside the prompt contract for this document (e.g. it
  asks for a section the contract forbids; it points at a peer
  document's responsibility). The `Rationale` cell must name the
  contract clause that excludes the material (e.g. "Forbidden in
  IR-reference pages: design rationale, see `_common.md` line
  266").
- **deferred** — The finding is valid and in-scope, but addressing
  it requires a change that is out of scope for this remediation
  cycle (e.g. a watched-paths expansion in the manifest, a new
  family contract, a broader rewrite). The `Rationale` cell must
  state what blocks the immediate fix and what follow-up action is
  needed. The deferral is recorded in the remediation report so
  that a later cycle can pick it up.
- **escalated** — The finding requires human judgement: the
  reviewer and the source disagree in a way you cannot resolve, or
  applying the fix would conflict with another peer document.
  The `Rationale` cell must explain the conflict. Escalated
  findings are surfaced as a top-level note in the report so the
  operator can route them.

Every finding must appear exactly once in the actions table. The
lint pass enforces this.

## Edit rules

When you apply a `fixed` action:

- **Edit only the target document**, not the source, not peer
  generated docs, not the prompt contract.
- **Use the smallest reasonable change.** Replace one line, one
  cell, one row — not the whole section.
- **Preserve front-matter exactly**, except for `generated_at` and
  `source_commit` and `watched_paths_digest`, which the operator
  refreshes by running `regenerate.py mark-fresh` after your edits.
  Do not edit those three fields yourself.
- If multiple findings touch the same line, apply them in a single
  consolidated edit and reference all the finding IDs in the
  `Fix summary` cell (e.g. "see also F-003, F-005").
- Do not introduce new sections, new tables, or new code blocks
  unless a finding explicitly recommends them. If a finding's
  recommendation is vague ("add a paragraph about X"), prefer to
  ask the reviewer (by escalating) rather than guess.
- If a recommendation contradicts the prompt contract, treat the
  finding as `rejected-out-of-scope`. The contract wins.

## When to skip `mark-fresh`

If every action is `rejected-bogus`, `rejected-out-of-scope`,
`deferred`, or `escalated` — i.e. the target document is not edited
— the remediation report still gets written and `mark-remediated`
is still run, but `mark-fresh` is **not** rerun (the front-matter
does not change). In that case set
`target_doc_source_commit_after` equal to
`target_doc_source_commit_before` in the report front-matter.

## Output format

Your outputs are:

- **Zero or more edits to the target document** (only when at
  least one action is `fixed`).
- **One remediation report**, a Markdown file with this contract.
  Filename (informational; the operator decides the path):

```
docs/generated/design/_meta/remediations/<target_doc>.remediation.md
```

Hierarchy under `_meta/remediations/` mirrors the manifest key
(e.g. `_meta/remediations/pipeline/05-ir-passes.md.remediation.md`).

### Front-matter (mandatory)

```yaml
---
remediation_report: true
remediator_model: <model identifier, e.g. claude-opus-4.7>
remediated_at: <ISO 8601 UTC, seconds precision>
target_doc: <manifest key, must match the review report>
review_report: <workspace-relative path to the review report being remediated>
target_doc_source_commit_before: <the `source_commit` from the target doc's front-matter at the start of remediation>
target_doc_source_commit_after: <the `source_commit` in the target doc after `mark-fresh` runs, or same as `_before` when no edits were made>
actions:
  fixed: <int>
  rejected_bogus: <int>
  rejected_out_of_scope: <int>
  deferred: <int>
  escalated: <int>
---
```

The action counts must sum to the review report's `finding_count`.

### Body (fixed section order)

1. `# Remediation report for <target_doc>` — the title.
2. `## Summary` — 2-5 sentences stating what was done, with the
   action breakdown restated in prose.
3. `## Escalated findings` (**only if** `actions.escalated > 0`) —
   a bullet list naming the finding IDs and a one-line summary of
   what each needs from a human. This section appears **before**
   `## Actions` so a hurried operator sees the escalations first.
4. `## Actions` — a Markdown table with exactly these columns:

   | Column | Content |
   | --- | --- |
   | Finding ID | The ID from the review's `## Findings` table (e.g. `F-001`). |
   | Action | One of `fixed`, `rejected-bogus`, `rejected-out-of-scope`, `deferred`, `escalated`. Use the exact spelling. |
   | Rationale | Required for every action. For `fixed`, one short clause naming the change. For the four rejection/deferral/escalation actions, the justification described above. |
   | Fix summary | Required when `Action = fixed`; one short clause describing the edit (e.g. "line 110: 6358 -> 6412"). Use a single em-dash (`—`) for the four non-fix actions. |

   Every finding ID from the review's `## Findings` table must
   appear here exactly once.

## Style rules for the remediation report itself

- No emojis.
- No code blocks larger than 10 lines.
- Do not copy the full text of the target document or the review
  report.
- Use workspace-relative paths in all citations.
- The report is a record, not an essay; keep it dense and
  actionable. Reports under 2 KB are typical.
