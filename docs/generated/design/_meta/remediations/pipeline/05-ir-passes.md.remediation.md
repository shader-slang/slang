---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:05:39Z
target_doc: pipeline/05-ir-passes.md
review_report: ../../reviews/pipeline/05-ir-passes.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/05-ir-passes.md

## Summary

Both minor source-alignment findings were verified against source and are resolved in the target document. F-001 concerns the false claim that `propagateConstExpr` re-runs from `linkAndOptimizeIR`; the document already drops it from the "invoked both" list and states it runs pre-link only. F-002 concerns helper-infrastructure rows in the target-specific lowering table; the document already keeps `Translate` under Pass utilities and `SPIR-V snippet` under Other passes, not as target-gated passes. The corrective edits were already present in the document at the reviewed commit, so no further edits were applied this cycle and front-matter is unchanged. Two fixed; zero rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Verified `propagateConstExpr` is called only at `source/slang/slang-lower-to-ir.cpp:15341` and never in `slang-emit.cpp` (grep returns no match); post-link cleanup is `simplifyIR` running SSA/SCCP/SimplifyCFG/DCE (`source/slang/slang-ir-ssa-simplification.cpp:50`, called at `slang-emit.cpp:1452`). The document at "How the passes are ordered" (lines 71-76) already lists only constructSSA, eliminateDeadCode, simplifyCFG, peepholeOptimize as invoked both, and states `propagateConstExpr` runs pre-link only while post-link uses the `simplifyIR` fixed point. The finding is resolved; no further edit needed this cycle. | Lines 71-76 already exclude `propagateConstExpr` from the "invoked both" list and state it runs pre-link only. |
| F-002 | fixed | Verified `SpvSnippet` is a parsed SPIR-V ASM helper (`source/slang/slang-ir-spirv-snippet.h:22`) and `slang-ir-translate` exposes a shared translation dictionary / `TranslationContext` (`source/slang/slang-ir-translate.h:18`) cleared as shared post-link cleanup (`slang-emit.cpp:1479`); neither is a target-gated pass. The document already keeps `Translate` under "Pass utilities" (line 355) and `SPIR-V snippet` under "Other passes" (line 315); the "Target-specific lowering" table (lines 282-296) contains neither row. The finding is resolved; no further edit needed this cycle. | Target-specific table holds no snippet or Translate row; `Translate` sits in Pass utilities (line 355), `SPIR-V snippet` in Other passes (line 315). |
