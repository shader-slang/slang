---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:17:04+00:00
target_doc: target-pipelines/wgsl.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: f7ebb6018661b63fb04f0c5c697661718fe7c83752a8fdc750e6914dfeb10700
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 1
  major: 2
  minor: 0
  nit: 0
---

# Review report for target-pipelines/wgsl.md

## Summary

The WGSL target-pipeline page mostly tracks the ordered `linkAndOptimizeIR` sequence and distinguishes source emit from the Tint downstream path. The most important issue is a contradicted description of `floatNonUniformResourceIndex`: the document says WGSL preserves a textual marker for Tint to forward, while the source says WGSL drops the wrapper because WGSL has no such annotation. I also found two table-contract issues in the phase tables.

## Items checked

- Verified the target document front matter against the current digest for `target-pipelines/wgsl.md`.
- Ran `regenerate.py show target-pipelines/wgsl.md` and checked the listed watched files plus the downstream transition code used by the WGSLSPIRV paths.
- Spot-checked more than 20 source claims and line-number claims across `slang-emit.cpp`, `slang-ir-wgsl-legalize.cpp`, `slang-emit-wgsl.cpp`, `slang-emit-c-like.cpp`, `slang-code-gen.cpp`, and `slang-global-session.cpp`.
- Checked the ordered WGSL-reachable `SLANG_PASS` sequence in Phases A-C, the WGSL gates around `legalizeIRForWGSL`, `legalizeLogicalAndOr`, `specializeAddressSpaceForWGSL`, and the source-emission/Tint downstream path.
- Resolved the relative links used by the target page and checked that the required target-pipeline sections are present.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `### floatNonUniformResourceIndex`, lines 739-745 | The page says WGSL preserves a textual `NonUniformResourceIndex(...)` marker because `Tint forwards the marker to the SPIR-V NonUniform decoration`, but the source says the opposite: WGSL has no annotation to carry and the wrapper is dropped at emit time. | `source/slang/slang-ir-float-non-uniform-resource-index.cpp:42-57` states `Metal / WGSL / CUDA / CPU: the wrapper is dropped at emit time` and that WGSL/WebGPU has `no non-uniform annotation`; `source/slang/slang-emit.cpp:2072-2074` is only the Textual-mode call site. | Replace this callout with the source behavior: the pass runs in textual mode for WGSL only to reposition the wrapper, and the emitter drops it because WGSL has no non-uniform resource-indexing syntax. Do not claim Tint forwards it to a SPIR-V decoration. |
| F-002 | major | `## Phase B`, lines 267-407 | The Phase B diagram includes a `checkStaticAssert` node (`cSA`) after `specializeArrayParameters`, but the companion ordered table stops at `specializeArrayParameters`. The target-pipeline contract requires one table row per pass node in the diagram. | `docs/generated/design/target-pipelines/wgsl.md:267-336` shows `cSA["checkStaticAssert (direct call)"]` and the edge `sRU --> sFBLA1 --> dBL --> sAP --> cSA`; `docs/generated/design/_meta/prompts/_common.md:326-335` requires one ordered-table row per pass node; `source/slang/slang-emit.cpp:1792-1794` calls `checkStaticAssert` after specialization. | Add a final Phase B table row for `checkStaticAssert` with its direct-call gate, or remove the diagram node if direct calls are intentionally excluded from the table. Keep the diagram and table consistent. |
| F-003 | major | `## Phase D`, lines 607-616 | The Phase D ordered table uses the header `Pass / step` instead of the required `Pass` column. The target-pipeline table contract requires the exact columns `#`, `Pass`, `File`, `Gate`, and `Notes`. | `docs/generated/design/target-pipelines/wgsl.md:607` names the second column `Pass / step`; `docs/generated/design/_meta/prompts/_common.md:329-335` defines the required table columns. | Rename the Phase D table column to `Pass`. If the rows are intentionally steps rather than passes, keep the column contract and explain step-like rows in the `Notes` column. |

## No-issues notes

- The WGSL front matter contains all mandatory generated-document keys, and the recorded digest matches `regenerate.py digest target-pipelines/wgsl.md`.
- The central WGSL legalizer call is correctly tied to `CodeGenTarget::WGSL`, `WGSLSPIRV`, and `WGSLSPIRVAssembly` in `slang-emit.cpp`.
- The byte-address-buffer option summary matches the WGSL switch arm in `slang-emit.cpp`.
- The downstream path correctly identifies Tint for `WGSLSPIRV` and the extra disassembly transition for `WGSLSPIRVAssembly`.
