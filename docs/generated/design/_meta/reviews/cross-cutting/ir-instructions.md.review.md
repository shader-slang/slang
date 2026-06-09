---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:58:30+00:00
target_doc: cross-cutting/ir-instructions.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 2e16e4101249030a9cd977caed2ab437622083f3808b536a0a0e81f0c47cf487
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

# Review report for cross-cutting/ir-instructions.md

## Summary
The page is mostly aligned with the IR source and all workspace-relative links resolve, but review found 2 findings. The most important issue is that the prompt requires a top-level `## Decorations` section, while the page only has a `### Decorations` family subsection. The summary tables also use several display names that are not actual Lua opcode entries.

## Items checked
- Ran `regenerate.py show cross-cutting/ir-instructions.md` and used the listed prompt, dependency, and watched files.
- Verified required front-matter keys, source commit and watched digest shape, first paragraph, size cap, and required heading structure.
- Resolved all 42 workspace-relative markdown links at target source commit `52339028a2aa703271533454c6b9528a534bac31`.
- Spot-checked 18 source-alignment claims against `slang-ir-insts.lua`, `slang-ir-insts.h`, `slang-ir.h`, and `slang-ir.cpp`, including schema fields, flag bits, opcode families, module-version warning, and add-opcode workflow.
- Checked representative opcode rows for type, value, memory, control-flow, structure, existential, decoration, and resource families.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | after `## Hoistable / global / deduplicated values` | The page omits the required top-level `## Decorations` section. A `### Decorations` subsection exists under instruction families, but the prompt specifically requires a separate section that notes decoration opcodes and avoids over-claiming the migration story. | `docs/generated/design/_meta/prompts/cross-cutting-ir-instructions.md` lines 56-60 require `## Hoistable / global / deduplicated values` followed by `## Decorations`. | Add a top-level `## Decorations` section or rename and move the existing decoration discussion so it satisfies the required structure. |
| F-002 | major | `### Type instructions` and `### Control-flow instructions` | Several representative rows list names that are not Lua opcode entries: `vector`, `matrix`, `branch`, and `condBranch`. This violates the prompt rule that every listed opcode must appear in `slang-ir-insts.lua`. | `source/slang/slang-ir-insts.lua` lines 103-113 declare `Vec` and `Mat` with wrappers `VectorType` and `MatrixType`; lines 1327-1351 declare `unconditionalBranch` and `conditionalBranch`. | Replace those cells with the actual opcode names, for example `Vec`, `Mat`, `unconditionalBranch`, and `conditionalBranch`, or explicitly label them as prose aliases outside the Opcode column. |
