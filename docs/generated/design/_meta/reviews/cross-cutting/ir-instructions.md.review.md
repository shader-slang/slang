---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T08:21:16Z
target_doc: cross-cutting/ir-instructions.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 70e71f05715d4cfb8c9d7012181f6823a6a06933c58c0925647a008f9c72bb91
source_commit: 801b9e6e57d41f5cef1a5d54c0eeff9e0db4fee6
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 1
  major: 0
  minor: 2
  nit: 1
---

# Review report for cross-cutting/ir-instructions.md

## Summary

The document covers the required sections and generally matches the implementation. Its most important error conflates native vector spelling with padded buffer storage, incorrectly extending Metal’s native-vector padding behavior to every other target. Three smaller findings concern the stable-name checker, a definition’s location, and oversized table notes.

## Items checked

- Reviewed against the **current working tree**, which the operator states equals the document’s source commit for all of `source/`; no files were written.
- Read `_review.md`, `_common.md`, and the document-specific prompt obtained through `regenerate.py show`; checked required sections, table columns, audience, and the 48 KB cap. The document is 31,201 bytes.
- Checked all nine explicit line-number citations: `slang-ir.cpp` lines 4078, 4291, 7605, 9985; `slang-lower-to-ir.cpp` lines 6049, 6314; `slang-ir-insts.lua` line 1457; `slang-ir-lower-expand-type.cpp` line 107; and `slang-ir-insts.h` line 2861.
- Spot-checked more than 15 claims, including wrapper generation, schema defaults, flag mapping, pointer and texture operands, opcode masking, intrinsic producers, array initialization, generic returns, expand yields, unused `emitAlloca`, set canonicalization, module-owned deduplication, module version 37, and optional debug-function parent scopes.
- Verified stable-name serialization, all 882 stable-name assignments and maximum ID 909, and both checker workflows.
- Resolved all 70 Markdown links, covering 33 distinct destinations; confirmed linked generated pages have manifest entries. Consulted the architecture dependency only for cross-reference context.
- Checked required front-matter keys and formats, relative-link style, and Liquid safety.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | Type instructions, line 179 | The `MetalPackedVec` row says every other target “keeps the padded vector,” explicitly including HLSL. Native `float3` spelling does not imply padded structured-buffer storage. The unconditional SPIR-V stride claim also omits supported layout options. This can mislead readers implementing buffer ABI layouts. | `source/slang/slang-ir-lower-buffer-element-type.cpp:2501` selects natural layout for non-Khronos, non-LLVM targets; line 2541 selects natural layout with scalar-layout options. `source/slang/slang-ir-layout.cpp:681` computes natural vector layout as `IRSizeAndAlignment(element.size * count, element.alignment)`, giving three floats size 12 and alignment 4. | Retain the Metal packed-versus-local distinction. Remove “every other target keeps the padded vector”; state that HLSL retains `float3` spelling without implying 16-byte storage, and qualify SPIR-V `ArrayStride 16` as the default layout case rather than a universal result. |
| F-002 | minor | Adding a new opcode, lines 439–440 | Saying `check` “verifies that the two files are a bijection” overstates its acceptance criteria. Retired stable-name entries are permitted, so successful checking does not establish a bijection. | `extras/check-ir-stable-names.lua:229` reports extra entries as `WARNING`; line 276 returns failure only when `has_errors` is set. Missing names and duplicate IDs set that flag; extra entries do not. | Say that `check` requires every live opcode to have a stable name and all IDs to be unique, while allowing retired entries with warnings. |
| F-003 | minor | Decorations, lines 354–356 | The prose locates the `IRDecoration` wrapper in `slang-ir.h`, which only forward-declares it. | `source/slang/slang-ir.h:140` contains `struct IRDecoration;`; its definition, `struct IRDecoration : IRInst`, is in `source/slang/slang-ir-insts.h:35`. | Change the definition-site link to `slang-ir-insts.h`. |
| F-004 | nit | Instruction families, especially lines 179, 193, 201, 224 | Several Notes cells contain extended explanations and examples rather than the required short phrase. The `MetalPackedVec` and `generic` rows are particularly difficult to scan as catalog entries. | `docs/generated/design/_meta/prompts/cross-cutting-ir-instructions.md`, Required structure item 4: “The `Notes` column is one short phrase.” | Condense these cells to brief summaries and links. Place any essential retained explanation in prose following the existing tables, without adding new sections. |