---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:09:45Z
target_doc: ir-reference/differentiation.md
review_report: ../../reviews/ir-reference/differentiation.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 8
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/differentiation.md

## Summary

All eight findings were verified against the current source and fixed in the document. The doc grew from 46439 to about 47.6 KB, under the 65536 cap. Actions: 8 fixed, 0 rejected, 0 deferred, 0 escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `slang-lower-to-ir.cpp:11138-11165` emits `synModifier->op` from an `InheritanceDecl`; `slang-check-decl.cpp:9877,16261` pass through `extendContainerDecl`. | Source paragraph qualifies the single-site claim and adds the inheritance path; both witness rows now name `InheritanceDecl` with `SynthesizedModifier`. |
| F-002 | fixed | `slang-emit-spirv.cpp:6327-6335` turns `NameHintDecoration` into `OpName`. | Replaced the no-OpName and every-target claims with a name-hint description. |
| F-003 | fixed | `slang-ir-autodiff-unzip.cpp:796-803` replaces a primal projection of a mapped pair. | Removed the blanket no-fold statement; cited unzip elimination. |
| F-004 | fixed | `slang-ir.cpp:2246-2250` and `slang-ir.h:1962-1971` include the full type in the key. | Key described as opcode, full result type, operands; example qualified. |
| F-005 | fixed | `slang-ir-insts.lua:2857-2891` declares only `min_operands`; `_common.md:272-274` mandates `(N unnamed)`. | Cells now `(N unnamed)`, `val` with dagger for accessor-named cells, legend corrected, two producers relabeled `no producer at HEAD`. |
| F-006 | fixed | `slang-ir-dce.cpp:645-649` is the `ReportCheckpointStore` rule; 652 is `Annotation`. | Citation changed to lines 645-649. |
| F-007 | fixed | `_common.md:288-295` requires AST-reference links; the three pages exist. | Added a See also bullet linking expressions, declarations, values. |
| F-008 | fixed | `slang-ir-insts.h:2457,2466,2474` declare three bases. | "two" changed to "three". |
