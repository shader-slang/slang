---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T21:03:56Z
target_doc: target-pipelines/cuda.md
review_report: ../../reviews/target-pipelines/cuda.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 8
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for target-pipelines/cuda.md

## Summary

Eight of nine findings were verified against the current source and fixed in the document; the doc stays under its 98304-byte cap (97421 bytes) because the F-002 and F-006 edits shortened text. F-004 (diagram control-flow rewrite) is deferred. Action breakdown: 8 fixed, 1 deferred.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | source/slang/slang-code-gen.cpp:268-269 maps only PTX to CUDASource; lines 1270-1285 keep CUDAHeader as its own target. | Intro and Phase A: CUDAHeader keeps its own target value; removed "all three" claim. |
| F-002 | fixed | source/slang/slang-ir-legalize-varying-params.cpp:1779-1786 and 2004-2006 skip unresolved callees, so a lookupWitness callee never reaches the diagnostic. | Replaced the dynamic-dispatch test paragraph with the resolvable-callee boundary. |
| F-003 | fixed | slang-ir-lower-buffer-element-type.cpp:2698-2714 keeps default-layout Natural matrices; slang-ir-legalize-matrix-types.cpp:18-40 excludes CUDA; slang-ir-lower-cuda-builtin-types.cpp:163-167 builds a scalar array. | Matrix arm described as reachable; storage struct holds scalar array; Phase C rows 18 and 20 qualified as shared ordering. |
| F-004 | deferred | Valid, but redrawing four diagrams with gate diamonds, per-pass nodes, init node and a validation summary is a broader rewrite that cannot fit the 98304-byte cap (about 880 bytes free) without cutting content. | — |
| F-005 | fixed | docs/user-guide/09-targets.md:355 has "CUDA and OptiX"; contract item 9 requires the link. | Added user-guide link to See also. |
| F-006 | fixed | _common.md item 6 requires a single gates table. | Merged four tables into one two-column table; removed sub-headings. |
| F-007 | fixed | slang-ir-optix-entry-point-uniforms.cpp:259 createParam, 228-232 replaced by kIROp_GetOptiXSbtDataPtr and deleted. | Rewrote "already a global" explanation. |
| F-008 | fixed | slang-emit.cpp:1902-1904, 2354-2358, 2439-2442 are other CUDA-only calls. | Replaced exclusivity claim with "a CUDA-specific optimization". |
| F-009 | fixed | slang-emit.cpp:423-434, 522-528, 1592-1598 show an IR-content predicate only. | Removed the importance ranking; stated the two paths. |
