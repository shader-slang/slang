---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:59:10Z
target_doc: cross-cutting/ir-instructions.md
review_report: ../../reviews/cross-cutting/ir-instructions.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 2
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/ir-instructions.md

## Summary
Both findings were verified against the watched Lua source and against the current target document. Each describes table content that does not exist in the document at the reviewed commit: the rows in question already use the correct Lua opcode names and operand spellings. Both findings are therefore rejected as bogus. No edits were made this cycle and the front-matter source commit is unchanged.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | The Value-instructions row at `docs/generated/design/cross-cutting/ir-instructions.md:138` already lists `integer_constant, float_constant, string_constant, ...` in the Opcode column and `IntLit, FloatLit, StringLit, ...` in the `struct_name` column, matching `source/slang/slang-ir-insts.lua:870,872,877`. The reviewer's claim that the Opcode column names the wrapper structs does not hold for the current text; the recommended change is already present. | — |
| F-002 | rejected-bogus | The Type-instructions row at `docs/generated/design/cross-cutting/ir-instructions.md:130` already names the opcode `TextureType` and spells the operand `accessOperand`, matching `source/slang/slang-ir-insts.lua:417,424`. No `Texture` opcode cell or `access` operand spelling exists in the document. | — |
