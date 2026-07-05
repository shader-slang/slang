---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:01:27Z
target_doc: ir-reference/types.md
review_report: ../../reviews/ir-reference/types.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/types.md

## Summary
All three findings were verified against the watched sources at HEAD and resolved in the target document. F-001 corrected the overstated "hoistable throughout" claim in `## Source`; F-003 filled in the `TorchTensor` operands cell; F-002 reworked the `## Family hierarchy` diagram to use the actual immediate Lua subgroup names as children of `Type`, and I completed it this cycle by adding the missing `TranslatedTypeBase` group node so every immediate Lua intermediate group appears. No findings were rejected, deferred, or escalated. The operator refreshes `source_commit` via `mark-fresh`; `_after` is HEAD since the body changed.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `slang-ir-insts.lua` line 149 `Enum` `parent=true` only; lines 662/666 `struct`/`class` `parent=true`; line 668 `interface` `global=true`; `AfterBaseType` (line 42) and `MakeTensorAddressing*` (lines 642-643) unflagged. `slang-ir.h` lines 51-56 define `Parent`/`Hoistable`/`Global` as distinct flags; "hoistable throughout" overstated the invariant. | Source para now says most leaf opcodes are hoistable plus an exceptions clause naming the parent/global/unflagged entries and deferring to the Flags column. |
| F-002 | fixed | Prompt `ir-reference-types.md` lines 47-49 and `_common.md` line 223 require the diagram to show every immediate Lua subgroup as a child of `Type`. The immediate intermediate groups in the Lua are the 18 `*TypeBase`/group entries (`BasicType` ... `WitnessTableTypeBase`), including `TranslatedTypeBase` (line 168), interleaved with concrete leaves. | Diagram first level uses the actual Lua subgroup node names; this cycle added the missing `Type --> TranslatedTypeBase` edge so all 18 immediate groups are present. |
| F-003 | fixed | `slang-ir.cpp` lines 3001-3004 `getTorchTensorType(IRType* elementType)` creates `kIROp_TorchTensorType` with one operand, though the Lua entry (line 222) omits an explicit `operands` field. | TorchTensor row operands cell reads `elementType: IRType`. |
