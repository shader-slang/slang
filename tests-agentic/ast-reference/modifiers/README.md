---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T16:50:00+00:00
source_commit: bbd84dc65e58598bfa71fafe72764b4076b0869b
watched_paths_digest: 0da77494c7202daa683caa86dd59c3c52ee670047c4ae4266818e2cd8b88bf99
source_doc: docs/llm-generated/ast-reference/modifiers.md
source_doc_digest: ce606e060d178f7091bd26e74fee49fd1d2442061f54a0d3e8579cc174038e5e
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ast-reference/modifiers

## Intent

Tests verify the user-observable roles of concrete `Modifier` and
`Attribute` subclasses described in
[`docs/llm-generated/ast-reference/modifiers.md`](../../../docs/llm-generated/ast-reference/modifiers.md):
parameter direction (`in`/`out`/`inout`), storage class (`const`,
`static`, `groupshared`), visibility (`public`/`private`),
interpolation modes (`nointerpolation`, `sample`), matrix layout
(`row_major`), HLSL semantics (`: register(...)`), stage-specific
entry-point attributes (`[shader(...)]`, `[numthreads(...)]`),
compile-time hint attributes (`[unroll]`, `[ForceInline]`),
mutability (`[mutating]`), inheritance control (`[open]`),
differentiability (`[Differentiable]`, `no_diff`).

The doc enumerates a very large family of modifier/attribute classes.
Most carry no user-spellable surface — they are internal checker
flags, core-module bindings, or per-target intrinsic bindings. This
bundle picks the kinds whose role is naturally observable through
`slangc` text I/O at parse, check, or emit, and writes one small
test per role. The bundle is intentionally medium-sized (19 tests)
to span the major families without exhausting every concrete
subclass.

A mix of functional and negative tests is included: emit-text
`SIMPLE(filecheck=...)` directives for modifier spellings that
survive into HLSL/GLSL; `INTERPRET` directives for direction /
storage / mutability semantics that produce a runtime-observable
value; `DIAGNOSTIC_TEST` directives for misuse claims (`const`
assignment, `private` access from outside, non-mutating `this`
assignment, `inout` of a literal).

## Coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| A function-scope `static` variable retains its value across calls to the function. | functional | [#compatibility-and-hlsl-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#compatibility-and-hlsl-storage-class-modifiers) | [`static-local-persists-across-calls.slang`](static-local-persists-across-calls.slang) |
| The `groupshared` storage modifier survives lowering to emitted HLSL. | functional | [#compatibility-and-hlsl-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#compatibility-and-hlsl-storage-class-modifiers) | [`groupshared-emits-on-hlsl.slang`](groupshared-emits-on-hlsl.slang) |
| `[ForceInline]` on a small helper inlines it into the caller; the helper's name no longer appears in emitted HLSL. | functional | [#compile-time-hint-attributes-loops-branches-opt-levels](../../../docs/llm-generated/ast-reference/modifiers.md#compile-time-hint-attributes-loops-branches-opt-levels) | [`force-inline-on-hlsl.slang`](force-inline-on-hlsl.slang) |
| `[unroll]` on a loop appears as an `[unroll]` attribute in emitted HLSL. | functional | [#compile-time-hint-attributes-loops-branches-opt-levels](../../../docs/llm-generated/ast-reference/modifiers.md#compile-time-hint-attributes-loops-branches-opt-levels) | [`unroll-attribute-on-hlsl.slang`](unroll-attribute-on-hlsl.slang) |
| A `[Differentiable]` function can be passed to `fwd_diff` and produces a sensible derivative. | functional | [#differentiability-attributes](../../../docs/llm-generated/ast-reference/modifiers.md#differentiability-attributes) | [`differentiable-allows-fwd-diff.slang`](differentiable-allows-fwd-diff.slang) |
| `: register(...)` binding semantic propagates to emitted HLSL. | functional | [#hlsl-semantics-sv](../../../docs/llm-generated/ast-reference/modifiers.md#hlsl-semantics-sv) | [`register-semantic-emits-on-hlsl.slang`](register-semantic-emits-on-hlsl.slang) |
| An `[open]` interface accepts implementations; a basic implementation works through dynamic dispatch. | functional | [#inheritance-control-attributes](../../../docs/llm-generated/ast-reference/modifiers.md#inheritance-control-attributes) | [`sealed-interface-rejects-inheritance.slang`](sealed-interface-rejects-inheritance.slang) |
| `nointerpolation` on a pixel-shader varying input survives into emitted HLSL. | functional | [#interpolation-modes](../../../docs/llm-generated/ast-reference/modifiers.md#interpolation-modes) | [`nointerpolation-emits-on-hlsl.slang`](nointerpolation-emits-on-hlsl.slang) |
| `sample` interpolation modifier is preserved through emitted HLSL. | functional | [#interpolation-modes](../../../docs/llm-generated/ast-reference/modifiers.md#interpolation-modes) | [`sample-interpolation-emits-on-hlsl.slang`](sample-interpolation-emits-on-hlsl.slang) |
| `row_major` matrix-layout modifier survives into emitted GLSL layout. | functional | [#matrix-layout-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#matrix-layout-modifiers) | [`row-major-emits-on-hlsl.slang`](row-major-emits-on-hlsl.slang) |
| A non-`[mutating]` struct method cannot assign through `this`; the checker diagnoses it. | negative | [#mutability-autodiff-annotations](../../../docs/llm-generated/ast-reference/modifiers.md#mutability-autodiff-annotations) | [`nonmutating-rejects-this-assignment.slang`](nonmutating-rejects-this-assignment.slang) |
| `[mutating]` on a method allows assigning to fields through `this`; the mutation is visible after the call. | functional | [#mutability-autodiff-annotations](../../../docs/llm-generated/ast-reference/modifiers.md#mutability-autodiff-annotations) | [`mutating-allows-this-assignment.slang`](mutating-allows-this-assignment.slang) |
| A function without `[ForceInline]` survives as a callable in emitted HLSL. | functional | [#parameter-direction-and-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers) | [`extern-link-time-decl.slang`](extern-link-time-decl.slang) |
| An `in` parameter is a copy; mutation inside the callee does not propagate to the caller. | functional | [#parameter-direction-and-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers) | [`param-in-default.slang`](param-in-default.slang) |
| An `inout` parameter both reads the caller's value and writes back to it. | functional | [#parameter-direction-and-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers) | [`param-inout-reads-and-writes.slang`](param-inout-reads-and-writes.slang) |
| An `inout` parameter requires an l-value argument; passing a literal is diagnosed. | negative | [#parameter-direction-and-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers) | [`inout-rejects-rvalue.slang`](inout-rejects-rvalue.slang) |
| An `out` parameter writes a value back to the caller's variable. | functional | [#parameter-direction-and-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers) | [`param-out-writes-back.slang`](param-out-writes-back.slang) |
| `const` makes a local variable immutable; assigning to it is diagnosed. | negative | [#parameter-direction-and-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers) | [`const-rejects-assignment.slang`](const-rejects-assignment.slang) |
| `[numthreads(x,y,z)]` lowers to GLSL `layout(local_size_x=..., local_size_y=..., local_size_z=...)`. | functional | [#stage-specific-entry-point-attributes](../../../docs/llm-generated/ast-reference/modifiers.md#stage-specific-entry-point-attributes) | [`numthreads-emits-on-glsl.slang`](numthreads-emits-on-glsl.slang) |
| `[numthreads(x,y,z)]` workgroup dimensions appear in emitted HLSL. | functional | [#stage-specific-entry-point-attributes](../../../docs/llm-generated/ast-reference/modifiers.md#stage-specific-entry-point-attributes) | [`numthreads-emits-on-hlsl.slang`](numthreads-emits-on-hlsl.slang) |
| `[shader("compute")]` marks a function as a compute entry point selectable without -entry. | functional | [#stage-specific-entry-point-attributes](../../../docs/llm-generated/ast-reference/modifiers.md#stage-specific-entry-point-attributes) | [`shader-entrypoint-attribute.slang`](shader-entrypoint-attribute.slang) |
| `no_diff` type modifier suppresses differentiation through a parameter. | functional | [#type-modifiers-wrapping-the-type-rather-than-the-declaration](../../../docs/llm-generated/ast-reference/modifiers.md#type-modifiers-wrapping-the-type-rather-than-the-declaration) | [`no-diff-suppresses-derivative.slang`](no-diff-suppresses-derivative.slang) |
| A `private` member is not accessible from outside its enclosing type. | negative | [#visibility-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#visibility-modifiers) | [`private-rejects-outside-access.slang`](private-rejects-outside-access.slang) |
| A `public` member is accessible from outside its enclosing type and a `public` function from anywhere. | functional | [#visibility-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#visibility-modifiers) | [`public-allows-cross-module-access.slang`](public-allows-cross-module-access.slang) |

## Out of scope

Many claims in `modifiers.md` describe **internal AST shape** that
is not user-observable through `slangc`. These are not testable
through this bundle's directives and are recorded here:

- C++ class identity of the parser-allocated modifier (e.g. that
  `groupshared` becomes `HLSLGroupSharedModifier` specifically and
  not a synonym; that `[unroll]` becomes `UnrollAttribute`).
- Parent class in the C++ hierarchy (`InOutModifier` derives from
  `OutModifier`; `Attribute` derives from `AttributeBase` derives
  from `Modifier`; `HLSLRegisterSemantic` derives from
  `HLSLLayoutSemantic` derives from `HLSLSemantic`).
- Private/key field names and types (e.g. the `irOp: uint32_t`
  field on `IntrinsicOpModifier`, the bitmask on
  `MemoryQualifierSetModifier`, the version `Token` on
  `GLSLVersionDirective`).
- Abstract intermediates that carry no user spelling
  (`VisibilityModifier`, `InterpolationModeModifier`,
  `MatrixLayoutModifier`, `HLSLSemantic`, `TypeModifier`,
  `AttributeBase`, `InheritanceControlAttribute`,
  `RayPayloadAccessSemantic`).
- Internal-only modifiers (`ToBeSynthesizedModifier`,
  `SynthesizedModifier`, `IgnoreForLookupModifier`,
  `VarReassignedModifier`, `ExistentialOpenedOnVarModifier`,
  `LocalTempVarModifier`, `ActualGlobalModifier`,
  `IsOverridingModifier`, `OptionalConstraintModifier`).
- Core-module / target binding modifiers (`IntrinsicOpModifier`,
  `TargetIntrinsicModifier`, `SpecializedForTargetModifier`,
  `BuiltinTypeModifier`, `MagicTypeModifier`,
  `BuiltinAttribute`, `AutoDiffBuiltinAttribute`,
  `KnownBuiltinAttribute`) — not user-spellable outside the core
  module.
- `UncheckedAttribute` is the parser-time shape before checker
  resolves it; user-visible behavior is the resolved attribute.
- `MemoryQualifierSetModifier` bitmask aggregation is a checker-
  internal representation; user observation is the resulting
  GLSL emit (where individual qualifiers appear).
- The mermaid "Family hierarchy" graph as a graph — the topology
  is structural metadata about the class hierarchy, not user
  behavior.

## Out of scope (no-GPU runner)

- Ray-payload semantics (`RayPayloadReadSemantic`,
  `RayPayloadWriteSemantic`, `VulkanRayPayloadAttribute`) — exercise
  a ray-tracing pipeline; the no-GPU runner cannot dispatch them.
- `[earlydepthstencil]`, `[maxvertexcount]`, geometry/tessellation
  stage attributes — observable through emit text only on
  specific pipeline stages whose entry-point signatures need GPU
  pipelines to dispatch.
- CUDA / Python / FFI attributes (`[CudaKernel]`, `[CudaHost]`,
  `[TorchEntryPoint]`, `[PyExport]`, `[DllImport]`,
  `[AutoPyBindCuda]`) — require a CUDA toolchain or python
  runtime that the no-GPU runner does not have.
- GLSL `layout(...)` family (`[vk::binding(...)]`,
  `[vk::location(...)]`, `[push_constant]`, etc.) when the role
  is best observed via SPIR-V validation on a real Vulkan driver.
  Emit-text observations for these are partially covered by
  sibling bundles; we omit duplicates here.

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#hlslnointerpolationmodifier](../../../docs/llm-generated/ast-reference/modifiers.md#hlslnointerpolationmodifier) | undocumented-behavior | The doc enumerates `HLSLNoInterpolationModifier`, `HLSLNoPerspectiveModifier`, `HLSLLinearModifier`, `HLSLSampleModifier`, `HLSLCentroidModifier`, `PerVertexModifier` but does not specify which combinations are mutually exclusive. A reader cannot tell from the doc whether `nointerpolation sample` on the same field is valid or diagnosed. |  |
| [#internal-synthesized-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#internal-synthesized-modifiers) | undocumented-behavior | The "Internal / synthesized modifiers" subsection lists modifiers but the doc does not state from which checker pass each one is introduced. A reader cannot tell whether `IgnoreForLookupModifier` is set at parse, check, or IR-lowering time. |  |
| [#forceinline](../../../docs/llm-generated/ast-reference/modifiers.md#forceinline) | undocumented-behavior | `[ForceInline]` is documented as `ForceInlineAttribute` but the doc does not state whether the helper function survives in emitted text (helpful guarantee for downstream tooling). The bundle's `force-inline-on-hlsl.slang` documents the observed behavior but the claim itself is not in the doc. |  |
| [#openattribute](../../../docs/llm-generated/ast-reference/modifiers.md#openattribute) | undocumented-behavior | The doc mentions `OpenAttribute` and `SealedAttribute` under `InheritanceControlAttribute` but does not specify the default for an interface that carries neither — i.e. whether a bare `interface` is implicitly open or sealed. |  |
| [#modern-language-defaults-to-internal](../../../docs/llm-generated/ast-reference/modifiers.md#modern-language-defaults-to-internal) | undocumented-behavior | The visibility default rule ("modern language defaults to `internal`") is mentioned only in the "Visibility modifiers and language version" subsection. A reader looking at the `PublicModifier` / `PrivateModifier` / `InternalModifier` row in the main table would not know the default is per-module-language. |  |
