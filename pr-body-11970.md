## Motivation

On `-target metal`, storing a bindless descriptor handle as a buffer *element* produces MSL that Apple's compiler rejects. Consider a Vulkan-style bindless shader ported to Metal:

```slang
StructuredBuffer<StructuredBuffer<uint>.Handle> indexBuffers;   // buffer of buffer-handles
StructuredBuffer<Texture2D<float4>.Handle>      textures;       // buffer of texture-handles

[shader("compute")]
void computeMain(uint tid : SV_GroupIndex)
{
    StructuredBuffer<uint> b = indexBuffers[tid];
    Texture2D<float4>      t = textures[tid];
    ...
}
```

Today this emits illegal buffer parameters:

- `indexBuffers` → `uint device* device* indexBuffers_0 [[buffer(0)]]`
- `textures`     → `texture2d<float, access::sample> device* textures_1 [[buffer(1)]]`

Both are rejected: *"type 'device uint *device *' / 'device texture2d<...> *' is not valid for attribute 'buffer'"*. The reporter confirmed the intended ABI is correct (it matches spirv-cross's argument-buffer-tier-2 `spvDescriptor<T>`); only the bare-pointer *spelling* is illegal, and wrapping the pointee in a one-member struct makes the identical code legal.

This PR fixes **Variant 3** of #11970 (`StructuredBuffer<DescriptorHandle<R>>`), which the assignee split out as the first of two PRs. Variants 1–2 (arrays of resource *bindings*, `R[N]` / `R[]`) are a separate, larger scope and are not addressed here.

## Proposed solution

The root cause is that `StructuredBuffer<E>` emits `emitType(E) + " device*"` (`slang-emit-metal.cpp:1431`), and a bindless `DescriptorHandle<R>` emits `R` directly, because Metal's `isResourceTypeBindless` always returns true (`slang-emit-metal.h:28` → `slang-emit-c-like.cpp:444`). So the element `R = StructuredBuffer<uint>` emits `uint device*`, and the outer buffer binding appends another `device*`.

There is already an IR pass whose documented job is exactly *"the buffer binding is `device*`, so any `T*` element becomes `device T* device*` which Metal rejects → lower it"*: `MetalPointerBufferElementTypeLoweringPolicy` (the third of three Metal buffer-element-lowering passes; it runs after `specializeAddressSpaceForMetal` and is immediately followed by a dedicated `performForceInlining` to materialize its pack/unpack helpers). But its discovery filter `needsElementLowering` only recurses into `IRPtrType` / array / struct — it never matches a bare `IRDescriptorHandleType`, so the descriptor element falls through un-lowered.

The fix extends that pass to wrap a descriptor-handle StorageBuffer element in a **one-member struct** `{ DescriptorHandle<R> handle; }`, so the outer buffer becomes `const device Wrapper*` — one legal level of indirection — while the handle emits legally as a *struct member* (`uint device* handle;` / `texture2d<...> handle;`, both accepted by Metal inside a `const device Wrapper*` binding). This is exactly the spvDescriptor<T> / argument-buffer-tier-2 layout the reporter identified, and it is ABI byte-identical: a single-field struct has the size and alignment of its field. It covers the buffer and texture sub-cases uniformly, because the wrapper is agnostic to the handle's resource type.

This is the principled layer: rather than special-casing emit, the fix makes the buffer element a legal representation upstream, keeping emission simple — consistent with how the same pass already lowers `device T* device*` pointers to `uintptr_t`, and how the framework lowers matrices / packed vectors into one-field storage structs.

## Change summary

| File | Change |
| --- | --- |
| `source/slang/slang-ir-lower-buffer-element-type.cpp` | In `MetalPointerBufferElementTypeLoweringPolicy`: (1) `needsElementLowering` now also matches `IRDescriptorHandleType`; (2) new `lowerDescriptorHandleElement` + three IR-builder helpers that create the one-member wrapper struct and its `[ForceInline]` pack/unpack converters; (3) `lowerLeafLogicalType` dispatches to it, gated on `AddressSpace::StorageBuffer`. |
| `tests/metal/bindless-descriptor-array-in-buffer.slang` | New regression test — `-target metal` (asserts the wrapper struct is emitted and there is no `device* device*` / bare `texture2d<...> device*` parameter left) and `-target metallib` (Apple's compiler must accept the MSL end-to-end). Exercises both buffer and texture sub-cases with indexed access. |

## Concepts and vocabulary

- **Buffer-element-type lowering** — a pre-emit IR pass (`lowerBufferElementTypeToStorageType`) that rewrites the *element type* of a buffer global into a target-legal *storage* type, inserting `[ForceInline]` pack/unpack conversion functions so load/store sites transparently convert between the logical and storage forms. Metal runs it three times with different policies: (1) `MetalParameterBlock` (resource fields → `DescriptorHandle` in ParameterBlocks), (2) `Metal` (matrix/bool/vector), (3) `MetalPointerLowering` (`device T*` fields → `uintptr_t`). This PR extends pass (3).
- **`DescriptorHandle<R>`** — a bindless handle to an opaque resource `R`. On Metal it emits as `R` directly (bindless), which is what makes it collapse into the outer buffer's `device*` and produce the illegal double pointer.
- **`AddressSpace::StorageBuffer`** — the address space `getTypeLoweringConfigForBuffer` assigns to `StructuredBuffer` / RW / append / consume buffers. Constant/argument buffers get `Uniform` and reflect native layout, so the wrapper is gated to fire only for StorageBuffer.
- **`isResourceTypeBindless`** — always true on Metal (`slang-emit-metal.h:28`), the reason a `DescriptorHandle<R>` element emits `R` directly instead of as a `uint2`.

## Process report

**Why the fix lives in `MetalPointerBufferElementTypeLoweringPolicy` (pass 3), not the emitter or an earlier pass.** The illegal `device T* device*` shape is created by two composing emit rules (`StructuredBuffer<E>` appends `device*`; a bindless handle emits its resource type). Fixing it in emit would mean pattern-matching the composed spelling — a symptom fix. Pass 3 already owns this exact invariant for pointers and runs late enough (after `specializeAddressSpaceForMetal`) that the buffer's address space is known, which the StorageBuffer gate requires. It is also the pass whose output helpers are materialized by the `performForceInlining` call that immediately follows it, so the `[ForceInline]` pack/unpack functions created here are inlined without any additional plumbing.

**Input-shape check.** The input shape handled is a top-level buffer element type that is an `IRDescriptorHandleType`, under a `StorageBuffer`-address-space buffer. Is that shape correct and principled, or should its producer be fixed instead? It is correct and intentional: `StructuredBuffer<DescriptorHandle<R>>` is exactly the *"buffer of handle data"* the user wrote and the reporter confirmed is the right meaning (as opposed to Variants 1–2, arrays of resource *bindings*, which are a genuine feature gap tracked separately). The producer is not wrong — the buffer legitimately contains handle data; only Metal's emit spelling for that element is illegal. So the right layer is precisely this element-legalization pass, which normalizes a valid-but-illegally-spelled element into a legal storage form, mirroring the existing pointer→`uintptr_t` rule. No upstream producer needs to change.

**Why widening `needsElementLowering` is safe.** The filter is documented as a conservative over-approximation: a false positive is harmless because `getLoweredTypeInfoImpl` computes the lowered type and, when nothing changed, returns the original (identity) so the buffer is skipped. For a descriptor handle under StorageBuffer the lowering is never identity (it always produces the wrapper struct), so the buffer is correctly rewritten; for a descriptor handle in a non-StorageBuffer context the `lowerLeafLogicalType` gate returns identity and the buffer is skipped. The one observable side effect of the widened filter — a ParameterBlock whose descriptor fields get re-visited — produces a structurally-equivalent type that is collapsed by `simplifyNonSSAIR`; this is compile-time IR churn only, matching the pre-existing TODO in the same class for pointer fields, with no runtime or ABI impact.

**Why returning a fresh struct from `lowerLeafLogicalType` is safe.** The struct is created once and entered into the framework's `mapLoweredTypeToInfo`; a subsequent query for an already-lowered type returns identity, so there is no infinite re-lowering and no double-wrap even though `shouldSkipPhysicalTypes()` is false for this pass.

**Constraints honored (per the assignee's directive).** The existing `IRPtrType → uintptr_t` rule is untouched (the descriptor branch is additive and returns before the pointer branch). Top-level `.Handle` *parameters* are unaffected — they are function parameters, not buffer elements, so they never reach this pass as a StorageBuffer element (the single-`.Handle`-param path from #11073 and its test `tests/metal/entry-point-descriptor-handle-buffer.slang` are unchanged). The wrapper covers buffer and texture handles uniformly. The whole change is gated on `AddressSpace::StorageBuffer`.

Addresses **Variant 3** of #11970. Per the assignee, this is the first of two PRs — the issue is intentionally kept open until Variant 1–2 (PR2) also lands, so this does **not** use a closing keyword.

---

<sub>🤖 Generated by an automated Slang coworker — may be inaccurate. A human maintainer should verify.</sub>
