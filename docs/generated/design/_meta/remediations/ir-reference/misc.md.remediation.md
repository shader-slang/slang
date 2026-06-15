---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-12T14:15:48Z
target_doc: ir-reference/misc.md
review_report: ../../reviews/ir-reference/misc.md.review.md
target_doc_source_commit_before: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_source_commit_after: eb9403ef595a99c2ff6def1d538dbd7a792d9371
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/misc.md

## Summary

Both findings in the review were verified against source at the target
commit and fixed; none were rejected, deferred, or escalated. F-001 was
a genuine coverage gap: the concrete `capabilityConjunction` /
`capabilityDisjunction` opcodes were undocumented on any page, so a
capability-set sub-table was added to this catch-all. F-002 was a
genuine source-alignment error: the kernel-launch operand shapes and the
`CudaKernelLaunch` consumer link did not match the builder/emitter code,
so the two rows and the callout were corrected.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed `source/slang/slang-ir-insts.lua:871` declares `CapabilitySet` as a top-level group whose concrete children `capabilityConjunction` / `capabilityDisjunction` carry stable names 153/154 (`source/slang/slang-ir-insts-stable-names.lua:153-154`). This group is distinct from the `CapabilitySet` type opcode at `slang-ir-insts.lua:59` (wrapper `CapabilitySetType`) that types.md documents, so the two concrete opcodes were unclaimed by any page; the misc catch-all rule applies. | Added a `### Capability sets` sub-table under `## Opcodes` with rows for `capabilityConjunction` and `capabilityDisjunction` (variadic, hoistable, synthesized), citing `IRBuilder::getCapabilityValue` and noting the distinction from the type opcode. |
| F-002 | fixed | Confirmed `source/slang/slang-ir.cpp:3618-3619` appends variadic call args to `DispatchKernel` and `slang-ir.cpp:3630-3638` creates `CudaKernelLaunch` with five operands `baseFn, gridDim, blockDim, argsArray, cudaStream`; `source/slang/slang-emit-torch.cpp:71-99` consumes those five operands. The doc's prior operand shapes and its slang-emit-cuda.cpp consumer link were both wrong. | Corrected the `DispatchKernel` row to `baseFn, threadGroupSize, dispatchSize, args...`, the `CudaKernelLaunch` row to the five-operand shape with the torch-emitter link, and rewrote the `### CudaKernelLaunch` callout to describe the actual five operands and the `cudaLaunchKernel` operand mapping. |
