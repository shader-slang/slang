---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:05:51+00:00
target_doc: ir-reference/index.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 9221d4167460d8aa57ead3a905a1ce4b763de1371a68088ad1f20133ed887720
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
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

# Review report for ir-reference/index.md

## Summary
The page has valid front matter and all relative links resolve at the target document source commit. I found two coverage and prompt-contract issues: the index claims exhaustive opcode coverage that the dependency pages do not currently provide, and `## Cross-cutting topics` omits the required target-backends cross-reference.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ir-reference/index.md`.
- Read `_common.md`, `ir-reference-index.md`, the target document, all listed dependency family pages, and `source/slang/slang-ir-insts.lua` at `52339028a2aa703271533454c6b9528a534bac31`.
- Resolved all 31 relative Markdown links at the target source commit.
- Checked front matter keys and target digest fields for obvious validity.
- Spot-checked the family taxonomy, the `## Pages` table, approximate opcode counts, and more than 10 source-backed claims about Lua roots, abstract entries, AST-origin conventions, and cross-reference targets.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | lines 12-21 and 87-93 | The page says every concrete opcode appears in a family page, but the dependency pages do not currently include all concrete opcodes from the recorded Lua source. | `source/slang/slang-ir-insts.lua:1534` declares `GetPerVertexInputArray` and `source/slang/slang-ir-insts.lua:1539` declares `ResolveVaryingInputRef`; the dependency family pages contain no opcode rows for either name. | Add the missing opcodes to their owning family page, then update this index's exhaustive wording and approximate counts to match the fixed family coverage. |
| F-002 | major | `## Cross-cutting topics` | The required target-backends cross-reference is missing from the cross-cutting topic list. | `docs/generated/design/_meta/prompts/ir-reference-index.md:51-56` requires bullets for target backends; this section links AST-to-IR, IR passes, emit, IR instructions, serialization, diagnostics, and glossary, but not `../cross-cutting/targets.md`. | Add a bullet for `../cross-cutting/targets.md` or otherwise include the target-backends cross-reference required by the prompt. |
