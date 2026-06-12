---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:05:51+00:00
target_doc: ir-reference/misc.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 5ac7df35674b391db414495e8be54b9c8c58690cd2b324a3a4c6804a1748f586
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 1
  minor: 0
  nit: 0
---

# Review report for ir-reference/misc.md

## Summary
The page has valid front matter and all relative links resolve. The sampled opcode rows mostly match the recorded Lua declarations, but the catch-all coverage misses concrete opcodes under a grouping parent that the misc prompt explicitly calls out as a typical inhabitant.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ir-reference/misc.md`.
- Read `_common.md`, `ir-reference-misc.md`, the target document, dependency docs, and watched source files at `52339028a2aa703271533454c6b9528a534bac31`.
- Resolved all 25 relative Markdown links at the target source commit.
- Verified front matter keys and checked the target source commit and watched-path digest values against the document front matter.
- Spot-checked more than 10 factual claims across pack helpers, type queries, storage casts, liveness markers, string hashing, and kernel-launch rows.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Opcodes` | The catch-all table omits the concrete `ForceVarIntoStructTemporarily` and `ForceVarIntoRayPayloadStructTemporarily` opcodes even though the misc prompt names their grouping parent as a typical inhabitant and no sibling page lists the children. | `docs/generated/design/_meta/prompts/ir-reference-misc.md:14-18` names `ForceVarIntoStructTemporarilyBase`; `source/slang/slang-ir-insts.lua:1541-1548` declares the two concrete child opcodes. | Add rows for `ForceVarIntoStructTemporarily` and `ForceVarIntoRayPayloadStructTemporarily`, or move them to a better owning family page and cross-link from misc. |
