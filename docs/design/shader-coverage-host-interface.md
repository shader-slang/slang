Shader Coverage Host Interface
==============================

This document describes the host-facing interface used for hidden
synthetic resources introduced by shader coverage, and how the same
contract is intended to serve:

- direct hosts that do not use `slang-rhi`
- `slang-rhi` itself
- descriptor-backed backends such as Vulkan, Metal, and D3D12
- uniform-marshaling backends such as CUDA and CPU

Problem
-------

Shader coverage injects a hidden bindable resource,
`__slang_coverage`. That resource must be:

- discoverable by hosts
- bindable by hosts
- attributable for reporting

The current design keeps coverage reflection-invisible and instead
uses two explicit metadata channels:

- `ICoverageTracingMetadata`
  - coverage semantics and attribution
- `ISyntheticResourceMetadata`
  - hidden bindable resource layout and binding data

Design split
------------

### 1. Coverage semantics

`ICoverageTracingMetadata` answers:

- how many counters exist
- what source file/line each counter maps to
- what buffer binding was assigned for coverage
- how to serialize the canonical coverage manifest

This is the coverage-specific reporting layer.

### 2. Hidden binding semantics

`ISyntheticResourceMetadata` answers:

- what hidden bindable resources exist
- what kind they are
- what descriptor-facing binding they use
- what CPU/CUDA marshaling location they use

This is the generic hidden-resource binding layer.

That split is intentional:

- `ICoverageTracingMetadata` stays coverage-specific
- `ISyntheticResourceMetadata` can support future features such as
  `printf`, profiling buffers, or sanitizers

Slang-side API
--------------

### Coverage metadata object

The coverage-specific public object is:

- `slang::ICoverageTracingMetadata`

Its intended use is:

- coverage reporting
- slot-to-source attribution
- coverage manifest serialization
- coverage-specific buffer information

The coverage query methods are:

- `getCounterCount()`
- `getEntryInfo(index, ...)`
- `getBufferInfo(...)`

These answer:

- how many counters exist
- what each counter slot means
- what coverage-specific buffer binding was assigned

Hosts use `ICoverageTracingMetadata` when they need to interpret the
counter values they read back, emit LCOV or manifest output, or inspect
the coverage-specific buffer view directly.

### Synthetic resource metadata object

The hidden-binding public object is:

- `slang::ISyntheticResourceMetadata`

and its primary payload struct is:

- `slang::SyntheticResourceInfo`

Current key fields:

- `id`
  - stable synthetic resource identifier within the compiled program
- `bindingType`
  - Slang binding kind
- `arraySize`
- `scope`
  - `Global` or `EntryPoint`
- `access`
  - `Read`, `Write`, `ReadWrite`
- `entryPointIndex`
- descriptor-facing binding:
  - `space`
  - `binding`
- CPU/CUDA marshaling location:
  - `uniformOffset`
  - `uniformStride`
- `debugName`

Coverage currently emits exactly one synthetic resource record for the
hidden `__slang_coverage` buffer. Hosts should treat its synthetic
resource id as an opaque, stable, non-zero identifier returned by the
metadata queries, rather than hardcoding a literal numeric value.

### Query functions

The core query methods are:

- `getResourceCount()`
- `getResourceInfo(index, ...)`
- `findResourceIndexByID(id, ...)`
- `getResourceDescriptorBindingInfo(index, ...)`
- `getResourceUniformBindingInfo(index, ...)`

This gives two levels of access:

- full record access through `SyntheticResourceInfo`
- review-friendly direct helper queries for the two practical host
  cases

### Raw binding queries

The raw binding API is the low-level query surface that exposes the
actual hidden binding locations without adding any descriptor-layout
policy on top.

For descriptor-backed paths:

- `getResourceDescriptorBindingInfo(index, ...)`
  - returns the explicit `(space, binding)` location and descriptor-
    facing binding properties for the synthetic resource

For CPU/CUDA-style marshaling paths:

- `getResourceUniformBindingInfo(index, ...)`
  - returns `uniformOffset` and `uniformStride`

For hosts that already own their runtime binding logic, these are the
primary low-level APIs. The descriptor helper functions below are an
optional convenience layer built on top of the same metadata.

### Slang helper functions for descriptor-backed hosts

To reduce host-side duplication of `BindingType -> descriptor class`
logic, Slang also provides helper functions in `slang.h`:

- `getSyntheticResourceDescriptorClass(...)`
- `getSyntheticResourceDescriptorRange(...)`
- `findSyntheticResourceDescriptorRangeByID(...)`
- `getSyntheticResourceDescriptorSpaceSpan(...)`
- `getSyntheticResourceDescriptorRangeCountForSpace(...)`
- `getSyntheticResourceDescriptorRangesForSpace(...)`

