---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:16:39+00:00
target_doc: target-pipelines/index.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 88909e4def1133ca5cd3ccb36f17d01f8bcc633abff88b21acd9208e1a05d1f2
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: pass
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 0
  minor: 1
  nit: 0
---

# Review report for target-pipelines/index.md

## Summary
The index has the required navigation sections, links all five peer target pages, and its cross-target facts match the peer pages and spot-checked source. One contract issue remains: two comparison-table cells expand the HLSL and CUDA "no single entry" cases into individual pass names, even though the index prompt requires compact exact labels and forbids per-pass detail.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show target-pipelines/index.md` and used the target front matter source commit and watched-path digest in this report.
- Read the target document, `_common.md`, `target-pipelines-index.md`, and all five dependency peer docs: `spirv.md`, `hlsl.md`, `metal.md`, `wgsl.md`, and `cuda.md`.
- Checked every relative link visible in the index, including the five peer-page links, source-file links, prompt link, and See also links; no dangling links were found.
- Verified the required index sections, table column order, peer-page coverage, front matter keys, and size cap.
- Spot-checked more than 10 factual claims against peer docs and source, including `linkAndOptimizeIR` at `source/slang/slang-emit.cpp:895`, emitter construction for HLSL/CUDA/Metal/WGSL, SPIR-V direct emit, CUDA existential/resource legalization gating, the CUDA/Metal `undoParameterCopy` arm, SPIR-V loop bounds, downstream tool summaries, and the five loop-summary cells.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Cross-target comparison`, HLSL and CUDA rows | The Phase C entry cells include pass-level inventories: `per-pass HLSL arms: wrapStructuredBuffersOfMatrices, legalizeNonStructParameterToStructForHLSL, ...` and `per-pass CUDA arms: synthesizeActiveMask, legalizeEntryPointVaryingParamsForCUDA, ...`. The index prompt asks for compact exact entries for these rows and the index contract forbids per-pass details. | `docs/generated/design/_meta/prompts/target-pipelines-index.md:46-54` says to use `(no single entry; per-pass HLSL arms)` and `(no single entry; per-pass CUDA arms)`. `docs/generated/design/_meta/prompts/_common.md:412-414` says the index must not include per-pass details or duplicated target-page content. | Replace the HLSL and CUDA Phase C entry cells with the exact compact labels from the prompt; leave the detailed pass names on the peer target pages. |
