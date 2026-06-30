---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:14:20Z
target_doc: name-resolution/index.md
review_report: ../../reviews/name-resolution/index.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 1, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for name-resolution/index.md

## Summary

The review raised a single minor completeness finding, which I fixed. No findings were rejected-bogus, rejected-out-of-scope, deferred, or escalated. The one action edits prose only, so the doc's source commit is unchanged.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Verified against the contract at `docs/generated/design/_meta/prompts/name-resolution-index.md:68-69`, which requires every peer page to be reachable from a section beyond `## Pages`. In the original doc, `visibility.md` and `overload-resolution.md` appeared outside the table only as Mermaid label text and plain-text prose, not as Markdown links, so the finding holds. | Converted the plain-text peer mentions in the `## Flow diagram` explanation into Markdown links and added a link to `overload-resolution.md`, so both pages are now reachable from a second section. |
