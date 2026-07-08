---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:06:49Z
target_doc: target-pipelines/metal.md
review_report: ../../reviews/target-pipelines/metal.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 2
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/metal.md

## Summary
Both findings (1 critical, 1 major) concern the MetalLib and MetalLibAssembly downstream narrative. Verified against the target document at source_commit c21ead269 and the watched source, each finding describes a doc state the current page does not exhibit; the page already presents the corrected narrative both findings recommend. The reviewer's cited line ranges (836-840, 891-895) do not match current content, indicating the review ran against an earlier revision. No edits were made this cycle; front-matter is unchanged and target_doc_source_commit_after equals before.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | The doc does not claim MetalLibAssembly skips the wrap; the cited quote a direct request for MetalLibAssembly would not get the wrap is absent from the page. Intro at docs/generated/design/target-pipelines/metal.md lines 16-31, the Phase B note at lines 185-188, the gates row at line 737, and the wrapCBufferElementsForMetal callout at lines 855-860 already state the assembly target comes from an intermediate MetalLib emitted from intermediate Metal source, so linkAndOptimizeIR runs as CodeGenTarget::Metal and the wrap always fires. Source confirms source/slang/slang-emit.cpp lines 1812-1816 places the pass in the Metal and MetalLib arm. | — |
| F-002 | rejected-bogus | Phase D is not collapsed and has no or its disassembly prose. The diagram at lines 637-656 has three arms: Metal terminal, MetalLib to the Apple metal compiler, and MetalLibAssembly to the compiler then to metal-objdump. Table rows 8 and 9 at lines 667-668 separate the Apple metal MetalC compile from metal-objdump --disassemble; the gates row at line 739 and the downstream callout at lines 909-918 name the disassembler explicitly. Source confirms metal-objdump --disassemble at source/compiler-core/slang-metal-compiler.cpp lines 48-53. | — |
