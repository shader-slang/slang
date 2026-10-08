---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T12:34:00Z
target_doc: target-pipelines/index.md
review_report: ../../reviews/target-pipelines/index.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for target-pipelines/index.md

## Summary

Four of the five findings (both criticals and both minors) were verified against source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23` and fixed with single-clause edits: the Metal `printf` gate claim, the "whole of it" claim about HLSL/CUDA target arms, the unsupported autodiff "main consumer" comparison, and the missing intended-reader clause. The one major finding, F-003, is deferred: its core complaint is valid but its remedy is a whole-page restructuring, and part of its recommendation contradicts the index contract. The document was edited.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed. `source/slang/slang-emit-metal.cpp:903-910` handles `kIROp_Printf` by unconditionally calling `requireMetalLanguageVersion(SemanticVersion(3, 2))` and `requireLogging()` on the extension tracker — there is no capability test. `source/slang/slang-code-gen.cpp:783-785` states outright that `printf` emits metal3.2 "even though no metallib atom is present", and lines 792-800 merge the emitter-recorded version and the logging flag into the downstream options. The `metallib_3_2` atom does exist (`source/slang/slang-capabilities.capdef:204`, aliased into `printf` at line 2473), but it is not what drives this emit path, so "gated on" misdescribes it. Child page `docs/generated/design/target-pipelines/metal.md:713-717` already describes it as the emitter recording the requirement. | `## Pages`, Metal bullet: "(gated on the `metallib_3_2` capability atom)" -> "(which makes the emitter require MSL 3.2 and enable Metal logging for the downstream compile)". |
| F-002 | fixed | Confirmed. Beyond the passes the table names, HLSL reaches `legalizeNonVectorCompositeSelect` at `source/slang/slang-emit.cpp:1498` (inside a `case CodeGenTarget::HLSL:` arm under the `requiredLoweringPassSet.nonVectorCompositeSelect` gate) and `wrapStructuredBuffersOfMatrices` at line 1999; CUDA reaches `synthesizeActiveMask` at line 2164 (`CUDASource`/`CUDAHeader`/`PTX` arm) and `legalizeEntryPointVaryingParamsForCUDA` at line 2252 (`CUDASource`/`CUDAHeader` arm). The named passes are therefore not the full inventory. | `## Cross-target comparison` caveat: "(the named passes above are the whole of it)" -> "(the passes named above are examples, not the full inventory — each child page lists them all)". |
| F-003 | deferred | The core complaint is valid: `docs/generated/design/_meta/prompts/_common.md:421-423` forbids per-pass details in the index, and the `### Filtering by IR content` subsection plus several `## Pages` bullets carry them. But the recommended remedy touches all five page bullets, both caveat paragraphs, and an entire subsection — roughly half the body — which exceeds the smallest-reasonable-change edit rule at `docs/generated/design/_meta/prompts/_remediate.md:95-96` ("Replace one line, one cell, one row — not the whole section"). Part of the recommendation is also contract-contradicting: `_common.md:404-409` *mandates* the two pass-level caveats the finding asks to delete (the `legalizeIRForSPIRV`-in-Phase-D placement and the unenforced `simplifyIRForSpirvLegalization` bounds), so "remove the pass-level caveats" cannot be applied as written. Follow-up needed: regenerate the page against `docs/generated/design/_meta/prompts/target-pipelines-index.md` under the compact index contract, keeping the two mandated SPIR-V caveats, rather than spot-editing it. | — |
| F-004 | fixed | Confirmed. `docs/generated/design/_meta/prompts/_common.md:66-67` requires the first body paragraph to say what the document covers **and** who its intended reader is; the prompt at `docs/generated/design/_meta/prompts/target-pipelines-index.md:12-14` names that reader. The intro named only the coverage. | Intro, first sentence: added ", written for compiler developers who need to pick the right per-target page". |
| F-005 | fixed | Confirmed. `source/slang/slang-emit.cpp:1446-1453` selects `finalizeAutoDiffPass` or `stripAutoDiffDecorations` purely on `requiredLoweringPassSet.autodiff`, with no target predicate anywhere in the branch, so the gate is target-independent and nothing in the watched source supports "matters most" on CUDA or ranks autodiff consumers. | `## Pages`, CUDA bullet: "why the autodiff gate matters most here (the PyTorch / `slangpy` binding path is autodiff's main consumer)" -> the autodiff gate described as target-independent, with the PyTorch / `slangpy` path named as one path that reaches it. |
