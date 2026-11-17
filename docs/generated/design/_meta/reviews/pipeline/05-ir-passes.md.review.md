---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T21:19:34Z
target_doc: pipeline/05-ir-passes.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 434c98b6b03bec03448f6e6a2edb7332984d1006ddfb79bd35d3933b4f25bc49
source_commit: be3c344414f6541ba6731813b36591a94cd05482
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: fail
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 2
  major: 1
  minor: 2
  nit: 0
---

# Review report for pipeline/05-ir-passes.md

## Summary

The inventory and pipeline citations are largely accurate, but two behavioral claims need correction. Most importantly, the copy-logical row incorrectly excludes GLSL, contradicting both the source and the document’s own phase table. The description of `-dump-ir`, AnyValue marshalling granularity, target-specific table coverage, and document length also need adjustment.

## Items checked

- Read `_review.md`, `_common.md`, the document-specific prompt, and the resolved watched paths from `regenerate.py show pipeline/05-ir-passes.md`.
- Verified against the **current working tree**. Its `source/` contents match the document’s recorded source commit; `git diff 2518a942d59b1228fc8cc70a37739baab82b6002 --stat -- source/` returned no differences.
- Checked every explicit line-number citation, including all 15 phase ranges, orchestrator definitions and callers, plain calls, SPIR-V legalization, metadata interfaces, and the coverage option.
- Spot-checked more than 10 behavioral and structural claims: the 184 wrapped calls, accumulating lowering flags, HostVM early return, uniformity gate and error exit, witness-lookup option, autodiff-context target gate, pre-link bit-field synthesis and loop inversion, SPIR-V legalization order, active-mask handling, copy-logical target gates, and AnyValue marshalling keys.
- Resolved all 208 relative-link occurrences (191 distinct targets); all 11 linked generated pages have manifest entries. Read the dependency page only for cross-reference checks.
- Confirmed all 167 `slang-ir-*.cpp` files appear in category tables.
- Checked required sections, table columns, front-matter fields, size limits, and Liquid safety. The document is 67,257 UTF-8 bytes and contains no raw Liquid openers.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Pass categories`, “Lower copy logical” row, line 356 | The claim `Its only call site is inside SPIR-V legalization, so it never runs for the source-language targets` is false. GLSL explicitly runs this pass; the phase table already records that correctly. | `source/slang/slang-emit.cpp:2653-2657` explains that GLSL lacks `OpCopyLogical` and calls `SLANG_PASS(lowerCopyLogical)` under `target == CodeGenTarget::GLSL`. `source/slang/slang-ir-spirv-legalize.cpp:2985-2994` also calls it for SPIR-V versions earlier than 1.4. | Replace the exclusivity claim with both call sites: GLSL post-link lowering and SPIR-V legalization when targeting versions below 1.4. |
| F-002 | critical | `## How the passes are ordered`, lines 65-70 | The assertion that `-dump-ir` shows `exactly the SLANG_PASS call sites of linkAndOptimizeIR and nothing else` incorrectly excludes the pre-link result snapshot. This misdirects readers debugging lowering. | `source/slang/slang-lower-to-ir.cpp:15846-15855` checks `shouldDumpIR()` and calls `dumpIR` with the label `"LOWER-TO-IR"` after the pre-link passes. | State that individual plain pre-link calls lack wrapped before/after headers, but their combined result appears in the `LOWER-TO-IR` snapshot. Retain the separate observation that the surrounding SPIR-V legalization dumps are disabled. |
| F-003 | minor | `### Specialization and generics`, “AnyValue marshalling” row, line 250 | `one ... function per size` gives the wrong generation granularity. Distinct concrete types sharing an AnyValue size can require separate pack/unpack pairs. | `source/slang/slang-ir-any-value-marshalling.cpp:78-89` keys functions by both `originalType` and `anyValueSize`; `:1325-1336` constructs that key before generating the pair. `:878` constructs a size-based name hint, not a unique per-size function identity. | Describe one pair per concrete-type/AnyValue-size combination. Clarify that size-based name hints can repeat and do not establish one shared pair per size. |
| F-004 | major | `### Target-specific lowering`, lines 473-486 | This category replaces the required `Purpose` column with `Target`. Several rows contain only a target name, leaving their transformation purpose undocumented. | `docs/generated/design/_meta/prompts/pipeline-05-ir-passes.md:44-49` requires a `Pass / File / Purpose` table for each category. Rows such as GLSL liveness, CUDA immutable load, and Vulkan invert Y provide only targets. | Restore a `Purpose` column and add a short behavioral description for every row. Preserve target information in an additional column or within each purpose cell. |
| F-005 | minor | Whole document | At 67,257 UTF-8 bytes, the document exceeds the per-document prompt’s `under 64 KB` limit, even interpreting KB as 65,536 bytes. It remains below the manifest’s separate 81,920-byte cap. | `docs/generated/design/_meta/prompts/pipeline-05-ir-passes.md:72-74` sets the stricter limit and directs the author to summarize when necessary. | Shorten repetitive explanations and long table cells until the document is below 64,000 bytes, satisfying either interpretation without removing required sections. |