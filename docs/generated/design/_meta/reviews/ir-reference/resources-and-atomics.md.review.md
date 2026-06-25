---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T12:06:22+00:00
target_doc: ir-reference/resources-and-atomics.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 50a5584b2851342292d4b982e8c4767f3127bd44d5e4d4de95333b7b3e0e7fa5
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
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
  major: 1
  minor: 0
  nit: 0
---

# Review report for ir-reference/resources-and-atomics.md

## Summary
The page has valid front matter, required sections, and resolving relative links. The main issue is a source-alignment error in the sampling table and notable text: `sampleGrad` is described as variadic with `gradY` and optional trailing operands, but the watched Lua declaration exposes only four operands.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show ir-reference/resources-and-atomics.md`.
- Read `_common.md`, `ir-reference-resources-and-atomics.md`, the target document including front matter, dependency docs, and watched source files.
- Resolved the document's relative Markdown links and checked peer generated-doc links against the manifest-backed generated docs.
- Verified every `AtomicOperation` child row against the Lua declarations and checked the `IRMemoryOrder` enum in `slang-ir.h`.
- Spot-checked more than 10 additional claims across `imageSubscript`, `imageLoad`, `imageStore`, `ImageTexelPointer`, `SubpassLoad`, `sample`, `sampleGrad`, byte-address buffer loads/stores, structured-buffer operations, `nonUniformResourceIndex`, mesh-output ops, barriers, cooperative-vector ops, raytracing payload ops, descriptor-heap ops, and binding queries.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### Sampling and combined samplers` and `### sample and sampleGrad` | The `sampleGrad` row and notable text claim trailing operands such as `gradY`, offset, and bias, but the Lua schema for this opcode has exactly `texture, sampler, coord, gradX`. This makes the required operand-shape column inaccurate. | `source/slang/slang-ir-insts.lua:1526-1528` declares `sample` and `sampleGrad`, with `sampleGrad` operands limited to `texture`, `sampler`, `coord`, and `gradX`. | Change the `sampleGrad` operand cell and notable paragraph to match the Lua declaration, or add a caveat that additional gradient/offset semantics live in core-module intrinsics rather than as operands on this IR opcode. |
