---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:07:18+00:00
target_doc: pipeline/06-emit.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 8de686864f8c89a689087094669d66b19be061b10c489eb3d49177dc519b34b4
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: partial
finding_count: 4
severity_breakdown:
  critical: 0
  major: 1
  minor: 3
  nit: 0
---

# Review report for pipeline/06-emit.md

## Summary

The page now covers the required backend structure and most source behavior accurately, but its front-matter digest no longer matches the expanded watched set. The stale “Paths outside the watched set” discussion also contradicts the current manifest. Two smaller generalizations overstate which artifacts carry post-emit metadata and which textual targets ship prelude headers.

## Items checked

- Reviewed the target, `_common.md`, the per-document prompt, the resolved watched files from `regenerate.py show`, and dependency `pipeline/05-ir-passes.md` at commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Verified all nine line numbers in the body's six line-number citation sites: `slang-emit.cpp` lines 970, 2746, 2993, 3292, 3500, 3544, and 3587, plus `slang-emit-hlsl-prelude.cpp` lines 553 and 586.
- Spot-checked more than ten factual claims across outer dispatch, emitter selection, line-directive defaults, direct SPIR-V legalization and downstream gates, HLSL symbolic flags, Metal extension tracking, WGSL switch handling, inheritance, VM diagnostics, Slang stubbing, LLVM output forms, source maps, dependency output, preludes, and precedence handling.
- Resolved all 58 unique relative link targets at the recorded commit and confirmed generated-peer references are present in the manifest; the document is 22,089 bytes against a 32,768-byte cap.
- Swept 150 documented identifiers and 48 source filenames against the recorded tree; no source identifier or filename was fabricated.
- Recomputed the watched-path digest as `d41c93eaac05bae9ae158e44147f1873e1923b6c15a3d4d962ed724c292eb0ac`, which does not match the front matter.

## Findings

| ID    | Severity | Location                                                                             | Description                                                                                                                                                                                                                                                                                                                                    | Evidence                                                                                                                                                                                                 | Recommendation                                                                                                                                                                 |
| ----- | -------- | ------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| F-001 | major    | Front matter, line 6                                                                 | `watched_paths_digest` is `8de686...`, but the current manifest adds `slang-code-gen.cpp` and `slang-global-session.cpp`; `regenerate.py digest pipeline/06-emit.md` returns `d41c93eaac05bae9ae158e44147f1873e1923b6c15a3d4d962ed724c292eb0ac`. The mandatory freshness metadata is therefore stale.                                          | `docs/generated/design/_meta/manifest.yaml` lines 212-225 define the expanded watched set; the digest command at review time produced the value above.                                                   | Regenerate the front matter against the current manifest so `watched_paths_digest` is the computed `d41c93...` value.                                                          |
| F-002 | minor    | `## Emit dispatcher`, lines 41-44; `## Paths outside the watched set`, lines 420-437 | The page says `slang-code-gen.cpp` is outside the watched paths and that the manifest watches only `slang-emit.cpp` plus `slang-emit-*`; both claims are obsolete. It also recommends adding `slang-code-gen.cpp` even though that path and `slang-global-session.cpp` are already watched.                                                    | `docs/generated/design/_meta/manifest.yaml` lines 214-222 list both files, matching the resolved files printed by `regenerate.py show pipeline/06-emit.md`.                                              | Remove the “not in the watched paths” qualifier near the dispatcher and revise the final section to discuss only genuinely unwatched dependencies such as `prelude/*.h`.       |
| F-003 | minor    | `## Inputs and outputs`, lines 28-33                                                 | “The artefact carries ... the post-emit metadata” overgeneralizes across emit paths. Source artifacts and direct SPIR-V attach `linkedIR.metadata`, but the HostVM path creates an artifact containing only the serialized bytecode, and the LLVM dispatcher likewise returns backend-created artifacts without attaching `linkedIR.metadata`. | `source/slang/slang-emit.cpp` lines 2972-2986 and 3520-3531 attach metadata for source/direct-SPIR-V output; lines 3581-3582 and 3607-3636 do not do so for HostVM or LLVM.                              | Qualify the metadata statement by emit path: source and direct-SPIR-V artifacts attach post-emit metadata, while HostVM and LLVM do not attach it in these dispatch functions. |
| F-004 | minor    | `## Preludes`, lines 336-367                                                         | “Each textual target ships a prelude header” is false and contradicts the later statement that GLSL, Metal, and WGSL have no `prelude/` header. The session registers shipped prelude strings only for CUDA, C++, and HLSL, with Torch and heterogeneous host output handled separately.                                                       | `source/slang/slang-global-session.cpp` lines 125-128 register CUDA, C++, and HLSL strings; `source/slang/slang-emit.cpp` lines 2937-2951 special-case Torch and host C++ before consulting the session. | Change the opening sentence to say that only targets listed in the table use shipped prelude headers, while GLSL, Metal, and WGSL rely on backend-emitted vocabulary.          |

## No-issues notes

- Every required backend has a level-3 subsection and states its emitted form.
- Direct SPIR-V legalization and the downstream optimization/link/validation/debug gate match the recorded source.
- HLSL named flags, Metal tracker retention, WGSL switch handling, VM diagnostics, and the Slang round-trip stub are accurately described.
