---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:05:46Z
target_doc: pipeline/06-emit.md
review_report: ../../reviews/pipeline/06-emit.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 2
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/06-emit.md

## Summary
Both findings were verified against the current target doc and the watched sources at HEAD. Each describes text the current draft no longer contains: the doc already qualifies the new-backend checklist by textual vs. direct path (F-001) and already narrows the include claim while noting SPIR-V's forward declaration (F-002). The reviewer assessed an earlier draft. No body edits were made; both findings are rejected-bogus.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | Current `## Adding a new backend` step 2 (`docs/generated/design/pipeline/06-emit.md` lines 233-243) already restricts `emitEntryPointsSourceFromIR` to "a textual target" and adds "A direct/non-textual backend instead follows the pattern of SPIR-V, LLVM, and VM bytecode: a separate emit function (`emitSPIRVForEntryPointsDirectly`, `emitLLVMForEntryPoints`, `emitVMByteCodeForEntryPoints`)". Source confirms these functions at `source/slang/slang-emit.cpp:3251`, `:3338`, and `source/slang/slang-emit-vm.cpp:1191`. The recommended qualification is already present. | — |
| F-002 | rejected-bogus | Current `## Emit dispatcher` (`docs/generated/design/pipeline/06-emit.md` lines 50-55) already states the includes "pull in the header-backed emit helpers used by this file" and that "Direct SPIR-V is not header-included here; it is wired via the `emitSPIRVFromIR` forward declaration and implemented in slang-emit-spirv.cpp". Source confirms no `slang-emit-spirv.h` in the include block (`source/slang/slang-emit.cpp` lines 14-25) and the forward declaration at `source/slang/slang-emit.cpp:2787`. The recommended narrower claim is already present. | — |
