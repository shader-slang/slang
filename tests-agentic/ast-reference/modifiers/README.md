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

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                                          | Claim (one line)                                                                                                          | Tests                                          |
| -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------- |
| C-01     | [#parameter-direction-and-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers)                      | `OutModifier` makes a parameter write back to the caller's variable.                                                      | [`param-out-writes-back.slang`](param-out-writes-back.slang)                  |
| C-02     | [#parameter-direction-and-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers)                      | `InOutModifier` reads the caller's value and writes back.                                                                 | [`param-inout-reads-and-writes.slang`](param-inout-reads-and-writes.slang)           |
| C-03     | [#parameter-direction-and-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers)                      | `InModifier` (the default) passes by value; callee mutation does not propagate.                                           | [`param-in-default.slang`](param-in-default.slang)                       |
| C-04     | [#parameter-direction-and-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers)                      | `ConstModifier` makes a binding immutable; assignment is diagnosed.                                                       | [`const-rejects-assignment.slang`](const-rejects-assignment.slang)               |
| C-05     | [#parameter-direction-and-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers)                      | `InOutModifier` requires an l-value argument; r-value is diagnosed.                                                       | [`inout-rejects-rvalue.slang`](inout-rejects-rvalue.slang)                   |
| C-06     | [#compatibility-and-hlsl-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#compatibility-and-hlsl-storage-class-modifiers)                        | `HLSLStaticModifier` on a local variable persists state across calls.                                                     | [`static-local-persists-across-calls.slang`](static-local-persists-across-calls.slang)     |
| C-07     | [#compatibility-and-hlsl-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#compatibility-and-hlsl-storage-class-modifiers)                        | `HLSLGroupSharedModifier` (`groupshared`) survives into emitted HLSL.                                                     | [`groupshared-emits-on-hlsl.slang`](groupshared-emits-on-hlsl.slang)              |
| C-08     | [#stage-specific-entry-point-attributes](../../../docs/llm-generated/ast-reference/modifiers.md#stage-specific-entry-point-attributes)                                          | `NumThreadsAttribute` workgroup dimensions appear in emitted HLSL.                                                        | [`numthreads-emits-on-hlsl.slang`](numthreads-emits-on-hlsl.slang)               |
| C-09     | [#stage-specific-entry-point-attributes](../../../docs/llm-generated/ast-reference/modifiers.md#stage-specific-entry-point-attributes)                                          | `NumThreadsAttribute` lowers to GLSL `local_size_x/y/z`.                                                                  | [`numthreads-emits-on-glsl.slang`](numthreads-emits-on-glsl.slang)               |
| C-10     | [#stage-specific-entry-point-attributes](../../../docs/llm-generated/ast-reference/modifiers.md#stage-specific-entry-point-attributes)                                          | `EntryPointAttribute` (`[shader("compute")]`) marks a compute entry point selectable without `-entry`.                    | [`shader-entrypoint-attribute.slang`](shader-entrypoint-attribute.slang)            |
| C-11     | [#compile-time-hint-attributes-loops-branches-opt-levels](../../../docs/llm-generated/ast-reference/modifiers.md#compile-time-hint-attributes-loops-branches-opt-levels)        | `UnrollAttribute` (`[unroll]`) survives into emitted HLSL.                                                                | [`unroll-attribute-on-hlsl.slang`](unroll-attribute-on-hlsl.slang)               |
| C-12     | [#compile-time-hint-attributes-loops-branches-opt-levels](../../../docs/llm-generated/ast-reference/modifiers.md#compile-time-hint-attributes-loops-branches-opt-levels)        | `ForceInlineAttribute` (`[ForceInline]`) inlines a helper at the call site.                                               | [`force-inline-on-hlsl.slang`](force-inline-on-hlsl.slang)                   |
| C-13     | [#parameter-direction-and-storage-class-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers)                      | Without `[ForceInline]`, a user-defined helper survives as a callable in emitted HLSL.                                    | [`extern-link-time-decl.slang`](extern-link-time-decl.slang)                  |
| C-14     | [#interpolation-modes](../../../docs/llm-generated/ast-reference/modifiers.md#interpolation-modes)                                                                              | `HLSLNoInterpolationModifier` survives into emitted HLSL.                                                                 | [`nointerpolation-emits-on-hlsl.slang`](nointerpolation-emits-on-hlsl.slang)          |
| C-15     | [#interpolation-modes](../../../docs/llm-generated/ast-reference/modifiers.md#interpolation-modes)                                                                              | `HLSLSampleModifier` survives into emitted HLSL.                                                                          | [`sample-interpolation-emits-on-hlsl.slang`](sample-interpolation-emits-on-hlsl.slang)     |
| C-16     | [#matrix-layout-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#matrix-layout-modifiers)                                                                      | `HLSLRowMajorLayoutModifier` (`row_major`) survives into emitted HLSL.                                                    | [`row-major-emits-on-hlsl.slang`](row-major-emits-on-hlsl.slang)                |
| C-17     | [#hlsl-semantics-sv](../../../docs/llm-generated/ast-reference/modifiers.md#hlsl-semantics-sv)                                                                                  | `HLSLRegisterSemantic` (`: register(uN)`) survives into emitted HLSL.                                                     | [`register-semantic-emits-on-hlsl.slang`](register-semantic-emits-on-hlsl.slang)        |
| C-18     | [#mutability-autodiff-annotations](../../../docs/llm-generated/ast-reference/modifiers.md#mutability-autodiff-annotations)                                                      | `MutatingAttribute` (`[mutating]`) allows assignment through `this`; the mutation is observable to the caller.            | [`mutating-allows-this-assignment.slang`](mutating-allows-this-assignment.slang)        |
| C-19     | [#mutability-autodiff-annotations](../../../docs/llm-generated/ast-reference/modifiers.md#mutability-autodiff-annotations)                                                      | A method without `[mutating]` cannot assign through `this`; assignment is diagnosed.                                      | [`nonmutating-rejects-this-assignment.slang`](nonmutating-rejects-this-assignment.slang)    |
| C-20     | [#differentiability-attributes](../../../docs/llm-generated/ast-reference/modifiers.md#differentiability-attributes)                                                            | `DifferentiableAttribute` allows the function to be passed to `fwd_diff` and to produce a derivative.                     | [`differentiable-allows-fwd-diff.slang`](differentiable-allows-fwd-diff.slang)         |
| C-21     | [#type-modifiers-wrapping-the-type-rather-than-the-declaration](../../../docs/llm-generated/ast-reference/modifiers.md#type-modifiers-wrapping-the-type-rather-than-the-declaration) | `NoDiffModifier` (`no_diff`) on a parameter type suppresses differentiation through that parameter.                       | [`no-diff-suppresses-derivative.slang`](no-diff-suppresses-derivative.slang)          |
| C-22     | [#visibility-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#visibility-modifiers)                                                                            | `PrivateModifier` hides a member; access from outside diagnoses.                                                          | [`private-rejects-outside-access.slang`](private-rejects-outside-access.slang)         |
| C-23     | [#visibility-modifiers](../../../docs/llm-generated/ast-reference/modifiers.md#visibility-modifiers)                                                                            | `PublicModifier` exposes a member to outside callers; reading a `public` field through normal member access succeeds.     | [`public-allows-cross-module-access.slang`](public-allows-cross-module-access.slang)      |
| C-24     | [#inheritance-control-attributes](../../../docs/llm-generated/ast-reference/modifiers.md#inheritance-control-attributes)                                                        | `OpenAttribute` (`[open]`) on an interface allows implementation and interface-typed dispatch.                            | [`sealed-interface-rejects-inheritance.slang`](sealed-interface-rejects-inheritance.slang)   |

## Tests in this bundle

| File                                          | Intent     | Doc anchor                                                       |
| --------------------------------------------- | ---------- | ---------------------------------------------------------------- |
| [`param-out-writes-back.slang`](param-out-writes-back.slang)                 | functional | `#parameter-direction-and-storage-class-modifiers`               |
| [`param-inout-reads-and-writes.slang`](param-inout-reads-and-writes.slang)          | functional | `#parameter-direction-and-storage-class-modifiers`               |
| [`param-in-default.slang`](param-in-default.slang)                      | functional | `#parameter-direction-and-storage-class-modifiers`               |
| [`const-rejects-assignment.slang`](const-rejects-assignment.slang)              | negative   | `#parameter-direction-and-storage-class-modifiers`               |
| [`inout-rejects-rvalue.slang`](inout-rejects-rvalue.slang)                  | negative   | `#parameter-direction-and-storage-class-modifiers`               |
| [`static-local-persists-across-calls.slang`](static-local-persists-across-calls.slang)    | functional | `#compatibility-and-hlsl-storage-class-modifiers`                |
| [`groupshared-emits-on-hlsl.slang`](groupshared-emits-on-hlsl.slang)             | functional | `#compatibility-and-hlsl-storage-class-modifiers`                |
| [`numthreads-emits-on-hlsl.slang`](numthreads-emits-on-hlsl.slang)              | functional | `#stage-specific-entry-point-attributes`                         |
| [`numthreads-emits-on-glsl.slang`](numthreads-emits-on-glsl.slang)              | functional | `#stage-specific-entry-point-attributes`                         |
| [`shader-entrypoint-attribute.slang`](shader-entrypoint-attribute.slang)           | functional | `#stage-specific-entry-point-attributes`                         |
| [`unroll-attribute-on-hlsl.slang`](unroll-attribute-on-hlsl.slang)              | functional | `#compile-time-hint-attributes-loops-branches-opt-levels`        |
| [`force-inline-on-hlsl.slang`](force-inline-on-hlsl.slang)                  | functional | `#compile-time-hint-attributes-loops-branches-opt-levels`        |
| [`extern-link-time-decl.slang`](extern-link-time-decl.slang)                 | functional | `#parameter-direction-and-storage-class-modifiers`               |
| [`nointerpolation-emits-on-hlsl.slang`](nointerpolation-emits-on-hlsl.slang)         | functional | `#interpolation-modes`                                           |
| [`sample-interpolation-emits-on-hlsl.slang`](sample-interpolation-emits-on-hlsl.slang)    | functional | `#interpolation-modes`                                           |
| [`row-major-emits-on-hlsl.slang`](row-major-emits-on-hlsl.slang)               | functional | `#matrix-layout-modifiers`                                       |
| [`register-semantic-emits-on-hlsl.slang`](register-semantic-emits-on-hlsl.slang)       | functional | `#hlsl-semantics-sv`                                             |
| [`mutating-allows-this-assignment.slang`](mutating-allows-this-assignment.slang)       | functional | `#mutability-autodiff-annotations`                               |
| [`nonmutating-rejects-this-assignment.slang`](nonmutating-rejects-this-assignment.slang)   | negative   | `#mutability-autodiff-annotations`                               |
| [`differentiable-allows-fwd-diff.slang`](differentiable-allows-fwd-diff.slang)        | functional | `#differentiability-attributes`                                  |
| [`no-diff-suppresses-derivative.slang`](no-diff-suppresses-derivative.slang)         | functional | `#type-modifiers-wrapping-the-type-rather-than-the-declaration`  |
| [`private-rejects-outside-access.slang`](private-rejects-outside-access.slang)        | negative   | `#visibility-modifiers`                                          |
| [`public-allows-cross-module-access.slang`](public-allows-cross-module-access.slang)     | functional | `#visibility-modifiers`                                          |
| [`sealed-interface-rejects-inheritance.slang`](sealed-interface-rejects-inheritance.slang)  | functional | `#inheritance-control-attributes`                                |

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

- The doc enumerates `HLSLNoInterpolationModifier`,
  `HLSLNoPerspectiveModifier`, `HLSLLinearModifier`,
  `HLSLSampleModifier`, `HLSLCentroidModifier`, `PerVertexModifier`
  but does not specify which combinations are mutually exclusive.
  A reader cannot tell from the doc whether
  `nointerpolation sample` on the same field is valid or
  diagnosed.
- The "Internal / synthesized modifiers" subsection lists modifiers
  but the doc does not state from which checker pass each one is
  introduced. A reader cannot tell whether `IgnoreForLookupModifier`
  is set at parse, check, or IR-lowering time.
- `[ForceInline]` is documented as `ForceInlineAttribute` but the
  doc does not state whether the helper function survives in
  emitted text (helpful guarantee for downstream tooling). The
  bundle's `force-inline-on-hlsl.slang` documents the observed
  behavior but the claim itself is not in the doc.
- The doc mentions `OpenAttribute` and `SealedAttribute` under
  `InheritanceControlAttribute` but does not specify the default
  for an interface that carries neither — i.e. whether a bare
  `interface` is implicitly open or sealed.
- The visibility default rule ("modern language defaults to
  `internal`") is mentioned only in the "Visibility modifiers and
  language version" subsection. A reader looking at the
  `PublicModifier` / `PrivateModifier` / `InternalModifier` row in
  the main table would not know the default is per-module-language.
