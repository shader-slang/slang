---
generated: true
model: claude-opus-5[1m]
generated_at: 2026-08-05T00:00:00Z
source_commit: 5634a0ea1b
watched_paths_digest: 00b1ce917654fb0ad0b3cc68e06fe14b4e2bacbf7bed870661952abe53db8a90
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for coverage/legalize

## Intent

White-box characterization tests for the entry-point / varying-parameter
legalizers: `source/slang/slang-ir-glsl-legalize.cpp` and
`source/slang/slang-ir-legalize-varying-params.cpp`. They pin the **current
observed legalization** of CLI-reachable entry-point parameter shapes, not a
spec.

The first round of this bundle covered the easy half of the GLSL system-value
table from the vertex/fragment/compute stages. The remaining gap was
concentrated in three places, and this round targets all three:

1. **Stages nothing else drives.** `invokePathConstantFuncInHullShader`,
   `getOrCreateBuiltinParamForHullShader`, `createPatchConstantFuncResultTypeLayout`,
   `legalizeMeshOutputParam`, `getOrCreatePerVertexInputArray`,
   `consolidateParameters` and `assignRayPayloadHitObjectAttributeLocations`
   were all at 0% line coverage. Hull, domain, geometry, mesh, per-vertex
   fragment and ray-tracing entry points reach them from a plain `slangc`
   invocation, so each gets one probe.
2. **The long tail of the GLSL system-value table.** `getGLSLSystemValueInfo`
   was the single largest gap in the file (231 uncovered lines). The semantics
   that were never exercised are the ones whose GLSL form is _not_ a rename —
   array-valued `SV_Coverage`, the `SV_DepthGreaterEqual` layout qualifier, the
   `noperspective`-sensitive `SV_Barycentrics` fork, the int-typed
   `gl_BaseVertex` / `gl_BaseInstance`, and the input-vs-output split of
   `SV_PrimitiveID`. Those are grouped into three probes by stage.
3. **The non-Vulkan varying legalizers.** `emitOptiXPayloadRead` /
   `emitOptiXPayloadWrite` (322 uncovered lines between them), the Metal and
   WGSL `getSystemValueInfo` tables, and Metal's amplification/mesh rewrite.
   All are reachable from text emit alone (`-target cuda` / `metal` / `wgsl`),
   with no downstream toolchain, so none of these tests are tool-gated.

Every pinned token was copied verbatim from a local `slangc` run at
`source_commit`. Where the target is text GLSL, the emitted shader was also
round-tripped through glslang (`-target spirv -emit-spirv-via-glsl`) and, where
the target is SPIR-V, validated with `SLANG_RUN_SPIRV_VALIDATION=1`, so no test
in this bundle pins output that the downstream consumer rejects. Several
shapes that _did_ produce output the consumer rejects were pulled out of the
tests and filed as findings instead (see below); this is why, for example, the
mesh probe stops short of `SV_ShadingRate` and the hull probe uses the SPIR-V
arm rather than text GLSL.

The mappings pinned here follow the documented GLSL/Vulkan, Metal and WGSL
builtin names, so they are treated as verified — no test in this bundle carries
`characterization-unverified`.

## Functional coverage

