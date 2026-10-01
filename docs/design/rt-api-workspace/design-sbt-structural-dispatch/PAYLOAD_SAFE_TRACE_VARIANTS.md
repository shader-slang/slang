# Payload Model For Trace Program Schemas

Status: supporting analysis for [PROPOSAL.md](PROPOSAL.md). The proposal is normative.

This note defines how one trace program schema serves several payload types, what Slang checks,
and what remains the host's responsibility.

## 1. Decision

Payload is a stage ABI, not an acceleration-structure property or a table-layout property.
Therefore:

- `ITraceContext` describes traversal topology and motion.
- Each hit or miss context declares the payload used by its stages.
- `ITraceProgramSchema` contains all hit, miss, and callable entries for one trace context.
- A trace call infers its payload type from its payload argument.
- The host decides which runtime record each ray selects.

One schema and one `TraceProgramDescriptor<Schema>` can consequently serve radiance, shadow, and
other payloads.

## 2. Context Hierarchy

```slang
public interface ITraceContext
{
    associatedtype AccelerationStructure;
    associatedtype Motion;
    __constraint AccelerationStructure : IAccelerationStructure;
    __constraint Motion : IRayMotion;
}

public interface IStageContext
{
    associatedtype TraceContext;
    associatedtype Record;
    __constraint TraceContext : ITraceContext;
}

public interface IPayloadContext : IStageContext
{
    associatedtype Payload;
}

public interface IHitContext : IPayloadContext
{
    associatedtype Primitive;
    __constraint Primitive : IIntersectionPrimitive;
}

public interface ICallableContext : IStageContext
{
    associatedtype CallableData;
}
```

`IPayloadContext` is shared by hit and miss stages. A miss shader may reuse an `IHitContext` when
it uses the same trace context, payload, and record type. Callables use `CallableData`, not a ray
payload.

## 3. Stage And Schema Relationships

Every stage interface exposes an associated `Context`. A hit group uses separate constraints to
require all three hit stages to share its context:

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

Miss and callable records each bind one shader, so their schema sections list shader types
directly:

```slang
public interface ITraceProgramSchema
{
    associatedtype TraceContext;
    associatedtype HitGroups;
    associatedtype MissShaders;
    associatedtype CallableShaders;
    __constraint TraceContext : ITraceContext;
    __constraint HitGroups : IHitGroupList;
    __constraint MissShaders : IMissShaderList;
    __constraint CallableShaders : ICallableShaderList;
}
```

The schema's payload set is derived after linking:

```text
payloads(Schema) =
    { G.Context.Payload | G in Schema.HitGroups }
    union
    { M.Context.Payload | M in Schema.MissShaders }
```

Callables do not contribute a payload.

## 4. Trace Operation

```slang
public void trace<Payload>(
    RayTraversalDesc desc,
    Schema.TraceContext.AccelerationStructure accelerationStructure,
    TraceProgramDescriptor<Schema> descriptor,
    inout Payload payload);
```

`Payload` is inferred from the argument. Both calls below use one schema and one descriptor:

```slang
RadiancePayload radiance = {};
tracer.trace(desc, scene, descriptor, radiance);

OcclusionPayload occlusion = {};
tracer.trace(desc, scene, descriptor, occlusion);
```

The runtime `sbtOffset`, `sbtStride`, and `missIndex` fields in `RayTraversalDesc` retain their
native meanings.

## 5. Compile-Time Guarantees

For a trace with payload type `P`, Slang verifies:

1. `P` belongs to `payloads(Schema)`.
2. Every listed or linked entry uses `Schema.TraceContext`.
3. Every hit group's `_ClosestHit_`, `_AnyHit_`, and `_Intersection_` stages use the group's
   context and therefore one payload type.
4. Every stage input exposes `input.payload` as its context's payload type.
5. Payload and record types satisfy the portable data restrictions.

These checks prevent a stage from being compiled with an internally inconsistent payload ABI.
They do not prove which host-written record a runtime index will select.

## 6. Host Safety Rule

The selected hit record is determined at runtime:

```text
recordIndex =
    instanceContribution
    + geometryIndex * desc.sbtStride
    + desc.sbtOffset
```

The miss record is `desc.missIndex`. The host writes those records and the acceleration-structure
contributions, so it must maintain this invariant:

> Every record reachable by a trace with payload `P` is empty or names an entry whose context
> payload is `P`.

No shader type can prove this for an arbitrary native SBT. Reflection gives the host the payload
partition, entry identity, function index, and record layout needed to construct it correctly.

## 7. Function Indices

The compiler assigns dense function indices after linking:

- Hit-group and miss-shader indices restart at zero for each payload.
- Callable-shader indices are program-wide.
- Listed entries precede entries discovered through an open section.

On Metal, a runtime record stores the function index and the trace's payload type selects the table
that interprets it. D3D, Vulkan, and OptiX records use native shader identifiers; the reflected
function index is only an enumeration order on those targets. Hosts use qualified entry names as
stable keys and resolve numeric indices after every link.

## 8. Empty Payload

An empty payload is a user-defined struct with no data members. The standard module does not define
a distinguished empty-payload type.

When a schema contains exactly one empty payload type, this overload is available:

```slang
tracer.trace(desc, accelerationStructure, descriptor);
```

The compiler supplies the target's required native payload operand. User code cannot pass a value
of the empty payload type to `trace`, explicitly specialize `trace` with it, or read
`input.payload` from a stage using it. An ordinary value of the same empty struct outside these
structural API boundaries is harmless and remains ordinary Slang code. Two distinct empty payload
types in one schema are ambiguous and produce a diagnostic.

## 9. Open Sections

An open section adds linked entries that conform to its tag interface. Payload derivation,
context validation, function-index assignment, tag inference, and reflection run after that linked
set is complete.

A linked entry may introduce a new payload type. Code can trace that payload only when the payload
type is visible at the call site. Adding or removing linked entries produces a new compiled program
and may renumber function indices.

## 10. Stage Reuse And Native ABI

A concrete stage's context fixes its payload ABI. Reusing one source body for several payloads uses
ordinary generic specialization; each concrete context still has one payload type.

D3D, Vulkan, and OptiX lower those concrete stages through their existing native payload paths.
Metal chooses a payload-specific `ray_data` type and table set at each trace site. Candidate logic
is also decided per payload partition: a trace passes an IFT only when its own partition needs one,
independently of other payloads in the schema. Schema type, function index, and runtime record index
do not specialize the source stage body; ABI-distinct target adapters may still differ.

## 11. Future Traversal-Axis Split

The adopted model keeps acceleration-structure and motion types in `ITraceContext`. A possible
future design could infer the acceleration-structure type from the trace argument and motion from
the traversal descriptor, removing `ITraceContext`. That is not part of this revision.

## 12. Portable Data Restrictions

Payload and record types use the portable plain-data subset. Opaque resources, pointers, atomics,
runtime-sized arrays, and non-copyable types are rejected. `void` may represent an absent record,
but it is not a payload type.

A payload served only by hit groups or only by miss shaders is legal. The host writes empty records
where the other section must not invoke a stage.

## 13. Future Runtime Validation

An optional host validator may use reflection to check record bounds, payload partitions, function
indices, record bytes, primitive kinds, and acceleration-structure contributions. Such validation
would improve diagnostics but would not change the source type guarantees above.
