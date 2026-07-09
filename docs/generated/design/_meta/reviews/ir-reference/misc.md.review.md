---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:27:15+00:00
target_doc: ir-reference/misc.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: e27926ca78614bca20d3b57a5268d5884f642e04074ed66afbbed157eadbfdd7
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 2
  minor: 0
  nit: 0
---

# Review report for ir-reference/misc.md

## Summary
The misc page has two actionable issues. The most important one is that `CudaKernelLaunch` is documented with the five-operand `IRBuilder` helper shape, while the Lua opcode declaration that the reference tables are supposed to follow declares six named operands.

## Items checked
- Ran `regenerate.py show ir-reference/misc.md` and reviewed the target document, `_common.md`, `ir-reference-misc.md`, `cross-cutting/ir-instructions.md`, and `pipeline/04-ast-to-ir.md`.
- Checked front matter for all required generated-doc keys, the target source commit, the warning string, and a 64-character hex watched-path digest.
- Resolved the relative links in the document body to watched source files and generated peer docs, including all sibling IR-reference links and `../pipeline/04-ast-to-ir.md`.
- Verified the required IR-reference sections: `## Source`, `## Family hierarchy`, `## Opcodes`, `## Notable opcodes`, and `## See also`.
- Spot-checked more than 10 claims against watched files, including `PackBranch`, `MakeWitnessPack`, `Each`, `getStringHash`, `IsType`, `sizeOf`, `alignOf`, `countOf`, `CastStorageToLogicalBase`, `LiveRangeMarker`, `CudaKernelLaunch`, and `IRBuilder::emitCudaKernelLaunch`.
- Checked representative "typical inhabitant" opcodes from the per-doc prompt and confirmed several are intentionally documented on sibling pages, including binding queries, cooperative-vector helpers, and descriptor-heap opcodes.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### Kernel launch`, lines 239-242; `### CudaKernelLaunch`, lines 296-305 | The page says `CudaKernelLaunch` has `baseFn, gridDim, blockDim, argsArray, cudaStream` and "exactly five operands". The IR-reference contract says the operand column should come from the Lua entry, but the Lua declaration for `CudaKernelLaunch` names six operands: `kernel`, `gridDimX`, `gridDimY`, `gridDimZ`, `blockDimX`, and `blockDimY`. The watched `IRBuilder` helper does emit five operands, so the page should not present that helper shape as the opcode schema without explaining the source discrepancy. | `source/slang/slang-ir-insts.lua:2684` declares the `CudaKernelLaunch` opcode with six operands; `source/slang/slang-ir.cpp:3676` defines the five-argument `IRBuilder::emitCudaKernelLaunch` helper that currently emits the opcode. | Change the table's operand column to reflect the Lua declaration, and revise the notable callout to distinguish the Lua opcode schema from the five-argument builder helper. If the helper shape is the intended truth, call out the Lua mismatch for source remediation rather than documenting the helper as the opcode operand list. |
| F-002 | major | `## Notable opcodes`, lines 244-316 | The per-doc prompt requires a notable-opcode callout for `getStringHash`, but the section has callouts for `Each`, `PackBranch`, `MakeWitnessPack`, `IsType`, storage/logical casts, `CudaKernelLaunch`, and `Annotation` only. The opcode appears in the table, but the required notable discussion of its operand layout and stable-hash semantics is missing. | `docs/generated/design/_meta/prompts/ir-reference-misc.md:44` lists `getStringHash` under "Cover at least"; `source/slang/slang-ir-insts.lua:1530` declares the opcode with `stringLit: IRStringLit`. | Add a `### getStringHash` callout under `## Notable opcodes` that describes the `stringLit: IRStringLit` operand and the stable string-hash role, citing the Lua declaration and the relevant watched lowering/source helper if available. |

## No-issues notes
- The page correctly links `../pipeline/04-ast-to-ir.md` from `## See also`.
- The pack/expansion, type-query, storage-cast, annotation, liveness, and string-hash table rows checked against Lua are otherwise aligned.
- The prompt's no-duplicate concern is handled for sampled related opcodes: binding queries, descriptor heaps, cooperative-vector helpers, and per-vertex input are documented on sibling pages rather than duplicated here.
