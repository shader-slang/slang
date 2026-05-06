# Slang Synthetic Resource Metadata: Initial Implementation

## Purpose

This note records the Slang-side implementation slice completed for
the synthetic-resource metadata direction discussed for shader
coverage, `slang-rhi`, and direct hosts such as Omniverse Kit.

The immediate goal was to give hosts a clean way to discover hidden
coverage bindings without reintroducing AST/reflection-visible coverage
resources.

## Implemented in Slang

Public API additions in `include/slang.h`:

- `slang::SyntheticResourceScope`
- `slang::SyntheticResourceAccess`
- `slang::SyntheticResourceInfo`
- `slang::ISyntheticResourceMetadata`
- `slang::SyntheticResourceDescriptorBindingInfo`
- `slang::SyntheticResourceUniformBindingInfo`

Artifact metadata plumbing:

- `ArtifactPostEmitMetadata` now implements
  `ISyntheticResourceMetadata`
- synthetic resource records are stored in
  `m_syntheticResources`
- `IMetadata::castAs(ISyntheticResourceMetadata)` works on compiled
  entry-point metadata

Coverage integration:

- coverage now emits one synthetic resource record for
  `__slang_coverage`
- the record includes:
  - stable id
  - `BindingType::MutableRawBuffer`
  - scope/access
  - descriptor-facing `(space, binding)`
  - debug name
  - feature tag `"coverage"`

Pipeline integration:

- early coverage preparation populates descriptor-facing fields
- a post-`collectGlobalUniformParameters` finalization hook attempts
  to populate backend-independent marshaling fields where available

## What Works Now

Hosts can query:

- coverage semantics through `ICoverageTracingMetadata`
- hidden bindable resources through `ISyntheticResourceMetadata`
- review-oriented helper APIs through `ISyntheticResourceMetadata`

For shader coverage specifically, a host now has a stable way to ask:

- does this compiled artifact contain a hidden coverage resource?
- what is its synthetic id?
- what bindable class is it?
- what `(space, binding)` should descriptor-backed backends use?
- what CPU/CUDA marshaling offset/stride should wrapper-data
  backends use?

The versioned helper query API now includes:

- `findResourceIndexByID(...)`
- `getResourceDescriptorBindingInfo(...)`
- `getResourceUniformBindingInfo(...)`

This is enough to support the intended first integration path for:

- Vulkan
- D3D12
- Metal
- direct descriptor-backed hosts such as Omniverse Kit

## CPU/CUDA Status

The initial gap on CPU/CUDA marshaling metadata is now closed for the
coverage buffer itself.

Important detail:

- metadata finalization now uses IR natural-layout queries on the
  synthesized `globalParams.__slang_coverage` field
- this yields the same marshaling offset/stride that ordinary
  reflected CPU/CUDA resource fields use
- on a simple validation probe, both `cpp` and `cuda` source targets
  report:
  - `uniformOffset = 16`
  - `uniformStride = 16`

This means the compiler-side metadata contract for coverage now
contains:

- descriptor-facing `(space, binding)`
- CPU/CUDA-style marshaling location

This matches the earlier design conclusion from the planning notes:

- descriptor-backed hosts can use `(space, binding)` now
- CPU/CUDA need additional synthetic-resource marshaling metadata, not
  just descriptor-facing metadata

## Why This Is Still Useful

This gets the architecture onto the right track without polluting
public reflection:

- hidden resources are queryable
- the query contract is generic, not coverage-only
- the design remains extensible for future synthetic resources such as
  printf buffers, profiling buffers, or sanitizers

It also gives a concrete Slang-side API surface for the next
`slang-rhi` phase.

## Recommended Next Step in `slang-rhi`

The preferred follow-up remains:

- consume `ISyntheticResourceMetadata` through a
  `ShaderProgramDesc.next` extension
- merge synthetic resources into `slang-rhi` internal program/layout
  state
- expose hidden-resource binding through existing binding paths and
  helper queries, rather than by adding a Vulkan-shaped raw
  `setExtraBinding(set, binding, ...)` API

## Remaining Work

The remaining work is primarily on the `slang-rhi` side:

- synthesize internal hidden binding ranges from
  `ISyntheticResourceMetadata`
- map synthetic resource ids to runtime `ShaderOffset`s
- expose helper queries/binding APIs without polluting the base
  reflection-driven object model

The compiler-side metadata now provides the key fields needed to do
that for coverage without requiring reflection-visible AST synthesis.
