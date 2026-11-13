---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T21:21:58Z
target_doc: pipeline/04b-pre-link-passes.md
review_report: ../../reviews/pipeline/04b-pre-link-passes.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/04b-pre-link-passes.md

## Summary

All four findings were verified against the current source and found correct. All four were fixed with minimal edits to the target document; front-matter `generated_at` was refreshed and the digest is unchanged.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir-link.cpp` lines 2611-2651: `IRPrelinkContext::maybeCloneValue` creates witness tables without entries, bypassing `shouldDeepCloneWitnessTable` | Notable passes: replaced "always deep-clones" with the shared-predicate vs prelink-override distinction |
| F-002 | fixed | `source/slang/slang-ir-inline.cpp` line 213 `canInline` gates sites; `source/slang/slang-ir-dce.cpp` ~541 keeps exports alive (`source/slang/slang-lower-to-ir.cpp` line 15650) | Early-inlining section: eligible call sites only; exported tagged definitions can survive |
| F-003 | fixed | `source/slang/slang-ir-constexpr.cpp` lines 543-557 emit E40013 only when the argument cannot be marked constexpr; propagated demands give E40012 | Phase D table E40012 row: removed the absolute claim, cited the regression test |
| F-004 | fixed | `source/slang/slang-compile-request.h` lines 318-333: register-space macro is optional, defaults to `space0` | Phase A: only `NV_SHADER_EXTN_SLOT` triggers; space macro optional |
