---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T22:00:00Z
target_doc: target-pipelines/index.md
review_report: ../../reviews/target-pipelines/index.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/index.md

## Summary

All five findings were verified against the current source and sibling pages and fixed with edits to the index. The removed HLSL function, the two exclusivity claims, and the false subsection-heading claim were corrected; the per-pass detail the index contract forbids was cut. The document was edited and its front-matter refreshed.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed: no `legalizeRayPayloadAccessQualifiersForHLSL` anywhere in `source/`; `validateBarrierFlagsForHLSL` is called at `source/slang/slang-emit.cpp:1918` and defined in `source/slang/slang-ir-hlsl-legalize.cpp:67`. | HLSL Phase C cell: removed the obsolete example, kept `validateBarrierFlagsForHLSL`. |
| F-002 | fixed | Confirmed: `source/slang/slang-ir-legalize-varying-params.cpp:1966` has a `for (bool changed = true; changed;)` loop inside a pass; `cuda.md` documents it. | Removed "only target with iterative passes" from the SPIR-V cell; CUDA Loops cell now notes the pass-internal loop. The Pages-bullet claim went with F-004. |
| F-003 | fixed | Confirmed: `source/slang/slang-emit.cpp:1904` (`legalizeOptiXReportIntersectionsForCUDA`) and 2358 (`synthesizeActiveMask`) are CUDA-only too. | CUDA Phase C cell: "the one pass that exists solely for this target family" -> "e.g." example. |
| F-004 | fixed | Confirmed: `_common.md` "Forbidden in the index: per-pass details"; the prompt asks for one-clause bullets and a short filtering paragraph. | Pages bullets cut to one clause each; IR-content subsection reduced to a short gating summary, autodiff and tagged-union bullets removed; SPIR-V loop caveat and table kept. |
| F-005 | fixed | Confirmed: `wgsl.md` and `cuda.md` have no `### requiredLoweringPassSet.* flags` heading (grep of `^### ` finds none). | Shared shape and Filtering rules now say each page discusses the gating in Conditional gates, without the heading claim; see also F-004. |
