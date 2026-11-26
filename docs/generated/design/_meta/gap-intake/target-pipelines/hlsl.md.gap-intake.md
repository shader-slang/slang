---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-02T00:00:00Z
target_doc: target-pipelines/hlsl.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 3
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for target-pipelines/hlsl.md

## Summary

Nothing was escalated. All three open gaps are `drift-from-source` against the
same cause: the three HLSL ray-payload passes the page described
(`legalizeEmptyRayPayloadsForHLSL`, `legalizeNonStructParameterToStructForHLSL`,
`legalizeRayPayloadAccessQualifiersForHLSL`) no longer exist in
`source/slang/slang-ir-hlsl-legalize.cpp`, and the page was wrong, not the
compiler. All three were `fixed` in one consolidated edit, as part of a
focused refresh of the page to the current source: the Phase B pass table,
diagram, gate tables and the three notable-pass sections now attribute the
behaviour to `legalizeRayTracingPayloads`. The three section headings are kept
as historical names so existing anchors resolve. One observation in
`1f89f27b8e63` could not be confirmed (see Actions). The new pass lives in
`source/slang/slang-ir-ray-tracing-legalize.cpp`, which is not among the
page's watched paths; adding it to the manifest is recommended.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| a81f0ecb07d5 | fixed | `source/slang/slang-ir-hlsl-legalize.cpp` now defines only `validateBarrierFlagsForHLSL` (line 67). `source/slang/slang-emit.cpp` line 1989 calls `legalizeRayTracingPayloads` inside the `shouldLegalizeExistentialAndResourceTypes` block (line 1982), ahead of `legalizeExistentialTypeLayout` (2034) and `legalizeResourceTypes` (2055). `source/slang/slang-ir-ray-tracing-legalize.cpp`: `getOrCreateDummyPayloadType` (188-206) builds a separate struct `DummyRayPayload` / `DummyCallablePayload` with one `uint` field `_slang_dummy`; `createDummyPayloadArgument` (226) and `legalizeD3DCall` (416) use it at native calls; `legalizeEntryPoint` (786-881) retypes an all-empty receiving parameter to it while the body keeps a local of the original empty type. The pass name, its position and the gate references in the page's Phase B text were stale. | Rewrote the empty-payload section around the separate physical `DummyRayPayload` struct; renamed and renumbered the Phase B row (59), the diagram node, the Phase B bullets and the gate tables to `legalizeRayTracingPayloads`; added a `legalizeRayTracingPayloads` section. |
| 1f89f27b8e63 | fixed | `legalizeNonStructParameterToStructForHLSL` is gone from `source/slang/slang-ir-hlsl-legalize.cpp` (file ends after `validateBarrierFlagsForHLSL`). Wrapping now lives in `source/slang/slang-ir-ray-tracing-legalize.cpp`: `needsForcedStructWrapper` (89), `getForcedStructType` (330, wrapper structs `RayPayload_t` / `ForceVarIntoStructTemporarily_t` / `HitAttributes_t` with field `data`), `legalizeD3DCall` (416) and `emitForcedStructTemporary` (501) for native-call arguments including the `ForceVarIntoRayPayloadStructTemporarily` marker, and `wrapNonStructEntryPointParam` (696) called from `legalizeEntryPoint` (786). Source disagreement with the gap: the gap says an entry-point `inout float4` payload is emitted unwrapped; `legalizeEntryPoint` wraps any non-empty non-struct `out` / `inout` payload parameter of an `anyhit`, `closesthit`, `miss` or `callable` entry point on D3D (lines 809-845, via `IROutParamTypeBase`), and the wrapper support for receivers was added by 93a54974c (#13315), which is an ancestor of this HEAD. No runnable `slangc` was available here (the `build/Release/bin/slangc` binary does not execute on this host), so the observed unwrapped output could not be reproduced and may come from a build older than #13315; the page documents what the source does. | Re-attributed the section to `legalizeRayTracingPayloads`; rewrote it to cover native-call arguments, `ReportHit`, and receiving-shader parameters (not only call arguments); dropped the claim that DXC requires all DXR stage parameters to be structs and that the pass unwrapped them inside the entry point. |
| 41fc13467438 | fixed | `legalizeRayPayloadAccessQualifiersForHLSL` is gone from `source/slang/slang-ir-hlsl-legalize.cpp`. `source/slang/slang-ir-ray-tracing-legalize.cpp`: `requiresPayloadAccessQualifiers` (76-84: `isD3DTarget`, DX family, `>= DX_6_7`) is consulted at the end of `legalizeRayTracingPayloads` (928-937), which calls `addDefaultPayloadAccessQualifiersToStruct` (63) for every module-level struct with `IRRayPayloadDecoration`; `addDefaultPayloadAccessQualifiersToField` (30-61) still fills the full `caller, anyhit, closesthit, miss` list for a missing side. The SM 6.7 output in the page's example is consistent with that code. `source/slang/slang-emit.cpp` no longer has a separate gate for it. | Re-attributed the section, the pass-list row (merged into row 59) and the profile-predicate row to `legalizeRayTracingPayloads`; removed the `isD3DTarget` predicate row entries for the deleted passes and added the new line references. |
