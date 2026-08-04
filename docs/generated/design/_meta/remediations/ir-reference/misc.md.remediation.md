---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:00:21Z
target_doc: ir-reference/misc.md
review_report: ../../reviews/ir-reference/misc.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 2
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/misc.md

## Summary
Two major findings reviewed; both rejected as bogus. Each describes a prior
state of the document that the current text no longer matches: the corrections
the reviewer recommends are already present in the target document at this
commit. No edits were applied this cycle, so `target_doc_source_commit_after`
equals `_before`.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | Finding claims the page presents `CudaKernelLaunch` as five operands `baseFn, gridDim, blockDim, argsArray, cudaStream`. The current doc does the opposite: the `### Kernel launch` table at `docs/generated/design/ir-reference/misc.md:242` lists the six Lua-declared operands `kernel, gridDimX, gridDimY, gridDimZ, blockDimX, blockDimY`, matching `source/slang/slang-ir-insts.lua:2684`, and the `### CudaKernelLaunch` callout (lines 308-325) explicitly states the Lua declares six while `IRBuilder::emitCudaKernelLaunch` (`source/slang/slang-ir.cpp:3676`) uses five. The recommended change is already in place. | — |
| F-002 | rejected-bogus | Finding claims no `### getStringHash` notable callout exists. It is present at `docs/generated/design/ir-reference/misc.md:274-284`, describing the single `stringLit: IRStringLit` operand and the compile-time stable-hash role, consistent with `source/slang/slang-ir-insts.lua:1530`. | — |
