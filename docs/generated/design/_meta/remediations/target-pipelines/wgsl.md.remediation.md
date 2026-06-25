---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:15:55Z
target_doc: target-pipelines/wgsl.md
review_report: ../../reviews/target-pipelines/wgsl.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions: { fixed: 3, rejected_bogus: 0, rejected_out_of_scope: 0, deferred: 0, escalated: 0 }
---

# Remediation report for target-pipelines/wgsl.md

## Summary

All three review findings were correct and in-contract, so all three were fixed.
F-001 corrected the `floatNonUniformResourceIndex` callout to match the source, which
states WGSL drops the wrapper (no annotation exists) rather than forwarding it to a SPIR-V
NonUniform decoration via Tint. F-002 added the missing `checkStaticAssert` table row so the
Phase B table matches its diagram, and F-003 renamed the Phase D table column to the
contract-required `Pass`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed against `source/slang/slang-ir-float-non-uniform-resource-index.cpp:42-57`: WGSL/WebGPU has no non-uniform annotation, so the `CLikeSourceEmitter` base drops the wrapper at emit time and the decoration machinery is SPIR-V-only. The doc's claim that Tint forwards the marker to a SPIR-V NonUniform decoration is contradicted. | Rewrote the callout to state textual mode only repositions the wrapper and the emitter drops it for WGSL; removed the Tint-forwarding claim. |
| F-002 | fixed | Confirmed `checkStaticAssert` is a direct call after `specializeArrayParameters` (`slang-emit.cpp:1794`) and appears as diagram node `cSA`; the target-pipeline contract (`_common.md:326-335`) requires one table row per pass node. | Added Phase B table row 67 for `checkStaticAssert` (defined in `slang-emit.cpp` at line ~580; direct call after specialization). |
| F-003 | fixed | Confirmed the Phase D table used header `Pass / step`; the contract (`_common.md:329-335`) requires the exact column `Pass`. | Renamed the Phase D table second column from `Pass / step` to `Pass`. |
