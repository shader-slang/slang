---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:05:13+00:00
target_doc: target-pipelines/index.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 88909e4def1133ca5cd3ccb36f17d01f8bcc633abff88b21acd9208e1a05d1f2
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: partial
  style_consistency: fail
  source_alignment: fail
  front_matter_validity: pass
finding_count: 5
severity_breakdown:
  critical: 2
  major: 1
  minor: 2
  nit: 0
---

# Review report for target-pipelines/index.md

## Summary
The page has the required section order, complete five-target comparison, valid front matter, and resolving links, but it no longer conforms to the compact navigation-hub contract. Two source-behavior statements are actively misleading: Metal `printf` is not gated on a `metallib_3_2` atom, and the named HLSL/CUDA passes are not the whole of their target-specific work. Extensive pass-level material also duplicates details that the index contract explicitly reserves for child pages.

## Items checked
- Read the target, `_common.md`, its per-document prompt, all seven resolved watched files, and all five `depends_on` target pages reported by `regenerate.py show`.
- Verified source against commit `53b76e6d3009b8e6434d41573524c7ce5c499d23` and spot-checked 18 factual claims, including enum values, emitter selection, downstream transitions, target legalizers, loop behavior, and `RequiredLoweringPassSet`.
- Re-derived all 12 line-number citations in the body; every cited line or range matches the recorded source commit.
- Resolved all 34 relative-link occurrences (23 unique targets) at the recorded commit and confirmed generated-page references are manifest entries; none are dangling.
- Checked required sections, comparison-table columns and rows, peer-page coverage, front-matter keys, style basics, and the 14,148-byte document against its 32,768-byte cap.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Pages`, Metal bullet, lines 41-46 | The text says Metal `printf` is `gated on the metallib_3_2 capability atom`. The emitter actually records an MSL 3.2 requirement when it encounters `printf`, and downstream code explicitly handles this when no metallib atom is present. | `source/slang/slang-emit-metal.cpp:903-910` calls `requireMetalLanguageVersion(3, 2)` and `requireLogging()`. `source/slang/slang-code-gen.cpp:783-800` says `printf` emits metal3.2 “even though no metallib atom is present” and merges the emitter-recorded version into downstream options. | Replace the capability-gate claim with: Metal `printf` makes the emitter require MSL 3.2 and enables Metal logging for the downstream compile. |
| F-002 | critical | `## Cross-target comparison`, caveat, lines 126-134 | The sentence that HLSL and CUDA scatter work across individual arms but `the named passes above are the whole of it` is false. The table names two HLSL examples and one CUDA-only pass, while the orchestrator contains additional target-specific calls for both families. | HLSL additionally runs `legalizeNonVectorCompositeSelect` and `wrapStructuredBuffersOfMatrices` at `source/slang/slang-emit.cpp:1493-1499` and `1988-2001`. CUDA additionally runs `synthesizeActiveMask` and `legalizeEntryPointVaryingParamsForCUDA` at `source/slang/slang-emit.cpp:2158-2164` and `2249-2253`. | Delete `the named passes above are the whole of it`; say only that HLSL and CUDA use multiple target-specific arms, and leave the inventory to their child pages. |
| F-003 | major | `## Pages` through `## Filtering rules`, especially lines 27-58 and 124-229 | The index contains detailed per-pass behavior, gate implementation, decoration cleanup, scan ordering, and pass-result mutation. This substantially violates the navigation-hub contract and duplicates child-page content. | `docs/generated/design/_meta/prompts/_common.md:377-423` defines a compact index and forbids `per-pass details`; `docs/generated/design/_meta/prompts/target-pipelines-index.md:8-14,70-75` likewise says this is not a target page and must not document passes. | Reduce each page bullet to one clause, retain only the required four-phase overview and comparison table, and shorten filtering to the target-arm reminder. Remove the pass-level caveats and `RequiredLoweringPassSet` deep dive. |
| F-004 | minor | Introductory paragraph, lines 12-23 | The first paragraph explains what the page covers but does not identify its intended reader, despite the universal content rule and the per-document audience declaration. | `docs/generated/design/_meta/prompts/_common.md:65-66` requires both coverage and intended reader; `docs/generated/design/_meta/prompts/target-pipelines-index.md:12-14` identifies the reader as a developer choosing a per-target page. | Add a short audience phrase such as “for compiler developers choosing the relevant target pipeline page.” |
| F-005 | minor | `## Pages`, CUDA bullet, lines 53-58 | The claim that the PyTorch/`slangpy` path is autodiff’s `main consumer` and therefore the gate matters `most` on CUDA is a comparative usage assertion not established by the watched source or dependencies. The source only establishes a target-independent autodiff gate and CUDA/PyTorch-related paths. | `source/slang/slang-emit.cpp:1446-1453` applies the autodiff/strip branch without a CUDA-specific condition; no cited source provides consumer-frequency data. | Remove `main consumer` and `matters most`; summarize only the concrete CUDA/PyTorch autodiff connection documented by source. |
