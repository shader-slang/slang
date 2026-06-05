---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: target-pipelines/spirv.md
review_report: ../../reviews/target-pipelines/spirv.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/spirv.md

## Summary

All four findings were fixed. The three entry-point-shape passes were
moved from Phase A to Phase C (after `resolveTextureFormat`, before
`legalizeEntryPointsForGLSL`) in both the diagrams and tables, with
row renumbering and stale step-number cross-references updated. The
Phase D spirv-val description now states the freshly emitted buffer
is validated, the `legalizeEmptyTypes` file link was corrected, and
the editorial "re-enable the disabled block" recommendation was made
neutral.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-emit.cpp:982-1001` holds only borrow/replace/bind passes; `1952-1962` runs the three varying passes after specialization. | Moved `translateGlobalVaryingVar`/`resolveVaryingInputRef`/`fixEntryPointCallsites` from Phase A (diagram+rows 5-7) to Phase C (after `resolveTextureFormat`, new rows 3-5); renumbered both tables and updated step references and phase prose. |
| F-002 | fixed | `source/slang/slang-emit.cpp:3135-3138` validates `spirv.getBuffer()`, not the linked artifact replaced at 3131. | Clarified the spirv-val Notable-passes bullet to say it validates the freshly emitted `spirv` buffer even after `spirv-link` replaces `artifact`. |
| F-003 | fixed | `source/slang/slang-ir-legalize-types.cpp:4026` defines `legalizeEmptyTypes`. | Changed the `legalizeEmptyTypes` Phase C row File link from `slang-ir-legalize-empty-array.cpp` to `slang-ir-legalize-types.cpp`. |
| F-004 | fixed | `source/slang/slang-emit.cpp:3053-3060` shows only a disabled `#if 0` block with no re-enable guidance. | Replaced the "future readers should re-enable that block" closing sentence with a neutral statement that the inline `optimizeSPIRV` is disabled. |