These are not a second metadata channel. They are convenience helpers
on top of `ISyntheticResourceMetadata`, mainly for Vulkan-style
descriptor-layout construction.

Backend usage
-------------

| Backend / host style | Primary query path | Typical helper path |
|---|---|---|
| Vulkan / Metal / D3D12 / direct descriptor-backed hosts | `getResourceDescriptorBindingInfo(...)` | `getSyntheticResourceDescriptorRange(...)`, `findSyntheticResourceDescriptorRangeByID(...)`, `getSyntheticResourceDescriptorRangesForSpace(...)` |
| CUDA / CPU-style marshaling hosts | `getResourceUniformBindingInfo(...)` | `getResourceInfo(...)` when the host wants the full record |
| `slang-rhi` Vulkan / CUDA backends | `getResourceInfo(...)` while building `ShaderProgramSyntheticResourcesDesc` | `bindSyntheticResource(...)` after `ISyntheticShaderProgram` resolves the location |

`slang-rhi` consumption model
-----------------------------

The companion `slang-rhi` implementation is tracked in
`shader-slang/slang-rhi#739`.

The intended `slang-rhi` contract is:

1. Slang compiles the shader and exposes:
   - `ICoverageTracingMetadata`
   - `ISyntheticResourceMetadata`
2. The host converts synthetic-resource metadata into
   `ShaderProgramSyntheticResourcesDesc`
3. `slang-rhi` consumes that through `ShaderProgramDesc.next`
4. backend layouts append synthetic bindings into their normal
   internal layout model
5. `slang-rhi` exposes resolved binding locations through:
   - `ISyntheticShaderProgram`
6. runtime binding uses:
   - `bindSyntheticResource(...)`
   - or direct `IShaderObject::setBinding(location.offset, ...)`

The important design choice is that `slang-rhi` does not introduce a
separate backend-specific raw binding model in its core API. Instead,
it maps hidden resources into ordinary resolved `ShaderOffset`s.

That keeps the runtime binding model uniform.

Binding styles
--------------

The current interface supports two host-side binding styles.

### 1. Explicit binding

The host reads the hidden resource location and binds it directly:

- direct descriptor-backed hosts use `(space, binding)`
- direct CPU/CUDA-style hosts use `uniformOffset` and
  `uniformStride`
- `slang-rhi` hosts use the resolved location from
  `ISyntheticShaderProgram` and then call:
  - `bindSyntheticResource(...)`
  - or `IShaderObject::setBinding(location.offset, ...)`

This is the lowest-level path. It is appropriate for hosts that
already own their descriptor layout or parameter-marshaling logic and
just need an accurate hidden-resource contract from Slang.

### 2. Helper-assisted binding

The host uses helper functions layered on top of the same metadata:

- direct descriptor-backed hosts can use:
  - `getSyntheticResourceDescriptorRange(...)`
  - `findSyntheticResourceDescriptorRangeByID(...)`
  - `getSyntheticResourceDescriptorRangesForSpace(...)`
- `slang-rhi` hosts can use:
  - `bindSyntheticResource(...)`

This path is meant to reduce the amount of raw binding code a
reflection-driven codebase has to write, while still keeping the
resource out of normal reflection.

Direct-host model without `slang-rhi`
-------------------------------------

For direct hosts, the intended usage is:

1. compile via Slang C++ API with coverage enabled
2. get the linked or compiled artifact's `IMetadata`
3. call `castAs(...)` on that metadata to obtain:
   - `ICoverageTracingMetadata`
   - `ISyntheticResourceMetadata`
4. query `ICoverageTracingMetadata` for:
   - counter count
   - slot-to-source attribution
   - coverage buffer info when the host wants the coverage-specific
     binding view
5. query `ISyntheticResourceMetadata` for the hidden binding contract:
   - `getResourceCount()`
   - `getResourceInfo(...)`
   - `getResourceDescriptorBindingInfo(...)` for descriptor-backed
     paths
   - `getResourceUniformBindingInfo(...)` for CPU/CUDA-style
     marshaling paths
6. if the host wants a higher-level descriptor helper layer, call the
   free helper functions in `slang.h` on the same metadata object:
   - `getSyntheticResourceDescriptorRange(...)`
   - `findSyntheticResourceDescriptorRangeByID(...)`
   - `getSyntheticResourceDescriptorRangesForSpace(...)`
7. allocate and bind the hidden coverage buffer
8. dispatch
9. read counters back
10. use `ICoverageTracingMetadata` for reporting, LCOV, or manifest
    generation
