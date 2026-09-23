---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T14:10:00Z
target_doc: pipeline/04c-layout-ir.md
review_report: ../../reviews/pipeline/04c-layout-ir.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 0
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/04c-layout-ir.md

## Summary

The review raised a single major finding, and it concerns the front-matter `watched_paths_digest`. That field is owned by the operator's `mark-fresh` run, not by remediation, so the finding is rejected as out of scope. No fix, rejection-as-bogus, deferral, or escalation was recorded, and the target document was not edited.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-out-of-scope | The digest value the reviewer computes is correct, but `docs/generated/design/_meta/prompts/_remediate.md` lines 97-100 reserve `generated_at`, `source_commit`, and `watched_paths_digest` for the operator's `regenerate.py mark-fresh` run and state "Do not edit those three fields yourself." Since no other finding required an edit, `mark-fresh` will not be rerun for this page and the digest stays as recorded. | — |
