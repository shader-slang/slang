---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T21:10:43Z
target_doc: target-pipelines/index.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: ca52bc15faa8039821f1456579dee989310bf65674d817e74da5aba208126d04
source_commit: f40a5af1027e6fd524a5855f77d62bddffd998c9
checklist:
  factual_accuracy: partial
  cross_references: partial
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 0
  major: 2
  minor: 3
  nit: 0
---

# Review report for target-pipelines/index.md

## Summary

The document includes all required sections, but needs corrections before acceptance. Most importantly, its HLSL comparison cites a removed legalization function. It also overstates SPIR-V and CUDA exclusivity, misdescribes two peer pages’ headings, and includes pass-level explanations explicitly forbidden by the index contract.

## Items checked

- Reviewed against the CURRENT working tree; `git diff --quiet 2518a942d59b1228fc8cc70a37739baab82b6002 -- source/` returned success, confirming source equality with the document’s source commit.
- Read `_review.md`, `_common.md`, and the target’s own prompt; ran `regenerate.py show target-pipelines/index.md`.
- Verified all 12 body line-number citations: `slang-emit.cpp` at 409, 1015, 1085, 1592–1599, 1666, 1760, 2524, and 3774; `slang-code-gen.h` at 52; `slang-ir-spirv-legalize.cpp` at 3130–3165 and 3433; and `slang-emit-spirv.cpp` at 12469. All match.
- Spot-checked more than 20 claims, including target enums, downstream source selection, emitter inheritance, `precise` handling, legalization-driver placement, SPIR-V iteration counters, CUDA-specific gates, the 40 lowering flags, accumulating scans, autodiff branching, and tagged-union reinterpret gating.
- Resolved all 35 Markdown links, covering 24 distinct destinations, including the local heading anchor. Confirmed all 10 linked generated peer pages appear in the manifest.
- Read dependency-page excerpts only to check this index’s descriptions and structural cross-references.
- Checked required section order, all five comparison rows and six columns, required front-matter fields, and size: 14,376 bytes against the 32,768-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | Cross-target comparison, line 122 | The HLSL row cites `legalizeRayPayloadAccessQualifiersForHLSL`, which no longer exists in `source/slang`. It also places that removed function in `slang-ir-hlsl-legalize.cpp`. | `source/slang/slang-ir-hlsl-legalize.cpp:67` defines `validateBarrierFlagsForHLSL`. Payload legalization now uses `legalizeRayTracingPayloads`, defined in `source/slang/slang-ir-ray-tracing-legalize.cpp:884` and invoked from `source/slang/slang-emit.cpp:1989`. | Remove the obsolete example from the Phase C cell and retain `validateBarrierFlagsForHLSL`. The replacement payload pass belongs earlier in the pipeline, so do not substitute it into the Phase C cell. |
| F-002 | minor | Pages, lines 30–31; Cross-target comparison, line 121 | SPIR-V is described as `the only target with iterative passes`. CUDA also has an internal fixed-point loop, already documented by the linked CUDA page. The narrower claim about no repeated orchestrator invocation does not establish this exclusivity. | `source/slang/slang-ir-legalize-varying-params.cpp:1966` uses `for (bool changed = true; changed;)` to inline shader-terminating callees. `docs/generated/design/target-pipelines/cuda.md:904` explicitly describes this internal loop. | Remove both exclusivity claims. Keep the SPIR-V loop names and distinguish orchestrator-level repetition from loops inside individual passes. |
| F-003 | minor | Cross-target comparison, line 125 | `lowerImmutableBufferLoadForCUDA` is called `the one pass that exists solely for this target family`, despite other CUDA-only passes in the orchestrator. | `source/slang/slang-emit.cpp:2352–2358` selects `synthesizeActiveMask` only for CUDA source/header and PTX; lines 1902–1904 select `legalizeOptiXReportIntersectionsForCUDA` only for CUDA source/header. | Describe `lowerImmutableBufferLoadForCUDA` as an example of a CUDA-specific pass and delete the uniqueness claim. |
| F-004 | major | Pages, lines 28–61; Filtering rules, lines 181–223 | The index exceeds its navigation-only scope: it explains emitter details, autodiff decoration stripping and DCE, and tagged-union reinterpret production. These are per-pass details expressly excluded by its generation contract. | `docs/generated/design/_meta/prompts/_common.md:455` states: `Forbidden in the index: per-pass details`. `docs/generated/design/_meta/prompts/target-pipelines-index.md`, Page shape items 3 and 6, requires one-clause page descriptions and a short filtering paragraph. | Reduce each Pages bullet to a one-clause target/path summary. Replace the detailed IR-content discussion with a brief gating summary and links to the child pages; remove the autodiff and tagged-union explanations. Preserve the comparison table’s explicitly required SPIR-V loop caveat. |
| F-005 | minor | Shared shape, lines 106–109; Filtering rules, lines 198–200 | The claim that every peer’s Conditional gates section opens with a `requiredLoweringPassSet.* flags` subsection is false for WGSL and CUDA. | `docs/generated/design/target-pipelines/wgsl.md:872` and `docs/generated/design/target-pipelines/cuda.md:795` begin those sections with explanatory prose and have no subsection with that heading. HLSL, Metal, and SPIR-V do have it. | State that all five pages discuss lowering flags in Conditional gates, without claiming identical subsection headings. |
