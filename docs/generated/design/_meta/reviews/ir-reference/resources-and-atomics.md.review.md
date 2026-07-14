---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:33:11+00:00
target_doc: ir-reference/resources-and-atomics.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: e27926ca78614bca20d3b57a5268d5884f642e04074ed66afbbed157eadbfdd7
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 1
  minor: 1
  nit: 0
---

# Review report for ir-reference/resources-and-atomics.md

## Summary
The page is structurally complete and its links/front matter look valid, but the checked source sample found two source-alignment problems. The most important issue is the atomic opcode table: several rows omit memory-order operands that the SPIR-V and Metal emitters read from the IR.

## Items checked
- Ran `regenerate.py show ir-reference/resources-and-atomics.md` and reviewed the target document, the common contract, its per-document prompt, and the resolved watched-file set (`source/slang/slang-ir-insts.h`, `source/slang/slang-ir-insts.lua`, `source/slang/slang-ir.cpp`, `source/slang/slang-ir.h`, `source/slang/slang-lower-to-ir.cpp`).
- Checked front matter for all required keys, the recorded source commit, the warning string, and a 64-character hex watched-path digest.
- Inspected the 19 relative markdown links in the document body and verified they point at workspace files or generated peer documents.
- Verified required IR-reference sections: `## Source`, `## Family hierarchy`, `## Opcodes`, `## Notable opcodes`, and `## See also`.
- Spot-checked more than 10 source-backed claims and named symbols against the watched files, including `AtomicOperation`, `IncrementCoverageCounter`, `IncrementFunctionCoverageCounter`, `IncrementBranchCoverageCounter`, `imageSubscript`, `imageLoad`, `imageStore`, `ImageTexelPointer`, `SubpassLoad`, `nonUniformResourceIndex`, `rwstructuredBufferGetElementPtr`, `sample`, `sampleGrad`, `ControlBarrier`, `LoadResourceDescriptorFromHeap`, and `BindingQuery`.
- Checked the prose line-number ranges in `## Source` against the Lua opcode clusters for atomic operations, coverage markers, texture/image ops, buffers, mesh outputs, wave/barrier ops, raytracing payload ops, binding queries, and descriptor heaps.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### Atomic operations`, rows for `atomicLoad`, `atomicStore`, `atomicExchange`, `atomicCompareExchange`, and RMW atomics | The table underreports the IR operand shape for atomics. It says rows such as `atomicAdd` have only `ptr, val` and `atomicCompareExchange` has only `ptr, expected, desired`, even though the emitters read memory-order operands from later operand slots. This contradicts the page's own prose that memory ordering is part of the atomic opcode shape. | `source/slang/slang-emit-spirv.cpp:5452` reads `atomicLoad` operand 1 as memory semantics, `source/slang/slang-emit-spirv.cpp:5481` reads `atomicStore` operand 2, `source/slang/slang-emit-spirv.cpp:5530`-`5533` read `atomicCompareExchange` operands 3 and 4, and `source/slang/slang-emit-spirv.cpp:5561`-`5562` read RMW operand 2. `source/slang/slang-ir.cpp:5556`-`5564` also constructs `atomicStore` with `dstPtr`, `srcVal`, and `memoryOrder`. | Update the atomic operand cells to include the memory-order operand(s): `atomicLoad` should include `ptr, memoryOrder`; `atomicStore` and `atomicExchange` should include `ptr, val, memoryOrder`; `atomicCompareExchange` should include `ptr, expected, desired, memoryOrderEqual, memoryOrderUnequal`; RMW atomics should include `ptr, val, memoryOrder` where applicable. |
| F-002 | minor | `### EntryPointParam and GlobalParam (cross-link)` | The notable-opcode heading names `EntryPointParam` as if it were an opcode peer of `global_param`, but the Lua source has `global_param` as an opcode and `EntryPointParamDecoration` as a decoration, not an `EntryPointParam` opcode. | `source/slang/slang-ir-insts.lua:817` declares `{ global_param = { global = true } }`. `source/slang/slang-ir-insts.lua:1975`-`1978` declares `EntryPointParamDecoration` as a decoration used for parameters moved to global params. A source search found `EntryPointParam` only in decoration/helper names, not as an opcode entry. | Rename the callout to avoid implying an `EntryPointParam` opcode exists, for example `global_param and EntryPointParamDecoration`, and explain that entry-point-origin information is represented by `EntryPointParamDecoration` on a `global_param`. |

## No-issues notes
- The document keeps generated-doc links workspace-relative and avoids absolute source paths.
- The page stays within the documented scope for its family and does not copy handwritten documentation prose.
- The source citations are concentrated in the watched paths listed by the manifest entry.
