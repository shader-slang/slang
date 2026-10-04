---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T13:35:00Z
target_doc: target-pipelines/wgsl.md
review_report: ../../reviews/target-pipelines/wgsl.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/wgsl.md

## Summary

All seven findings were re-verified against source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`. Six were fixed and one (F-007, the stale `watched_paths_digest`) was rejected as out of scope because the remediation contract reserves that front-matter field for the operator. The document was edited. The critical fix is F-001: the default `PhiEliminationOptions` were stated backwards, and the claimed WGSL-versus-SPIR-V contrast does not exist at this commit. F-003 added four ordered rows, so Phase A was renumbered to 18 rows and Phase B to 72; one in-page row cross-reference was updated to match.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed: `source/slang/slang-ir-eliminate-phis.h:13-14` initializes `eliminateCompositeTypedPhiOnly = false` and `useRegisterAllocation = true` — the opposite of what the page claimed. `source/slang/slang-emit.cpp:2570` default-constructs the options and the `isKhronosTarget && emitSpirvDirectly` branch at lines 2571-2575 assigns those same two values, so direct SPIR-V is not a contrast case at this commit. | `### eliminatePhis with default options`: both values corrected and the SPIR-V contrast replaced with a statement that the direct-SPIR-V branch assigns the same values. Phase C row 28 Notes now spell out both defaults and cite the header. |
| F-002 | fixed | Confirmed: `source/slang/slang-emit.cpp:2972-2973` builds the textual artifact with `ArtifactUtil::createArtifactForCompileTarget` and `addRepresentationUnknown`. `createArtifactFromIR` is defined at line 3292 and called only at line 3523, inside direct SPIR-V emission. | Phase D: intro sentence, diagram node, and table row 7 now name `createArtifactForCompileTarget` + `addRepresentationUnknown` at lines 2972-2973, with a note that `createArtifactFromIR` is not on this path. |
| F-003 | fixed | Confirmed all four call sites: `builder.emitDebugBuildIdentifier` at `slang-emit.cpp:1032`, `clearTranslationDictionary(irModule)` at `:1628`, the direct `eliminateDeadCode` at `:1654`, and `reportCheckpointIntermediates` at `:1727`. `_common.md:329-338` requires one table row per diagram node. | Added four ordered rows: `IRBuilder::emitDebugBuildIdentifier` in Phase A (now row 2) and `clearTranslationDictionary`, `eliminateDeadCode`, `reportCheckpointIntermediates` in Phase B. Each is marked as a direct call rather than a `SLANG_PASS`. Phase A renumbered 1-18, Phase B 1-72, and the "same file as row 64" note updated to row 67. |
| F-004 | fixed | Confirmed: the autodiff base-class checks are at `source/slang/slang-emit.cpp:419-422`; lines 1429-1432 are in `linkAndOptimizeIR` and unrelated. | `### requiredLoweringPassSet.* flags`: citation changed from lines 1429-1432 to lines 419-422. |
| F-005 | fixed | Confirmed: the opening paragraph stated coverage only; `_common.md:65-66` requires the intended reader, and `prompts/target-pipelines-wgsl.md:12-14` names that reader. | Opening paragraph: added the compiler-developer audience clause from the per-document prompt. |
| F-006 | fixed | Confirmed: `source/slang/slang-emit.cpp:2075-2082` sets the same four flags for the Metal arm; only the WGSL arm at lines 2083-2091 adds `useBitCastFromUInt = true`. | `### legalizeByteAddressBufferOps with WGSL options`: reworded to say WGSL shares the first four settings with Metal and is unique in adding `useBitCastFromUInt`, with both line ranges cited. |
| F-007 | rejected-out-of-scope | The mismatch is real, but `docs/generated/design/_meta/prompts/_remediate.md:97-100` reserves `generated_at`, `source_commit`, and `watched_paths_digest` for the operator's `regenerate.py mark-fresh` run and forbids the remediator from editing them. This remediation edits the page, so `mark-fresh` will record the correct digest. | — |
