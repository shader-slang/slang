---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:58:30+00:00
target_doc: glossary.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 3ec0150df965938276956bf39ea384534b1949417b8bac7832d9895d4ccada91
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 0
  minor: 3
  nit: 0
---

# Review report for glossary.md

## Summary
The glossary covers the required term floor and all workspace-relative links resolve, but review found 3 minor issues. The most substantive issue is that the `hoistable instruction` definition overstates placement at module scope, while the source hoists only as far as operands permit. Two additional prompt-contract issues cover adjacent ordering and an `External:` link on a `[Slang]` entry.

## Items checked
- Ran `regenerate.py show glossary.md` and used the listed prompt, dependencies, and watched files.
- Verified required front-matter keys, source commit and watched digest shape, first paragraph, size cap, and required sections.
- Resolved all 190 workspace-relative markdown links at target source commit `52339028a2aa703271533454c6b9528a534bac31`.
- Checked all 66 glossary entries for exactly one `[Slang]` or `[General]` tag and a mandatory `See:` link.
- Spot-checked 20 source-alignment claims, including `ASTBuilder`, `NodeBase`, conversion costs, `DeclRef`, `LookupMask`, `LookupResultItem_Breadcrumb`, `generateIRForTranslationUnit`, layout IR, visibility, and `TranslationUnitRequest`.
- Checked the floor terms, external URL format, language-reference glossary duplication, and cross-reference index coverage.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Terms` entry `hoistable instruction` | The definition says hoistable instructions are deduplicated globally inside an `IRModule` and live at module scope. The implementation starts from module scope but explicitly backs off when operands are defined in deeper parents, so module scope is not guaranteed. | `source/slang/slang-ir.cpp` lines 1645-1654 say the module-scope assumption may be invalid when operands are nested, and lines 1677-1706 choose the earliest valid point in the selected parent. | Rephrase the entry to say hoistable instructions are deduplicated and moved as far toward module scope as their operands allow. |
| F-002 | minor | `## Terms` near `target` entries | The glossary is not strictly alphabetically ordered: `target legalization driver` appears before `target intrinsic`, but `target intrinsic` should sort first. | `docs/generated/design/_meta/prompts/glossary.md` lines 7-9 require an alphabetically ordered glossary. | Move the `target intrinsic` entry before `target legalization driver` and re-run the ordering check. |
| F-003 | minor | `## Terms` entry `scope` | The `scope` entry is tagged `[Slang]` but includes an `External:` link. The glossary prompt allows `External:` only on `[General]` entries. | `docs/generated/design/glossary.md` lines 554-564 show `scope` tagged `[Slang]` with an `External:` line; `docs/generated/design/_meta/prompts/glossary.md` lines 76-83 reserve `External:` for `[General]` entries. | Remove the `External:` line from `scope` or retag the entry as `[General]` if the definition is rewritten as a general compiler term. |
