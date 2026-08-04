---
generated: true
model: claude-opus-4-8[1m]
generated_at: 2026-06-11T17:00:00Z
source_commit: ef1068b5485e09b3a7afadba2e25f9541e29af42
watched_paths_digest: 00b1ce917654fb0ad0b3cc68e06fe14b4e2bacbf7bed870661952abe53db8a90
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for coverage/legalize

## Intent

White-box characterization tests for the entry-point / varying-parameter
legalizers: `source/slang/slang-ir-glsl-legalize.cpp` (~63% covered),
`slang-ir-legalize-types.cpp` (~48%), and
`slang-ir-legalize-varying-params.cpp` (~61%). They pin the **current
observed legalization** of CLI-reachable entry-point parameter shapes,
not a spec.

Strategy: drive struct-typed and system-value entry-point parameters
through `slangc` and pin the per-target lowering the legalizers produce.
The chosen probes hit the divergence point the hint names — the GLSL
legalizer (`createGLSLBuiltinSystemVarStorage`, the system-value mapping
tables) versus the SPIR-V varying-param legalizer (`OpVariable` +
`OpDecorate Location`) — so the same struct shape is observed on both
arms. The system-value mappings (`SV_VertexID`->`gl_VertexIndex -
gl_BaseVertex`, the compute thread-id table, fragment `gl_FragCoord` /
`gl_FrontFacing`) and the struct-flatten naming are pinned with `SIMPLE`
emit + FileCheck across the targets each divergence is visible on; these
report `ignored` locally (FileCheck absent) and validate in CI. The
error-path probe (an unrecognized `SV_*` semantic, `E49999`) is a
`DIAGNOSTIC_TEST`, which validates locally on this runner.

All pinned tokens were copied verbatim from the local `slangc` at
`source_commit`. The mapped GLSL builtin names and the
`gl_VertexIndex - gl_BaseVertex` rebasing match Vulkan's documented
zero-vs-base-indexed contract, so the behaviours are treated as verified
(no `characterization-unverified` tags).

## Functional coverage

| Claim                                                                                                                                                                            | Intent           | Anchor                                                                                                                                | Tests                                                                              |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------- |
| A vertex entry point's struct-typed varying input/output flatten into per-field GLSL `in`/`out` globals, with SV_Position routed to gl_Position.                                 | characterization | [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | [`varying-struct-flatten-glsl.slang`](varying-struct-flatten-glsl.slang)           |
| The SPIR-V varying-param legalizer splits a struct input/output into per-field OpVariable globals with sequential Location decorations, routing SV_Position to BuiltIn Position. | characterization | [#entry-point-and-parameter-handling](../../../design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling)                    | [`varying-struct-flatten-spirv.slang`](varying-struct-flatten-spirv.slang)         |
| The four compute thread-id system values map to their GLSL gl\_\* builtins.                                                                                                      | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`compute-sysval-builtins-glsl.slang`](compute-sysval-builtins-glsl.slang)         |
| A fragment SV_Position input maps to gl_FragCoord and SV_IsFrontFace to gl_FrontFacing.                                                                                          | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`fragment-position-frontface-glsl.slang`](fragment-position-frontface-glsl.slang) |
| An unrecognized SV\_\* system-value semantic is rejected with E49999 by the GLSL entry-point legalizer (driven on the SPIR-V arm).                                               | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`unknown-system-value-semantic.slang`](unknown-system-value-semantic.slang)       |
| SV_VertexID/SV_InstanceID lower to gl_VertexIndex/gl_InstanceIndex with the base offset subtracted in GLSL.                                                                      | characterization | [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | [`vertex-id-base-offset-glsl.slang`](vertex-id-base-offset-glsl.slang)             |

## Unreachable gaps

- `slang-ir-legalize-types.cpp:1987` `UseOfUninitializedOpaqueHandle` and
  `:2283` `CooperativeMatrixUnsupportedCapture` are reachable only through
  very specific opaque-handle / cooperative-matrix capture shapes that are
  hard to drive cleanly from a single-file `slangc` invocation without
  first tripping an earlier semantic diagnostic; left for a follow-up
  cooperative-matrix-focused pass.
- `slang-ir-legalize-varying-params.cpp:2108` `Diagnostics::Unexpected`
  and `:2980` `SystemValueAttributeNotSupported` guard internal /
  target-attribute states that the front-end semantic checker rejects
  first (e.g. `SystemValueTypeIncompatible` surfaces as E30701 from
  `slang-check-shader.cpp` before legalization runs), so the legalizer
  arm is shadowed for normal CLI input.
- `diagnoseUnsupportedSystemVal` / `diagnoseUnsupportedUserVal`
  (`:1042`/`:1053`, `Diagnostics::Unimplemented`) fire only for targets
  whose varying-legalization table is incomplete for a given semantic;
  the reachable combinations overlap the front-end's own semantic
  rejection and were not separable into a clean single-file repro here.

## Doc gaps observed

| Anchor                                                                                                                                | Kind                  | Gap                                                                                                                                                                                                                                                                                                       | Suggested addition                                                                                                                                                                                                     |
| ------------------------------------------------------------------------------------------------------------------------------------- | --------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | undocumented-behavior | The doc notes the GLSL legalizer runs "despite the name" on the SPIR-V arm but does not state that it performs HLSL-to-Vulkan index rebasing — SV_VertexID becomes `gl_VertexIndex - gl_BaseVertex` (and SV_InstanceID likewise) so user code keeps HLSL's per-draw zero-based semantics.                 | Add a short note (and a one-line example) that SV_VertexID / SV_InstanceID are rebased by subtracting gl_BaseVertex / gl_BaseInstance on the GLSL/SPIR-V path, since the emitted value is not a bare `gl_VertexIndex`. |
| [#legalizeentrypointsforglsl-despite-the-name](../../../design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name) | missing-surface       | The doc describes the GLSL legalizer pass but does not enumerate the system-value-semantic to gl*\* builtin mapping table (SV_DispatchThreadID->gl_GlobalInvocationID, SV_Position(fragment)->gl_FragCoord, SV_IsFrontFace->gl_FrontFacing, etc.) nor the E49999 it raises for an unknown SV*\* semantic. | Add a mapping table of the recognized SV*\* semantics to their gl*\_ builtins per stage, and note that an unrecognized SV\_\_ semantic is rejected with error 49999 "unknown system-value semantic".                   |
