---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:34:52+00:00
target_doc: ir-reference/structure.md
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
  major: 1
  minor: 1
  nit: 0
---

# Review report for ir-reference/structure.md

## Summary
The page has the required IR-reference structure and its front matter and links look valid, but the checked source sample found two issues. The main factual problem is the `generic` callout: the document says a generic ends with `yield`, while the source comment and helper code use an `IRReturn` terminator for generic results.

## Items checked
- Ran `regenerate.py show ir-reference/structure.md` and reviewed the target document, the common contract, its per-document prompt, and the resolved watched-file set (`source/slang/slang-ir-insts.h`, `source/slang/slang-ir-insts.lua`, `source/slang/slang-ir.cpp`, `source/slang/slang-ir.h`, `source/slang/slang-lower-to-ir.cpp`).
- Checked front matter for all required keys, the recorded source commit, the warning string, and a 64-character hex watched-path digest.
- Inspected the 27 relative markdown links in the document body and verified they point at workspace files or generated peer documents.
- Verified required IR-reference sections: `## Source`, `## Family hierarchy`, `## Opcodes`, `## Notable opcodes`, and `## See also`.
- Spot-checked more than 10 source-backed claims and named symbols against the watched files, including `GlobalValueWithCode`, `GlobalValueWithParams`, `func`, `generic`, `global_var`, `global_param`, `globalConstant`, `key`, `builtinRequirementKey`, `witness_table`, `indexedFieldKey`, `thisTypeWitness`, `TypeEqualityWitness`, `global_hashed_string_literals`, `module`, `call`, `witness_table_entry`, `interface_req_entry`, and `field`.
- Checked the prose line-number ranges in `## Source` against the Lua opcode clusters around structural global values and the later `call` / `witness_table_entry` / `interface_req_entry` / `param` / `field` cluster.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### generic`, lines 173-183 | The callout says each `generic` block `ends with a \`yield\``, but generic results are represented with `return_val` / `IRReturn` in this source revision. The `yield` opcode exists, but the lowering use found in the watched source is for `ExpandExpr`, not the generic body result path. | `source/slang/slang-ir-insts.lua:809`-`811` says a generic "ends with a `return` instruction"; `source/slang/slang-ir.cpp:9743`-`9754` implements `findGenericReturnVal` by casting the last block terminator to `IRReturn`; `source/slang/slang-lower-to-ir.cpp:6447` shows `emitYield` used for `ExpandExpr` lowering, not generic termination. | Change the callout to say a `generic` has a single block that ends with `return_val` / `IRReturn`, and avoid naming `yield` as the generic terminator. |
| F-002 | minor | `## Opcodes`, C++ wrapper column | Several rows use `—` in the C++ wrapper column even though the watched header declares wrapper structs for those opcodes. This makes the mandatory wrapper column incomplete for global and witness-table structure rows. | `source/slang/slang-ir-insts.h:2188` declares `IRGlobalParam`, `source/slang/slang-ir-insts.h:2202` declares `IRGlobalConstant`, `source/slang/slang-ir-insts.h:2212` declares `IRWitnessTableEntry`, and `source/slang/slang-ir-insts.h:2228` declares `IRWitnessTable`; the corresponding table rows currently show `—`. | Fill the wrapper cells for at least `global_param`, `globalConstant`, `witness_table`, and `witness_table_entry` with the declared wrapper names. |

## No-issues notes
- The document keeps generated-doc links workspace-relative and avoids absolute source paths.
- The page stays within the documented scope for its family and does not copy handwritten documentation prose.
- The source citations are concentrated in the watched paths listed by the manifest entry.
