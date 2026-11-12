---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:09:49Z
target_doc: ir-reference/metadata.md
review_report: ../../reviews/ir-reference/metadata.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/metadata.md

## Summary

All four minor findings were verified against the current source and fixed in the document. No findings were rejected, deferred, or escalated. The document grew by under 1 KB and stays far below its 65,536-byte cap.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-lower-to-ir.cpp:16185-16188` lowers the element layout recursively, so a struct element gets `structTypeLayout`. | Example changed to `RWStructuredBuffer<float>`; general `T` described as recursively lowered. |
| F-002 | fixed | `source/slang/slang-ir-insts.lua:2913-3160` declares only `min_operands` for most of these opcodes; names come from accessors. | Added `†` to 37 accessor-named Operands cells and a legend in the Opcodes intro. |
| F-003 | fixed | `source/slang/slang-ir.cpp:7913` returns false in `Detailed` mode before the folding rules. | Folding claims qualified as ordinary dumps; Detailed mode noted. |
| F-004 | fixed | `source/slang/slang-ir-insts.lua:3062` declares `SPIRVAsmOperand`; `source/slang/slang-lower-to-ir.cpp:5594` is `visitSPIRVAsmExpr`. | Citations 3020 -> 3062 and 6150 -> 5594. |
