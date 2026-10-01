# Runtime-Sized Shader Binding Tables

Status: supporting rationale for [PROPOSAL.md](PROPOSAL.md). The proposal is normative.

## 1. Problem

A loaded scene commonly needs thousands of SBT records but only a few compiled shader groups:

```text
record 0   = OpaqueHitGroup + MaterialRecord { materialIndex = 12 }
record 1   = OpaqueHitGroup + MaterialRecord { materialIndex = 48 }
...
record 999 = OpaqueHitGroup + MaterialRecord { materialIndex = 6 }
```

Shader source cannot enumerate a scene-sized record table. D3D, Vulkan, and OptiX let the host
repeat one compiled group identifier in any number of records. The structural API must preserve
that model while still telling Metal every function that may be needed.

## 2. Schema Versus Instance

The design separates two objects:

```text
ITraceProgramSchema                         compile time
├── hit-group entries
├── miss-shader entries
└── callable-shader entries

host SBT instance                           runtime
├── record count and positions
├── entry selected by each record
└── data carried by each record
```

The schema is the shader-side source of truth for the finite executable set. A closed section lists
that set directly; an open section completes it from linked conformances. The host owns the scene
instance of the schema.

## 3. Shader-Side Contract

`IHitGroup` describes code and a record type, not one record occurrence:

```slang
public interface IHitGroup
{
    associatedtype Context;
    associatedtype ClosestHit;
    associatedtype AnyHit;
    associatedtype Intersection;
    __constraint Context : IHitContext;
    __constraint ClosestHit : IClosestHitShader;
    __constraint AnyHit : IAnyHitShader;
    __constraint Intersection : IIntersectionStage;
    __constraint ClosestHit.Context == Context;
    __constraint AnyHit.Context == Context;
    __constraint Intersection.Context == Context;
}
```

Miss and callable sections list shader types directly. Every stage exposes an associated `Context`;
the context supplies its trace context and record type, plus a payload for hit and miss stages.

One group is declared once:

```slang
struct OpaqueHitGroup : rt::IHitGroup
{
    typealias Context = OpaqueHitContext;
    typealias ClosestHit = OpaqueClosestHit;
    typealias AnyHit = rt::NoAnyHit<OpaqueHitContext>;
    typealias Intersection = rt::NoIntersection<OpaqueHitContext>;
}

struct SceneSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = SceneTraceContext;
    typealias HitGroups = rt::HitGroupList<OpaqueHitGroup, AlphaHitGroup>;
    typealias MissShaders = rt::MissShaderList<SkyMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}
```

Repeating `OpaqueHitGroup` in runtime records does not generate additional stage bodies.

## 4. Function Index And Record Index

These indices have different owners:

```text
functionIndex    compiler-assigned identity of one schema entry
recordIndex      host-defined position of one runtime record
```

Hit and miss function indices are dense within each payload partition. Callable indices are
program-wide. Reflection exposes the index with the entry's qualified name. Hosts use the name as
the stable key and resolve its numeric index after each link.

One function index may occur in any number of runtime records. Each occurrence carries an
independent `Context.Record` value.

## 5. Host Construction

Reflection supplies every entry's payload partition, function index, record type, and target
layout. A schema-aware builder can therefore express:

```text
setHitRecord(recordIndex, entryName, recordValue)
setMissRecord(recordIndex, entryName, recordValue)
setCallableRecord(recordIndex, entryName, recordValue)
```

The builder checks entry membership, data type, size, alignment, and section bounds. It cannot add
new shader code; adding linked entries requires linking a new program.

The hit record selected by a ray is:

```text
recordIndex =
    instanceContribution
    + geometryIndex * sbtStride
    + sbtOffset
```

The host supplies the acceleration-structure contribution and table contents. The trace supplies
the runtime stride and offset through `RayTraversalDesc`.

## 6. Native Targets

D3D, Vulkan, and OptiX already encode a complete record instance:

```text
native record
├── shader-group identifier or handle
└── target encoding of Context.Record
```

The same identifier may be repeated with different record values. Existing native traversal uses
the host-built SBT directly; the compiler only generates and reflects the stage entries.

OptiX follows the same structural split between a compiled program entry and host-created record
instances.

## 7. Metal Runtime Records

Metal uses fixed function tables plus a runtime-sized record buffer. For each payload the compiler
generates one `_Miss_` table and one `_ClosestHit_` table containing one function per schema entry.
When any hit group of that payload uses `NoClosestHit`, one shared no-op function is installed at
every corresponding entry, including when every group uses the placeholder. The callable table and
record buffer are shared.

```text
record buffer
├── 16-byte header with instance, hit, miss, and callable section offsets
└── fixed-stride records
    ├── u32 functionIndex
    ├── padding through byte 15
    └── Context.Record bytes
```

After traversal, generated code computes `recordIndex`, reads the record's `functionIndex`, calls
the payload-specific visible-function table, and passes a pointer to the record data. The adapter
uses that pointer to implement `input.record`.

Adding, removing, or repointing records changes only the record buffer. Function tables change only
when the linked schema changes.

## 8. Metal Candidate Dispatch

Metal's native intersection-function selection has no ray-type term, so it cannot directly select
the `_AnyHit_` or `_Intersection_` behavior named by a runtime record. For each payload partition
containing candidate logic, the adopted lowering uses one candidate dispatcher per primitive kind:

```text
IFT index 0 = triangle dispatcher
IFT index 1 = bounding-box dispatcher
IFT index 2 = curve dispatcher, when supported
```

The dispatcher recomputes `recordIndex`, loads `functionIndex`, and switches over the hit groups in
that payload partition. Existing group-specific candidate composition becomes an ordinary switch
arm. A trace of a payload partition without candidate logic passes no IFT, even if another payload
in the schema uses one. Geometry IFT offsets encode only primitive kind; material, ray type, and
group selection stay in the runtime record.

This adds record lookup and dispatch overhead to Metal candidate processing. It preserves the
native D3D/Vulkan/OptiX meaning of a hit record without generating material behavior.

## 9. Safety Boundary

Compile time validates the linked entry set, stage contexts, payload partitions, record types,
generated adapters, and Metal requirements. Runtime construction owns record placement and
acceleration-structure contributions.

Consequently, the host must ensure that every record reached by a trace uses the trace's payload
partition and a compatible primitive. Reflection contains the information needed for a future
validator, but the first version does not claim to prove arbitrary host-created table contents.

## 10. Consequences

The compiled entry set, stage code, payload and record types, function-table signatures, and Metal
tag requirements are fixed by the linked schema. Record counts, positions, selected entries, and
record values remain runtime data.

This split keeps the shader as the source of truth for executable possibilities while matching the
runtime-sized SBT model used by real engines.

## References

- [DirectX Raytracing functional specification](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html)
- [Vulkan ray-tracing shader binding table](https://docs.vulkan.org/spec/latest/chapters/raytracing.html#ray-tracing-shader-binding-table)
- [Metal intersection function tables](https://developer.apple.com/documentation/metal/mtlintersectionfunctiontable)
