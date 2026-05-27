---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T21:15:00+00:00
target_doc: glossary.md
review_report: ../reviews/glossary.md.review.md
target_doc_source_commit_before: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for glossary.md

## Summary

Both findings addressed: `DiagnosticSink` now appears before
`differential pair`, and the `architecture/overview.md`
session-vs-linkage entry was already corrected during Phase 2i so
the glossary's `session` entry is no longer in conflict with its
peer.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The prompt mandates alphabetical ordering; case-insensitive alphabetical puts `Diagnostic*` (di-a) before `differential` (di-f). | Moved the `**DiagnosticSink**` entry to appear before `**differential pair**`. Spot-checked the rest of the glossary for similar inversions and found none — all other adjacent pairs (`decl-ref` < `decoration`; `IRBuilder` < `IRDecoration` < `IRFunc` < `IRInst` < `IRModule` < `IROp`; `lookup *` cluster) remain alphabetical. |
| F-002 | fixed | The glossary entry was correct; the conflict was in `architecture/overview.md`, which I already corrected during Phase 2i (Session ↔ IGlobalSession, Linkage ↔ ISession). | No change to `glossary.md` needed; the F-002 cross-doc inconsistency is closed by the `architecture/overview.md` remediation reported separately. |
