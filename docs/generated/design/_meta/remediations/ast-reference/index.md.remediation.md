---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T13:45:00Z
target_doc: ast-reference/index.md
review_report: ../../reviews/ast-reference/index.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/index.md

## Summary

The single finding was fixed. `## How to navigate` really did run to
seven sentences across three paragraphs, against the per-document
prompt's three-to-four-sentence budget, so the section was condensed to
four sentences in two paragraphs. All four required elements — the
`base.md` starting point, the family-page shortcut, where abstract
intermediates live, and the literal reading of the `Grammar` column —
are retained. No other section and no front-matter field was touched.

## Actions

| Finding ID | Action | Rationale                                                                                                                                                                                                                         | Fix summary                                                                                                                                                                      |
| ---------- | ------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | fixed  | Confirmed at HEAD: the section held seven sentences (two per paragraph in the first two paragraphs, three in the third), while `docs/generated/design/_meta/prompts/ast-reference-index.md:51` specifies "3-4 sentence guidance". | Condensed `## How to navigate` from seven sentences to four, folding the abstract-intermediate and `Grammar`-column paragraphs into one and dropping the sub-table parenthetical |
