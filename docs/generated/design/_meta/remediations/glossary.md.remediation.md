---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:16:15Z
target_doc: glossary.md
review_report: ../reviews/glossary.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for glossary.md

## Summary

The review reported a single major finding (F-001) and it was fixed. The `## Cross-reference index` table omitted two peer pipeline documents that the manifest's `docs/generated/design/pipeline/*.md` glob includes; two rows were added. No findings were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Verified: `pipeline/04b-pre-link-passes.md` and `pipeline/04c-layout-ir.md` both exist and appear in the resolved file list from `regenerate.py show glossary.md` (matched by the glob at `manifest.yaml:756`). The prompt at `prompts/glossary.md:135` requires the index to cover every peer doc in the manifest entry, yet the table jumped from `pipeline/04-ast-to-ir.md` straight to `pipeline/05-ir-passes.md`. The glossary already defines `layout IR module` (See: 04c) and `mandatory optimization pass` (See: 04b), so both terms exist to populate the rows. | Added two cross-reference index rows after `pipeline/04-ast-to-ir.md`: `04b-pre-link-passes.md` -> `mandatory optimization pass`, and `04c-layout-ir.md` -> `layout IR module`. |