| Claim                                                                                                                                                                                                                                                        | Intent           | Anchor                                                                                                                                | Tests                                                                                                      |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------- |
| A vertex entry point's struct-typed varying input/output flatten into per-field GLSL `in`/`out` globals, with SV_Position routed to gl_Position. (`slang-ir-glsl-legalize.cpp`)                                                                              | characterization | [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | [`varying-struct-flatten-glsl.slang`](varying-struct-flatten-glsl.slang)                                   |
| The SPIR-V varying-param legalizer splits a struct input/output into per-field OpVariable globals with sequential Location decorations, routing SV_Position to BuiltIn Position. (`slang-ir-glsl-legalize.cpp`)                                              | characterization | [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | [`varying-struct-flatten-spirv.slang`](varying-struct-flatten-spirv.slang)                                 |
| The four compute thread-id system values map to their GLSL gl\_\* builtins. (`slang-ir-glsl-legalize.cpp`)                                                                                                                                                   | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`compute-sysval-builtins-glsl.slang`](compute-sysval-builtins-glsl.slang)                                 |
| A fragment SV_Position input maps to gl_FragCoord and SV_IsFrontFace to gl_FrontFacing. (`slang-ir-glsl-legalize.cpp`)                                                                                                                                       | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`fragment-position-frontface-glsl.slang`](fragment-position-frontface-glsl.slang)                         |
| An unrecognized SV\_\* system-value semantic is rejected with E49999 by the GLSL entry-point legalizer. (`slang-ir-glsl-legalize.cpp`)                                                                                                                       | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`unknown-system-value-semantic.slang`](unknown-system-value-semantic.slang)                               |
| SV_VertexID/SV_InstanceID lower to gl_VertexIndex/gl_InstanceIndex with the base offset subtracted in GLSL. (`slang-ir-glsl-legalize.cpp`)                                                                                                                   | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`vertex-id-base-offset-glsl.slang`](vertex-id-base-offset-glsl.slang)                                     |
| A hull shader's patch-constant function is spliced into the entry point behind a control barrier and an `invocation id == 0` guard, receiving the InputPatch parameter and a materialized OutputPatch. (`slang-ir-glsl-legalize.cpp`)                        | characterization | [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | [`hull-patch-constant-func-spirv.slang`](hull-patch-constant-func-spirv.slang)                             |
| A domain entry point routes its OutputPatch to Location inputs, SV_DomainLocation to TessCoord and the patch-constant struct's SV_InsideTessFactor to the Patch-decorated TessLevelInner. (`slang-ir-glsl-legalize.cpp`)                                     | characterization | [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | [`domain-shader-patch-input-spirv.slang`](domain-shader-patch-input-spirv.slang)                           |
| In a geometry shader SV_PrimitiveID splits into gl_PrimitiveIDIn on input and gl_PrimitiveID on output, SV_GSInstanceID becomes gl_InvocationID, and layer/viewport are narrowed to int. (`slang-ir-glsl-legalize.cpp`)                                      | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`geometry-shader-sysvals-glsl.slang`](geometry-shader-sysvals-glsl.slang)                                 |
| A mesh shader's outputs redeclare only the used members of gl_MeshPerVertexEXT / gl_MeshPerPrimitiveEXT, with indices in a standalone gl_PrimitiveTriangleIndicesEXT array. (`slang-ir-glsl-legalize.cpp`)                                                   | characterization | [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | [`mesh-shader-output-block-glsl.slang`](mesh-shader-output-block-glsl.slang)                               |
| A ray-tracing entry point with two or more inout varying parameters has them merged into one IncomingRayPayloadKHR struct variable at Location 0. (`slang-ir-glsl-legalize.cpp`)                                                                             | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`raytracing-multi-payload-consolidation-spirv.slang`](raytracing-multi-payload-consolidation-spirv.slang) |
| Ray payloads, callable payloads and hit-object attributes declared without a location get the lowest free number from a per-kind counter that skips explicitly claimed locations. (`slang-ir-glsl-legalize.cpp`)                                             | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`raytracing-payload-location-assignment-spirv.slang`](raytracing-payload-location-assignment-spirv.slang) |
| Repeated GetAttributeAtVertex on one nointerpolation fragment input share a single memoized `pervertexEXT` three-element array, declared without the interpolation qualifier. (`slang-ir-glsl-legalize.cpp`)                                                 | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`pervertex-input-array-glsl.slang`](pervertex-input-array-glsl.slang)                                     |
| SV_Coverage becomes element 0 of gl_SampleMaskIn/gl_SampleMask with a uint/int conversion, SV_InnerCoverage becomes the bool gl_FragFullyCoveredNV, and SV_DepthGreaterEqual becomes `layout(depth_greater)`. (`slang-ir-glsl-legalize.cpp`)                 | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`fragment-coverage-depth-glsl.slang`](fragment-coverage-depth-glsl.slang)                                 |
| SV_StartVertexLocation/SV_StartInstanceLocation map to the int gl_BaseVertex/gl_BaseInstance and force `#version 460`, and indexed SV_ClipDistanceN semantics share one gl_ClipDistance array. (`slang-ir-glsl-legalize.cpp`)                                | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`vertex-base-location-clip-distance-glsl.slang`](vertex-base-location-clip-distance-glsl.slang)           |
| SV_Barycentrics selects gl_BaryCoordEXT or gl_BaryCoordNoPerspEXT according to the field's `noperspective` modifier, alongside the fragment layer/viewport/sample/shading-rate mappings. (`slang-ir-glsl-legalize.cpp`)                                      | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`fragment-barycentrics-layer-viewport-glsl.slang`](fragment-barycentrics-layer-viewport-glsl.slang)       |
| A mixed-width OptiX ray payload is packed into 32-bit payload registers: sub-word fields share a register via shift/mask read-modify-write, aggregates recurse element-wise, and 64-bit fields span a pair. (`slang-ir-legalize-varying-params.cpp`)         | characterization | [#legalizeentrypointvaryingparamsforcuda](../../../design/target-pipelines/cuda.md#legalizeentrypointvaryingparamsforcuda)            | [`optix-payload-register-packing-cuda.slang`](optix-payload-register-packing-cuda.slang)                   |
| An aggregate hit-attribute struct is fetched one 32-bit optixGetAttribute slot per scalar in declaration order, with `__int_as_float` on float slots and no cast on integer slots. (`slang-ir-legalize-varying-params.cpp`)                                  | characterization | [#legalizeentrypointvaryingparamsforcuda](../../../design/target-pipelines/cuda.md#legalizeentrypointvaryingparamsforcuda)            | [`optix-hit-attribute-fetch-cuda.slang`](optix-hit-attribute-fetch-cuda.slang)                             |
| The Metal fragment system-value table maps SV_ViewID to `[[amplification_id]]` and SV_DepthGreaterEqual to `[[depth(greater)]]`, flattening the input struct into one attributed parameter per field. (`slang-ir-legalize-varying-params.cpp`)               | characterization | [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | [`metal-fragment-sysvals.slang`](metal-fragment-sysvals.slang)                                             |
| On Metal SV_GroupIndex has no attribute and is synthesized from `[[thread_position_in_threadgroup]]` and the `[numthreads]` extents, while the other vertex/compute ids map directly. (`slang-ir-legalize-varying-params.cpp`)                               | characterization | [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | [`metal-vertex-compute-sysvals.slang`](metal-vertex-compute-sysvals.slang)                                 |
| Metal amplification gains synthesized `_slang_mesh_payload` / `_slang_mgp` parameters and DispatchMesh becomes set_threadgroups_per_grid, while the mesh stage collapses its outputs into one `metal::mesh` object. (`slang-ir-legalize-varying-params.cpp`) | characterization | [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | [`metal-mesh-amplification.slang`](metal-mesh-amplification.slang)                                         |
| The WGSL fragment table maps SV_Depth to `@builtin(frag_depth)` and SV_Coverage to a scalar `@builtin(sample_mask)`, while SV_Target becomes `@location(0)`. (`slang-ir-legalize-varying-params.cpp`)                                                        | characterization | [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | [`wgsl-fragment-sysvals.slang`](wgsl-fragment-sysvals.slang)                                               |
| A system value the WGSL table marks unsupported is reported as E55202 naming the lowercased semantic at the declaration site, not silently dropped. (`slang-ir-legalize-varying-params.cpp`)                                                                 | characterization | [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | [`wgsl-unsupported-system-value-diag.slang`](wgsl-unsupported-system-value-diag.slang)                     |

## Unreachable gaps

- **`invokePathConstantFuncInHullShader` SV_PrimitiveID / SV_OutputControlPointID
  branches** (`slang-ir-glsl-legalize.cpp:1305-1315`) — dead for CLI input.
  Patch-constant-function parameters carry no `IRVarLayout`, so the
  `if (!layout)` guard at :1299 fires first and every scalar system-value
  parameter is rejected with E57002 before the named branches are consulted.
  Filed as `glsl-hull-patch-constant-system-value-param-rejected`; the branches
  become reachable only if that is fixed.
- **`getOrCreateBuiltinParamForHullShader`'s "parameter already present" path**
  (`:1128-1146`) — reachable in principle, but only jointly with the above: it
  looks up an existing entry-point parameter by system-value semantic, and the
  only two semantics it is ever called with are the two the patch-constant path
  rejects. The synthesis path (`:1148-1165`) is covered by
  `hull-patch-constant-func-spirv.slang`.
- **`legalizeDynamicResourcesForGLSL`** (`:5295`, 92 lines) — reachable, but
  already fully driven by the hand-written
  `tests/slang-extension/dynamic-resource-gl.slang`, which covers the same
  single/array `__DynamicResource` shapes. Duplicating it here would add no
  claim. Its `AmbiguousReferenceIr` arm (a dynamic-resource parameter with
  surviving uses after rewriting) is the only part that test misses, and I
  could not construct a `.as<T>()`-free use that survived the front end.
- **`getGLSLSystemValueInfo` `nv_x_right` / `nv_viewport_mask`** (`:876-905`) —
  the NVX multi-view-per-view attribute path. The source comment records it as
  a known-incomplete hack tracked by shader-slang/slang#109 ("This doesn't seem
  to work correctly on its own between hlsl/glsl"), so pinning current output
  would codify a behaviour the compiler itself does not claim is right.
- **`SV_StencilRef` (GLSL) and `SV_CullDistance` (GLSL)** — reachable and easy
  to drive, but the emitted `#extension` directive is malformed, so per the
  methodology this is a finding
  (`glsl-legalize-extension-name-missing-gl-prefix`), not a test. The same
  applies to `SV_ShadingRate` in a mesh primitive output
  (`glsl-mesh-shading-rate-block-member-type`), to a user-semantic
  patch-constant output (`hull-patch-constant-user-output-location-collides`),
  to a hull shader's whole text-GLSL form
  (`glsl-hull-patch-constant-func-void-return`), to a domain shader's text-GLSL
  form (`glsl-domain-shader-patch-input-array-size`), and to a callable shader
  with two `inout` parameters
  (`callable-shader-multi-inout-uses-ray-payload-storage`). Each of those is
  the reason the corresponding test here uses the SPIR-V arm, or omits the
  field.
- **`EntryPointVaryingParamLegalizeContext::diagnoseUnsupportedSystemVal` /
  `diagnoseUnsupportedUserVal`**
  (`slang-ir-legalize-varying-params.cpp:1042`/`:1053`) — still 0%. These are
  the _base-class_ fallbacks; every concrete target context (Metal, WGSL, CUDA,
  CPU) overrides the unsupported path with its own diagnostic, which is what
  `wgsl-unsupported-system-value-diag.slang` pins. The base versions fire only
  for a target whose context does not override them, and no such target is
  reachable from the CLI today.
- **`CUDAEntryPointVaryingParamLegalizeContext` oversized-hit-attribute arm**
  (`:2383`) — reachable (a hit-attribute struct over 32 bytes), but it reports
  an `E99999` internal error rather than a user diagnostic, so it is filed as
  `optix-oversized-hit-attributes-internal-error` instead of being pinned.
- **`LegalizeShaderEntryPointContext::handleSpecialSystemValue` base version**
  (`:2833`) and the `assign(IRBuilder&, LegalizedVaryingVal, IRInst*)` overload
  (`:3186`) — defensive base-class bodies that assert or no-op; every target
  that has a special system value overrides them.
- **`slang-ir-legalize-types.cpp` gaps** carried over from the previous round
  (`UseOfUninitializedOpaqueHandle`, `CooperativeMatrixUnsupportedCapture`) are
  no longer in scope: the bundle's `coverage_targets` are only
  `slang-ir-glsl-legalize.cpp` and `slang-ir-legalize-varying-params.cpp`.

## Doc gaps observed

| Anchor                                                                                                                                | Kind                  | Gap                                                                                                                                                                                                                                                                                                         | Suggested addition                                                                                                                                                                                                                                           |
| ------------------------------------------------------------------------------------------------------------------------------------- | --------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | undocumented-behavior | The doc notes the GLSL legalizer runs "despite the name" on the SPIR-V arm but does not state that it performs HLSL-to-Vulkan index rebasing — SV_VertexID becomes `gl_VertexIndex - gl_BaseVertex` (and SV_InstanceID likewise) so user code keeps HLSL's per-draw zero-based semantics.                   | Add a short note (and a one-line example) that SV_VertexID / SV_InstanceID are rebased by subtracting gl_BaseVertex / gl_BaseInstance on the GLSL/SPIR-V path, since the emitted value is not a bare `gl_VertexIndex`.                                       |
| [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | missing-surface       | The doc describes the GLSL legalizer pass but does not enumerate the system-value-semantic to gl\_\* builtin mapping table (SV_DispatchThreadID->gl_GlobalInvocationID, SV_Position(fragment)->gl_FragCoord, SV_IsFrontFace->gl_FrontFacing, etc.) nor the E49999 it raises for an unknown SV\_\* semantic. | Add a mapping table of the recognized SV\_\* semantics to their gl\_\* builtins per stage, and note that an unrecognized SV\_\* semantic is rejected with error 49999 "unknown system-value semantic".                                                       |
| [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | missing-surface       | The section does not mention that a hull entry point is rewritten to call its `[patchconstantfunc]` inline, behind a control barrier and an `invocation id == 0` guard, with the OutputPatch argument materialized from the control-point output array — the largest structural rewrite in the pass.        | Add a short subsection on hull-stage legalization describing the barrier + single-invocation guard, and note that the two patch kinds are supplied from different places (the entry point's own InputPatch parameter vs a materialized control-point array). |
| [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | missing-surface       | Ray-tracing entry points get a rewrite the section does not describe: a second (or later) `inout` varying parameter causes all of them to be merged into one anonymous struct in a single payload variable at location 0, and payloads left without an explicit location are numbered per kind afterwards.  | Document the `consolidateRayTracingParameters` / `assignRayPayloadHitObjectAttributeLocations` pair: when consolidation kicks in (two or more inout varying params), and that ray, callable and hit-object-attribute locations are numbered independently.   |
