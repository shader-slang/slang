---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:27:15+00:00
target_doc: ir-reference/metadata.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 1f43055876c44da0b02f3e913b22e16142720f3f452022bd3fdfcf1687346392
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for ir-reference/metadata.md

## Summary
The metadata page is mostly source-aligned, but it misses one required cross-reference from the IR-reference family contract. The `## See also` section omits the AST-to-IR lowering page even though the common contract requires IR-reference pages to link it.

## Items checked
- Ran `regenerate.py show ir-reference/metadata.md` and reviewed the target document, `_common.md`, `ir-reference-metadata.md`, `cross-cutting/ir-instructions.md`, and `pipeline/04-ast-to-ir.md`.
- Checked front matter for all required generated-doc keys, the target source commit, the warning string, and a 64-character hex watched-path digest.
- Resolved the relative links in the document body to watched source files and generated peer docs, including `decorations.md`, `types.md`, `../pipeline/05-ir-passes.md`, and `../pipeline/06-emit.md`.
- Verified the required IR-reference sections: `## Source`, `## Family hierarchy`, `## Opcodes`, `## Notable opcodes`, and `## See also`.
- Spot-checked more than 10 claims against watched files, including `Layout`, `varLayout`, `typeLayout`, `ptrTypeLayout`, `Attr`, `stage`, `userSemantic`, `DebugLine`, `DebugValue`, `SPIRVAsmOperandInst`, `SPIRVAsmOperandResult`, and the `IRBuilder::emitDebugValue` helper.
- Checked the Lua line-range claims for the `Layout`, `Attr`, `Debug*`, `SPIRVAsm`, and `SPIRVAsmOperand` groups.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## See also`, lines 256-272 | The `## See also` section omits the lowering pipeline page. The IR-reference family contract requires every family page's see-also list to include the lowering pipeline page, but this page links pipeline `05` and `06` without linking `../pipeline/04-ast-to-ir.md`. | `docs/generated/design/_meta/prompts/_common.md:251` requires the see-also list to link "the lowering pipeline page"; the manifest also lists `pipeline/04-ast-to-ir.md` as a dependency for this doc. | Add a `## See also` bullet for `../pipeline/04-ast-to-ir.md` describing it as the AST-to-IR lowering stage that introduces the few metadata opcodes with direct lowering origins. |

## No-issues notes
- The four metadata subfamilies in the hierarchy match the prompt: `Layout`, `Attr`, debug info, and `SPIRVAsmOperand`.
- The opcode rows checked against `slang-ir-insts.lua` match the Lua names, operand shapes, and hoistable/parent flags.
- The notable callouts cover the required `Layout`, `varLayout`, `DebugLine`, `DebugScope`, and SPIR-V inline-asm operand topics.
