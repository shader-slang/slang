---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:05:36+00:00
target_doc: ir-reference/decorations.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 156c66694255ff678fb0eaa18abd2bb50bfa9979d070210c7bb229025fcd0b6b
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
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

# Review report for ir-reference/decorations.md

## Summary
The decoration catalog is broadly aligned with the Lua `Decoration` family, but it misses one required notable-opcode callout group from the per-page prompt. The opcode tables themselves include the affected rows, so the issue is limited to the explanatory `## Notable opcodes` section.

## Items checked
- Ran `regenerate.py show ir-reference/decorations.md` and checked the resolved watched files and dependencies.
- Verified front matter keys, target source commit, watched-path digest shape, 15 relative links, required IR-reference sections, and table columns.
- Checked decoration rows across naming/provenance, layout/binding, loop and branch hints, target-specific intrinsics, capability, IO, entry-point, linkage, optimization, specialization, autodiff, SPIR-V hint, debug/reflection, and other buckets.
- Spot-checked more than 10 factual claims against `slang-ir-insts.lua`, `slang-ir-insts-info.cpp`, `slang-ir-insts.h`, `slang-ir.h`, `slang-ir.cpp`, and `slang-lower-to-ir.cpp`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Notable opcodes`, lines 339-413 | The required callouts for `branch`, `flatten`, and `loopControl` are absent from `## Notable opcodes`; listing those opcodes in the table does not satisfy the prompt's notable-opcode requirement. | `docs/generated/design/_meta/prompts/ir-reference-decorations.md:66-70` requires `branch`, `flatten`, and `loopControl`; `source/slang/slang-ir-insts.lua:1636-1642` defines those decorations. | Add a short notable subsection for `branch`, `flatten`, and `loopControl`, explaining that they are control-flow hints flowing to backend emission. |
