---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:01:53Z
target_doc: ir-reference/resources-and-atomics.md
review_report: ../../reviews/ir-reference/resources-and-atomics.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/resources-and-atomics.md

## Summary
Both findings were valid and in-scope; both were fixed by minimal edits to the target document. F-001 corrected the atomic-opcode operand cells to include the `IRMemoryOrder` operand(s) the emitters actually read, and updated the `atomicCompareExchange` callout to match. F-002 renamed the misleading notable-opcode heading so it no longer implies a non-existent `EntryPointParam` opcode. No rejections, deferrals, or escalations.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed: `source/slang/slang-emit-spirv.cpp:5452/5481/5505/5531-5533/5562` read memory-order operands at slots 1/2/2/3-4/2; `source/slang/slang-ir.cpp:5556` builds `AtomicStore(dstPtr, srcVal, memoryOrder)`. The doc's own prose and the per-doc prompt mandate the memory-order operand. | Atomic table: `atomicLoad`->`ptr, memoryOrder`; `atomicStore`/`atomicExchange`/RMW->`ptr, val, memoryOrder`; CAS->`ptr, expected, desired, memoryOrderEqual, memoryOrderUnequal`; updated CAS callout to match |
| F-002 | fixed | Confirmed: `source/slang/slang-ir-insts.lua:817` declares the `global_param` opcode; `source/slang/slang-ir-insts.lua:1974-1979` declares `EntryPointParamDecoration` (a decoration); no `EntryPointParam` opcode exists. | Renamed heading to "global_param and EntryPointParamDecoration (cross-link)" and clarified entry-point origin is carried by the decoration, not an opcode |
