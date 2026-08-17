---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:06:57+00:00
target_doc: target-pipelines/wgsl.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 893e68384601fb6107ed1d9426d6ba0a0ad7b13bd39f42f529bf8c28e6020a47
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: fail
  front_matter_validity: fail
finding_count: 7
severity_breakdown:
  critical: 1
  major: 3
  minor: 3
  nit: 0
---

# Review report for target-pipelines/wgsl.md

## Summary
The page has the required overall structure and all relative links resolve, but several pipeline details do not match the source at the recorded commit. Most importantly, it states the exact opposite of the actual default phi-elimination configuration. The Phase-D artifact-construction path and the watched-path digest are also incorrect.

## Items checked
- Ran `regenerate.py show target-pipelines/wgsl.md` and inspected the target page, `_common.md`, the per-document prompt, all eight resolved watched files, and all five dependency documents.
- Confirmed the recorded source commit equals review-time `HEAD` and that `source/` has no staged or unstaged differences, then verified every line-number citation against that source revision.
- Ran the document linter successfully and resolved all relative links, including generated-document dependencies and the WGSL user-guide link.
- Spot-checked more than ten factual claims, including target reduction, both required-lowering scans, all WGSL-specific legalization call sites, byte-address options, logical-operator legalization, address-space specialization, phi elimination, emitter policy overrides, bool-to-int emission, and the Tint/glslang downstream chain.
- Recomputed the watched-path digest and checked the required headings, phase diagrams, companion tables, conditional-gate groups, loops section, notable-pass callouts, and front-matter fields.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Phase C`, row 28; `### eliminatePhis with default options`, lines 981-989 | The page says default `PhiEliminationOptions` are `eliminateCompositeTypedPhiOnly = true` and `useRegisterAllocation = false`, and that SPIR-V changes both. The actual defaults are the opposite, and WGSL passes that default-constructed object unchanged. | `source/slang/slang-ir-eliminate-phis.h:11-15` initializes the fields to `false` and `true`; `source/slang/slang-emit.cpp:2570-2576` default-constructs the options and changes them only in the direct-SPIR-V branch, to the same `false`/`true` values. | Change both Phase C references to `eliminateCompositeTypedPhiOnly = false` and `useRegisterAllocation = true`, and remove the claimed WGSL-versus-SPIR-V contrast at this commit. |
| F-002 | major | `## Phase D: WGSL emit and downstream tools`, lines 663-706 | The diagram and table put `createArtifactFromIR` on the WGSL text-emission path, but that function is not called there. The source path constructs the text artifact directly with `ArtifactUtil::createArtifactForCompileTarget` and adds the string representation. | `source/slang/slang-emit.cpp:2972-2975` constructs and populates the textual artifact. `createArtifactFromIR` is defined at line 3292 and its call at line 3523 belongs to direct SPIR-V emission. | Replace the `createArtifactFromIR` node and row with the actual `ArtifactUtil::createArtifactForCompileTarget` plus `addRepresentationUnknown` packaging step, citing lines 2972-2975. |
| F-003 | major | Phase A diagram/table, lines 119-188; Phase B diagram/table, lines 213-471 | The companion tables do not contain one row per call node shown in their diagrams. Phase A diagrams `emit IRBuildIdentifier` without a row; Phase B diagrams `clearTranslationDictionary`, a direct `eliminateDeadCode`, and `reportCheckpointIntermediates` without rows. | The one-row-per-node requirement is in `docs/generated/design/_meta/prompts/_common.md:329-338`. The calls occur at `source/slang/slang-emit.cpp:1032`, `1628`, `1654`, and `1727`. | Add ordered table rows for these four diagram nodes, or remove any non-pass nodes from the diagrams and explain them in prose; keep diagram and table membership identical. |
| F-004 | minor | `### requiredLoweringPassSet.* flags`, lines 726-735 | The autodiff base-class checks are cited as lines 1429-1432, but those lines contain the `specializeHigherOrderParameters` call. | The checks for `IRTranslateBase` through `IRDifferentialPairGetPrimalBase` are at `source/slang/slang-emit.cpp:419-422`; lines 1429-1432 are unrelated. | Change the citation from lines 1429-1432 to lines 419-422. |
| F-005 | minor | Opening paragraph, lines 12-30 | The opening explains what the page covers but never identifies its intended reader, which is mandatory for the first body paragraph. | `docs/generated/design/_meta/prompts/_common.md:65-66` requires both coverage and intended reader; `docs/generated/design/_meta/prompts/target-pipelines-wgsl.md:12-14` identifies that reader as a compiler developer locating WGSL codegen passes and cooperation between the legalizer and emitter. | Add a short intended-reader clause to the first paragraph using the audience stated in the per-document prompt. |
| F-006 | minor | `### legalizeByteAddressBufferOps with WGSL options`, lines 970-979 | The page says WGSL is the only target combining the first four listed option values. Metal sets those same four values; WGSL is unique here only because it additionally sets `useBitCastFromUInt = true`. | `source/slang/slang-emit.cpp:2075-2082` sets the first four values for Metal, while lines 2083-2091 set those four plus `useBitCastFromUInt` for WGSL. | Say that WGSL is the only listed arm combining all five settings, or explicitly say it shares the first four with Metal and adds `useBitCastFromUInt`. |
| F-007 | major | YAML front matter, line 6 | The recorded watched-path digest does not match the digest for the resolved watched files at the recorded source commit. Because that commit is also review-time `HEAD` and `source/` is clean, this is not explained by source drift. | The page records `893e68384601fb6107ed1d9426d6ba0a0ad7b13bd39f42f529bf8c28e6020a47`; `regenerate.py digest target-pipelines/wgsl.md` returns `795676de268ef587945f67952a87124c5230562d8aa68f064f30db07c11f8889`. | Refresh `watched_paths_digest` through the normal freshness workflow, using the recomputed value if the source commit remains unchanged. |
