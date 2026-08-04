---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:26:03+00:00
target_doc: cross-cutting/ir-instructions.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 4d00bbdf4b0ae52ef1c205b4c4aa6ce08a50ed9d00f0d7e09bdaa67a56261b9d
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
  major: 2
  minor: 0
  nit: 0
---

# Review report for cross-cutting/ir-instructions.md

## Summary
The page mostly follows the prompt contract and its links/front matter look valid, but two summary-table entries name opcode identifiers that do not exist in the watched Lua source. Because the prompt specifically requires every listed opcode to appear in `slang-ir-insts.lua`, those rows should be corrected before marking the review cycle clean.

## Items checked
- Ran `regenerate.py show cross-cutting/ir-instructions.md` and reviewed the target document, the common contract, its per-document prompt, and the resolved watched-file set (`source/slang/slang-ir-insts.h`, `source/slang/slang-ir-insts.lua`, `source/slang/slang-ir.cpp`, `source/slang/slang-ir.h`).
- Checked front matter for all required keys, the recorded source commit, the warning string, and a 64-character hex watched-path digest.
- Reviewed the page against its per-document prompt and common generated-doc contract, including the required `## Source`, `## Schema`, `## Instruction families`, `## Hoistable / global / deduplicated values`, and `## Decorations` sections.
- Checked relative markdown links in the document body for workspace-relative shape and peer generated-doc targets.
- Spot-checked more than 10 concrete source-backed claims: `Vec`, `Mat`, `MetalPackedVec`, `Array`, `Ptr`, `TextureType`, `builtinRequirementKey`, `kIROpFlag_Hoistable`, `kIROpFlag_Global`, `IROpMeta::kIROpMeta_OtherShift`, `k_minSupportedModuleVersion`, `k_maxSupportedModuleVersion`, `NameHintDecoration`, `EntryPointDecoration`, `witness_table_entry`, and `getBuiltinRequirementKey`.
- Confirmed the page has no markdown source line-anchor citations requiring exhaustive line-anchor verification.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### Value instructions` table, lines 136-143 | The row lists `IntLit`, `FloatLit`, and `StringLit` under the `Opcode` column, but those are wrapper struct names, not the Lua opcode names. The prompt's quality checklist says every opcode listed must appear in `slang-ir-insts.lua`. | `source/slang/slang-ir-insts.lua:870` declares `integer_constant = { struct_name = "IntLit" }`; `source/slang/slang-ir-insts.lua:872` declares `float_constant = { struct_name = "FloatLit" }`; `source/slang/slang-ir-insts.lua:877` declares `string_constant = { struct_name = "StringLit" }`. | Change the opcode cell to the actual Lua names, such as `integer_constant`, `float_constant`, and `string_constant`, while keeping `IntLit`, `FloatLit`, and `StringLit` in the `struct_name` column. |
| F-002 | major | `### Type instructions` table, lines 122-132 | The resource-type row lists opcode `Texture`, but the watched Lua source declares `TextureType`; no `Texture` opcode entry appears in the checked source. | `source/slang/slang-ir-insts.lua:417` declares `TextureType = {`, with operands `elementType`, `shape`, `isArray`, `isMS`, `sampleCount`, `accessOperand`, `isShadow`, `isCombined`, and `format` on lines 418-427. | Rename the row's opcode cell from `Texture` to `TextureType` and, if desired, rename the operand `access` to the source spelling `accessOperand`. |

## No-issues notes
- The document keeps generated-doc links workspace-relative and avoids absolute source paths.
- The page stays within the documented scope for its family and does not copy handwritten documentation prose.
- The source citations are concentrated in the watched paths listed by the manifest entry.
