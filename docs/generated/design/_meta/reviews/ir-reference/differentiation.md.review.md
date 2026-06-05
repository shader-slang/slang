---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T15:05:36+00:00
target_doc: ir-reference/differentiation.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 5ac7df35674b391db414495e8be54b9c8c58690cd2b324a3a4c6804a1748f586
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 0
  minor: 1
  nit: 0
---

# Review report for ir-reference/differentiation.md

## Summary
The differentiation page covers the required opcode families and its links and front matter are valid. One minor wording error remains in the `## Source` section: it says there are two checkpointing opcodes while naming and documenting three.

## Items checked
- Ran `regenerate.py show ir-reference/differentiation.md` and checked the resolved watched files and dependencies.
- Verified front matter keys, target source commit, watched-path digest shape, 10 relative links, required IR-reference sections, and table columns.
- Checked all 37 opcode table rows against `slang-ir-insts.lua`, including the three differential-pair base groups, `TranslateBase` leaves, autodiff temporaries, `DiffTypeInfo`, checkpointing opcodes, and `detachDerivative`.
- Spot-checked more than 10 factual claims about operand counts, hoistable flags, wrappers, AST origins, and notable opcode behavior against `slang-ir-insts.lua`, `slang-ir-insts.h`, `slang-ir.h`, `slang-ir.cpp`, and `slang-lower-to-ir.cpp`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Source`, lines 33-36 | The text says `Two additional opcodes` but then names three checkpointing opcodes: `checkpointObj`, `ReportCheckpointStore`, and `loopExitValue`. | `source/slang/slang-ir-insts.lua:1479-1487` defines `checkpointObj`, `loopExitValue`, and `ReportCheckpointStore` in that value-producing range. | Change `Two additional opcodes` to `Three additional opcodes` or rephrase the sentence to avoid the count. |
