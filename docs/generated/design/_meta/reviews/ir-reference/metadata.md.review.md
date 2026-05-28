---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-05-15T16:50:36+00:00
target_doc: ir-reference/metadata.md
target_doc_source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
target_doc_watched_paths_digest: 4efe93afbd22f4572d6d334ca82947cebf8058c7572291261103fd18aa04f6bd
source_commit: 2580ad341db243d8bd27edd0327f08a29be906b3
checklist:
  factual_accuracy: fail
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

# Review report for ir-reference/metadata.md

## Summary
The page is structurally lint-clean, but review found 2 findings; the most significant severity is major. The main remediation need is to align the page with watched source evidence and the per-page prompt contract before marking this review cycle complete.

## Items checked
- Checked Layout, Attr, Debug, SPIRVAsm tables, special asm opcode names, abstract-parent handling, front matter, and links.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | lines 171-175 | Several SPIR-V asm Opcode cells use C++ wrapper names instead of Lua opcode names. | `source/slang/slang-ir-insts.lua:2823-2858` defines Lua entries such as `__truncate`, `__entryPoint`, `__sampledType`, `__imageType`, and `__sampledImageType`. | Use Lua names in the Opcode column and move wrapper names to the C++ wrapper column. |
| F-002 | major | lines 111-116 and 158 | Abstract/grouping parents are listed as opcode rows: `SemanticAttr`, `LayoutResourceInfoAttr`, and `SPIRVAsmOperand`. | `source/slang/slang-ir-insts.lua:2685-2694` and `:2756-2862` show these as grouping entries with children. | Remove those rows from Opcodes and show them only in hierarchy. |
