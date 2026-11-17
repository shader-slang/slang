---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T21:23:55Z
target_doc: pipeline/04-ast-to-ir.md
review_report: ../../reviews/pipeline/04-ast-to-ir.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/04-ast-to-ir.md

## Summary

All three findings were verified against the current source and fixed in the document. The interface-requirement claim and the cache-resolution description were corrected, and the page was condensed below the per-document 32 KB limit. Breakdown: 3 fixed, 0 rejected, 0 deferred, 0 escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-lower-to-ir.cpp` lines 12353-12381 expand property/subscript accessors into per-accessor entries and skip `InterfaceDefaultImplDecl`, so "exactly one entry per member" was false. | Associated-type-bounds bullet now states accessor expansion and default-impl skipping, and limits the one-entry statement to relocated constraints. |
| F-002 | fixed | Prompt `pipeline-04-ast-to-ir.md` line 49 requires under 32 KB; the doc was 39,887 bytes. | Condensed dump examples, work-graph table, debug/diagnostic detail and prose to 31,922 bytes; required sections retained. |
| F-003 | fixed | `slang-lower-to-ir.cpp` lines 3099-3108 do no resolution; `lowerVal` resolves at line 3115 and `lowerType` passes its type directly (3129-3136). | Cache paragraph now says the helper only looks up/stores, `lowerVal` resolves first, `lowerType` keys by its input. |
