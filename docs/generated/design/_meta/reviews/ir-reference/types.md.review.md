---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:32:56+00:00
target_doc: ir-reference/types.md
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
finding_count: 3
severity_breakdown:
  critical: 0
  major: 2
  minor: 1
  nit: 0
---

# Review report for ir-reference/types.md

## Summary
The page is broadly useful and most checked opcode rows match the watched sources, but this review found three issues. The most important is that the Source section says the Type family is "hoistable throughout", while the Lua definition includes parent/global and unflagged type-family entries such as `Enum`, `struct`, `class`, `interface`, and tensor-addressing helper opcodes.

## Items checked
- Ran `regenerate.py show ir-reference/types.md` and reviewed the target document, the common contract, its per-document prompt, and the resolved watched-file set (`source/slang/slang-ir-insts.h`, `source/slang/slang-ir-insts.lua`, `source/slang/slang-ir.cpp`, `source/slang/slang-ir.h`, `source/slang/slang-lower-to-ir.cpp`).
- Checked front matter for all required keys, the recorded source commit, the warning string, and a 64-character hex watched-path digest.
- Resolved the generated peer-doc links in `See also`, the `docs/design/ir.md` link, and the source links that are named in the page body.
- Verified the required IR-reference sections: Source, Family hierarchy, Opcodes, Notable opcodes, See also.
- Spot-checked more than 10 concrete source-backed claims against the watched files, including the `Type` family range, `IRType : IRInst`, the IR op flags, `BasicType` children, `AnyValueType(size)`, `Array` and `UnsizedArray` operands, `Func(resultType, paramTypes...)`, `Vec`, `Mat`, `MetalPackedVec`, pointer operands, texture operands, interface/global flags, witness-table types, and set-theoretic type entries.
- Compared the family hierarchy and opcode tables against the Lua nesting and operands in `source/slang/slang-ir-insts.lua`, with helper cross-checks in `source/slang/slang-ir.cpp` and declarations in `source/slang/slang-ir-insts.h`.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Source`, lines 24-30 | The page says the Type family is "hoistable throughout" and that identical types deduplicate to one IR value. That overstates the invariant: several Type-family entries are not hoistable, including `Enum` (`parent = true` only), `struct` / `class` (`parent = true`), `interface` (`global = true`), `AfterBaseType`, and `MakeTensorAddressingTensorLayout` / `MakeTensorAddressingTensorView` with no hoistable flag. | `source/slang/slang-ir-insts.lua` lines 149, 642-668; `source/slang/slang-ir.h` lines 51-56 define `Parent`, `Hoistable`, and `Global` as distinct flags. | Change the Source paragraph to say that most leaf type opcodes are hoistable/deduplicated, while parent/global/container and helper entries are exceptions; point readers at the Flags column for per-opcode truth. |
| F-002 | major | `## Family hierarchy`, lines 45-94 | The hierarchy diagram does not mirror the immediate Lua subgroups required by the prompt. It introduces summary nodes such as `Resource family`, `Differentiation types`, `Existential / interface`, `Struct / class / interface containers`, `Tuple / pack / target-tuple`, and `Set-theoretic types`, while omitting exact immediate Lua groups such as `SamplerStateTypeBase`, `ResourceTypeBase`, `UntypedBufferResourceType`, `HLSLPatchType`, `BuiltinGenericType`, `TupleTypeBase`, and `WitnessTableTypeBase`. | `docs/generated/design/_meta/prompts/ir-reference-types.md` lines 47-49 require the diagram to show immediate Lua subgroups as children of `Type`; `source/slang/slang-ir-insts.lua` lines 380, 409, 440, 457, 475, 687, and 737 show immediate subgroup names not present in the diagram. | Rework the diagram so the first level under `Type` uses the Lua subgroup names, and only use friendlier labels inside or after those exact subgroup nodes. |
| F-003 | minor | `## Opcodes` / `### Tensor and torch-tensor types`, line 223 | The `TorchTensor` row lists operands as `—`, but the watched builder helper takes an `IRType* elementType` and creates `kIROp_TorchTensorType` with one operand. | `source/slang/slang-ir.cpp` lines 3001-3004: `IRBuilder::getTorchTensorType(IRType* elementType)` calls `getType(kIROp_TorchTensorType, 1, (IRInst**)&elementType)`. | Change the `TorchTensor` operands cell to `elementType: IRType` or add a short note if the Lua entry intentionally omits the operand schema. |

## No-issues notes
- The document keeps generated-doc links workspace-relative and avoids absolute source paths.
- The front matter has all required keys, and the target source commit and watched-path digest match the reviewed document.
- The checked rows for `BasicType`, `Array`, `UnsizedArray`, `Func`, `Vec`, `Mat`, `Ptr`, `TextureType`, `BindExistentials`, `BoundInterface`, rate/kind opcodes, and witness-table types matched the watched source files.
