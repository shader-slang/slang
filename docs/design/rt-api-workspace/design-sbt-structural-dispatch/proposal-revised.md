# Dynamic SBT Revision Rationale

Status: incorporated into [PROPOSAL.md](PROPOSAL.md). That document is the sole normative API and
lowering proposal. This file records the reasoning behind the dynamic-schema revision.

## 1. Why the model changed

An earlier design made the shader describe physical SBT record positions. Each hit, miss, or
callable declaration carried a compile-time slot. That model had two practical problems.

First, native SBTs commonly contain many runtime records that reuse a small number of programs:

```text
record 0    = OpaqueHitGroup + material 0
record 1    = OpaqueHitGroup + material 1
...
record 9999 = OpaqueHitGroup + material 9999
```

The repeated records are created from scene data. Requiring 10,000 shader declarations would turn a
runtime scene property into source-code boilerplate.

Second, record selection depends on runtime values:

```text
instanceContribution + geometryIndex * sbtStride + sbtOffset
```

The host builds the acceleration structure and chooses the SBT ordering. Shader source cannot be
the sole owner of those physical positions.

## 2. Final decomposition

The revision separates three concepts:

```text
shader-owned schema
    finite set of hit groups, miss shaders, and callable shaders
    contexts, payloads, primitives, and record types

compiler-owned mapping
    entry points and target symbols
    dense function indices for one linked program
    ABI sizes, Metal function tables, and dispatch adapters

host-owned SBT instance
    record counts and positions
    entry selected by each record
    record data and acceleration-structure contributions
```

`ITraceProgramSchema` therefore describes executable entries, not physical records. A host may
instantiate any number of records from those entries. Reflection is the bridge: it maps each source
entry to the identifiers and layouts needed by the target API.

There is one schema family rather than separate static and dynamic layouts. A fixed table is simply
a host instance whose record count and ordering do not change.

## 3. Why payload is not part of `ITraceContext`

Acceleration-structure topology and motion apply to the whole schema. Payload does not. A normal
program may trace radiance and shadow rays through the same SBT:

```text
SceneSchema
├── RadiancePayload partition
│   ├── radiance hit groups
│   └── radiance miss shaders
└── ShadowPayload partition
    ├── shadow hit groups
    └── shadow miss shaders
```

Payload therefore belongs to hit and miss stage contexts. `trace<Payload>` infers the type from its
`inout` payload argument, and the compiler verifies that the completed schema serves that type.
This keeps stage inputs strongly typed without duplicating a schema or descriptor for every payload.

Runtime SBT selectors cannot be type-checked against host-written records. A host can still direct a
shadow trace to a radiance record by constructing an invalid table. This is the same responsibility
boundary as native D3D, Vulkan, and OptiX SBTs; reflection makes the required partition mapping
available to the host.

## 4. Why hit groups keep their identity

A hit-group entry still associates one context with _ClosestHit_, _AnyHit_, and _Intersection_
stage implementations. Metal needs that relationship to synthesize candidate and committed-hit
dispatch.

What disappeared is only the physical slot. The compiler assigns each finalized group a dense
function index within its payload partition. Every runtime record using that group stores the same
function index but may carry different `Context.Record` data.

_Miss_ and _Callable_ entries do not need wrapper groups because their native records each bind one
stage. Their stage structs carry their contexts directly.

## 5. Why Metal dispatch is record-driven

Metal's intersection-function-table selection does not include the portable ray-type offset. A
separate IFT entry per source hit group would therefore be unable to choose different _AnyHit_
behavior for different runtime SBT records over the same geometry.

The revised lowering uses one generated IFT dispatcher per primitive kind and payload partition.
The dispatcher computes the portable physical record index, reads that record's function index, and
selects the corresponding hit-group arm. _ClosestHit_ and _Miss_ dispatch use the same record-owned
identity through visible function tables.

This makes one runtime record the source of truth for candidate, committed-hit, and record-data
selection on every target.

## 6. Open sections

A schema section may be closed or completed at Slang link time. An open section names a tag
interface; concrete linked types conforming to that tag join the finalized schema. This supports
separately compiled material modules while keeping the final entry set finite before target code is
generated.

Open entries are not a stable plugin ABI. Linking again may change function indices, just as
rebuilding a native ray-tracing pipeline may change shader identifiers. Hosts resolve reflected
entry names after each linked program is built.

## 7. Resulting contract

The revision preserves the original goal—shader source is the authoritative description of which
programs can participate in dispatch—without claiming ownership of scene-dependent SBT records.

The complete contracts, target lowering, tag inference, reflection, diagnostics, and initial scope
are now specified only in [PROPOSAL.md](PROPOSAL.md).
