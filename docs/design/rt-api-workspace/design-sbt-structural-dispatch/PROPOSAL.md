# Structural Ray-Tracing Dispatch

Status: current working proposal.

This proposal adds an explicitly imported, experimental `slang.raytracing` module. It lets shader
authors describe the executable programs that may appear in a ray-tracing shader binding table
(SBT), while leaving the number, order, and contents of the actual SBT records to the host.

Ray-tracing stage logic is written as structs that implement Slang interfaces. A trace program
schema groups those structs into hit, miss, and callable sections. Slang uses that source-level
schema to generate ordinary native ray-tracing entry points on D3D, Vulkan, and OptiX, and to
synthesize the missing dispatch machinery on Metal.

The essential distinction is:

- The **schema** is shader-owned. It describes the finite set of executable entries and the types
  of their payloads, hit attributes, and record data.
- The **SBT instance** is host-owned. It contains any number of runtime records, and each record
  selects one schema entry and carries one value of that entry's record type.

The proposal therefore does not assign physical SBT slots in shader source.

## 1. Motivation

### 1.1 The dispatch-model gap

D3D, Vulkan, and OptiX expose native ray-tracing stages and native SBT dispatch. A trace operation
selects a miss record when traversal misses, or a hit-group record when traversal finds a
candidate or committed hit. The native pipeline then invokes the corresponding _Miss_, _AnyHit_,
_Intersection_, and _ClosestHit_ programs.

Metal exposes traversal and intersection functions, but _Miss_ and _ClosestHit_ logic are ordinary
post-trace control flow. Its intersection-function-table index also does not contain the complete
portable SBT record-index calculation. Slang must therefore generate dispatch code that restores
the source program's portable SBT semantics.

Existing source-level entry points are not sufficient for this synthesis. The relationship between
a trace call, a hit group, and its stage programs normally exists only in host pipeline setup. By
the time Slang emits Metal, that host-owned mapping is unavailable.

The new schema makes the executable relationship visible in shader source without pretending that
the shader owns the runtime table.

### 1.2 Design requirements

The API must:

- describe which stage programs form each hit group;
- allow one physical SBT to contain arbitrary numbers of records for a smaller set of programs;
- allow several payload types in one schema and one physical SBT;
- expose enough reflection for the host to construct every target's native pipeline and records;
- preserve native D3D, Vulkan, and OptiX behavior;
- synthesize only the dispatch and ABI adaptation Metal lacks;
- infer Metal intersection tags from the schema and reachable stage operations;
- allow independently compiled Slang modules to contribute programs before final linking; and
- diagnose invalid structural-stage use and structural/legacy pipeline-API mixing early.

## 2. Proposed Design

### 2.1 Source model

The source model is organized around four ideas, in order of importance:

1. `ITraceProgramSchema` declares the programs that may be placed in an SBT. The schema does not
   declare record positions.
2. Hit, miss, and callable logic is written as structs implementing stage interfaces rather than
   only as free-standing shader entry points. `IHitGroup` associates the three hit-stage structs
   that form one native hit group.
3. Context types describe the payload, primitive, record data, acceleration-structure topology,
   and motion contract available to those stages. Stage inputs expose built-in state through
   zero-storage properties.
4. Reflection maps schema entries to target symbols, function indices, ABI sizes, and Metal
   resources so the host can instantiate the runtime SBT.

`RayTracer<Schema>` uses the schema to synthesize trace dispatch, and
`TraceProgramDescriptor<Schema>` represents the target resources used by that dispatch.

### 2.2 Structural dispatch model

#### 2.2.1 Describing the executable SBT schema

The interface hierarchy is:

```text
ITraceProgramSchema
├── TraceContext : ITraceContext
│   ├── AccelerationStructure
│   └── Motion
├── HitGroups : IHitGroupList
│   └── IHitGroup
│       ├── Context : IHitContext
│       │   ├── TraceContext
│       │   ├── Payload
│       │   ├── Record
│       │   └── Primitive
│       ├── ClosestHit : IClosestHitShader
│       ├── AnyHit : IAnyHitShader
│       └── Intersection : IIntersectionStage
├── MissShaders : IMissShaderList
│   └── IMissShader
│       └── Context : IPayloadContext
└── CallableShaders : ICallableShaderList
    └── ICallableShader
        └── Context : ICallableContext
```

The simplified contract is:

```slang
public interface ITraceContext
{
    associatedtype AccelerationStructure;
    __constraint AccelerationStructure : IAccelerationStructure;
    associatedtype Motion;
    __constraint Motion : IRayMotion;
}

public interface IStageContext
{
    associatedtype TraceContext;
    __constraint TraceContext : ITraceContext;
    associatedtype Record;
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

[require(structural_raytracing_closesthit)]
public interface IClosestHitShader
{
    associatedtype Context;
    __constraint Context : IHitContext;
    void invoke(ClosestHitInput<Context> input);
}

[require(structural_raytracing_anyhit)]
public interface IAnyHitShader
{
    associatedtype Context;
    __constraint Context : IHitContext;
    void invoke(AnyHitInput<Context> input);
}

[sealed]
public interface IIntersectionStage
{
    associatedtype Context;
    __constraint Context : IHitContext;
}

[require(structural_raytracing_intersection)]
public interface IIntersectionShader : IIntersectionStage
{
    __constraint Context.Primitive : ICustomIntersectionPrimitive;
    void invoke(IntersectionInput<Context> input);
}

[require(structural_raytracing_miss)]
public interface IMissShader
{
    associatedtype Context;
    __constraint Context : IPayloadContext;
    void invoke(MissInput<Context> input);
}

[require(structural_raytracing_callable)]
public interface ICallableShader
{
    associatedtype Context;
    __constraint Context : ICallableContext;
    void invoke(CallableInput<Context> input);
}

public interface IHitGroup
{
    associatedtype Context;
    __constraint Context : IHitContext;
    associatedtype ClosestHit;
    __constraint ClosestHit : IClosestHitShader;
    associatedtype AnyHit;
    __constraint AnyHit : IAnyHitShader;
    associatedtype Intersection;
    __constraint Intersection : IIntersectionStage;
    __constraint ClosestHit.Context == Context;
    __constraint AnyHit.Context == Context;
    __constraint Intersection.Context == Context;
}

[sealed]
public interface IHitGroupList { ... }

[sealed]
public interface IMissShaderList { ... }

[sealed]
public interface ICallableShaderList { ... }

public struct HitGroupList<each Group> : IHitGroupList
    where expand each Group : IHitGroup
{ ... }

public struct MissShaderList<each Shader> : IMissShaderList
    where expand each Shader : IMissShader
{ ... }

public struct CallableShaderList<each Shader> : ICallableShaderList
    where expand each Shader : ICallableShader
{ ... }

public interface ITraceProgramSchema
{
    associatedtype TraceContext;
    __constraint TraceContext : ITraceContext;
    associatedtype HitGroups;
    __constraint HitGroups : IHitGroupList;
    associatedtype MissShaders;
    __constraint MissShaders : IMissShaderList;
    associatedtype CallableShaders;
    __constraint CallableShaders : ICallableShaderList;
}
```

Only a hit record binds several programs, so only hit entries need a group wrapper. _Miss_ and
_Callable_ sections list their stage structs directly.

`NoClosestHit<Context>`, `NoAnyHit<Context>`, and `NoIntersection<Context>` represent absent stages.
`NoIntersection` is valid only for built-in-intersection primitives. These placeholders require no
user stage implementation. `NoAnyHit` and `NoIntersection` emit no native stage;
`NoClosestHit` may use a shared no-op function where Metal requires a populated visible-function-
table entry.

Each primitive names the attributes seen by its hit stages:

```slang
[sealed]
public interface IIntersectionPrimitive
{
    associatedtype Attributes;
}

public struct TrianglePrimitive : IBuiltinIntersectionPrimitive
{
    typealias Attributes = TriangleData;
}

public struct CurvePrimitive : IBuiltinIntersectionPrimitive
{
    typealias Attributes = CurveData;
}

public struct BoundingBoxPrimitive<CustomAttributes> : ICustomIntersectionPrimitive
{
    typealias Attributes = CustomAttributes;
}
```

`TriangleData` and `CurveData` are built-in property views. A procedural _Intersection_ shader
reports the application-defined `CustomAttributes`; its matching _AnyHit_ and _ClosestHit_ stages
read that same type through their inputs.

Each section can be closed, empty, or open:

```slang
HitGroupList<GroupA, GroupB>
NoHitGroups
OpenHitGroups<IPluginHitGroup, BuiltInGroup>

MissShaderList<SkyMiss, ShadowMiss>
NoMissShaders
OpenMissShaders<IPluginMiss, SkyMiss>

CallableShaderList<ShadeCallable>
NoCallableShaders
OpenCallableShaders<IPluginCallable, ShadeCallable>
```

An open section includes its explicitly listed entries plus every concrete linked type conforming
to its tag interface. Listed entries retain declaration order. Linked entries follow in stable,
qualified-type-name order. Repeating an entry within an explicit list is an error. Deduplication
applies only when open-section discovery finds the same canonical entry through more than one
listed or linked path.

The compiler assigns dense **function indices** to the finalized entries:

- hit-group and _Miss_ indices start at zero within each payload partition;
- _Callable_ indices are schema-wide; and
- indices identify one linked program and are not persistent application IDs.

A function index identifies executable code, not a physical SBT record. The host may place the same
entry in records 1, 4, and 10,000, with different record data in each one.

Payload types are derived from the finalized hit and miss entries. One schema can therefore serve,
for example, both `RadiancePayload` and `ShadowPayload` without duplicating the schema or descriptor.

#### 2.2.2 `TraceProgramDescriptor` and native layout

`TraceProgramDescriptor<Schema>` is ParameterBlock-like: shader source has one typed descriptor,
while specialization chooses its physical target representation.

On D3D, Vulkan, and OptiX, the descriptor has no shader-visible resources. The host constructs the
native pipeline and SBT using reflected entry symbols and native shader identifiers.

On Metal, a schema with payload partitions `P0 ... Pn-1` lowers to:

```text
TraceProgramDescriptor<Schema>
├── for each payload Pi
│   ├── intersection_function_table<payload-tags-i>
│   ├── visible_function_table<MissSignature-i>
│   └── visible_function_table<ClosestHitSignature-i>
├── visible_function_table<CallableSignature>
└── device record buffer
```

There are `3 * payloadCount + 2` resource fields. Reflection reports each resource's kind, payload
partition, exact field name, and Metal argument-buffer `[[id]]`; enumeration order is not a host
binding contract.

A payload partition without candidate logic still has an intersection-function-table resource in
the descriptor shape, but its trace does not consume that table and reflection reports no generated
intersection functions. A host may bind an unpopulated minimum-capacity table where the Metal API
requires a resource object. The same rule applies to a physically present miss, _ClosestHit_, or
_Callable_ table whose reflected entry count is zero.

The Metal record buffer contains one header, optional instance-path lookup data, and three dynamic
record sections:

```text
byte 0   u32 instanceTrieOffset
byte 4   u32 hitSectionOffset
byte 8   u32 missSectionOffset
byte 12  u32 callableSectionOffset

instance-path lookup data

hit records      at hitSectionOffset      + physicalHitIndex * hitStride
miss records     at missSectionOffset     + missIndex        * missStride
callable records at callableSectionOffset + callableIndex    * callableStride
```

Every record has the same header shape within its section:

```text
+0   u32 functionIndex
+4   12 bytes reserved/padding
+16  Context.Record application data
```

`0xffffffff` represents an empty record. Section strides are the largest record in that section,
including the 16-byte header, rounded up to 16 bytes. Metal reflection exposes these strides and
the record header size. Application `Record` bytes use the reflected `DefaultStructuredBuffer`
layout on Metal. The stride queries return zero on targets that do not use this compiler-owned
record buffer; portable hosts instead use each entry's ordinary target record layout together with
the native API's SBT alignment and shader-identifier rules.

An empty record performs no _Miss_, _ClosestHit_, or _Callable_ dispatch. Within a triangle or curve
dispatcher that has real group arms, an empty record accepts the hardware candidate without source
_AnyHit_ behavior. A fixed reject-all dispatcher for a primitive kind absent from the payload
rejects every candidate, including an empty record. An empty procedural-bounding-box record also
rejects the box because no _Intersection_ stage reports a geometry hit.

For a committed hit, the physical record index is:

```text
instanceContribution + geometryIndex * desc.sbtStride + desc.sbtOffset
```

For a miss it is `desc.missIndex`. The argument to `callShader` is the physical callable record
index. The selected record's `functionIndex` then chooses the appropriate function-table entry or
generated dispatch arm.

The descriptor currently uses an intersection function table, visible function tables, and a
buffer resource. Intersection-function-buffer arguments and `[[user_data]]` are outside the first
version.

#### 2.2.3 Contexts, payloads, and stage inputs

`ITraceContext` contains only traversal-wide facts: acceleration-structure topology and motion.
Payload and record types belong to stage contexts because different groups in one schema may use
different types.

For example:

```slang
struct SceneTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct RadianceHitContext : rt::IHitContext
{
    typealias TraceContext = SceneTraceContext;
    typealias Payload = RadiancePayload;
    typealias Record = MaterialRecord;
    typealias Primitive = rt::TrianglePrimitive;
}

struct RadianceClosestHit : rt::IClosestHitShader
{
    typealias Context = RadianceHitContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.radiance = shade(input.record, input.triangle.barycentricCoord);
    }
}
```

`ClosestHitInput`, `AnyHitInput`, `IntersectionInput`, `MissInput`, `CallableInput`, `TriangleData`,
and `CurveData` are zero-storage views. Their built-in variables are properties rather than stored
fields. A property use maps to an existing native intrinsic or to a compiler-owned structural IR
operation. Payload, native hit attributes, and callable data remain mandatory parts of their native
stage ABIs even when `invoke` does not read them. Slang collects reachable uses to synthesize only
the optional built-in parameters and record plumbing that those uses require.

The trace operation is:

```slang
rt::RayTracer<SceneSchema> tracer;
tracer.trace(desc, accelerationStructure, descriptor, payload);
```

`trace<Payload>` infers `Payload` from the `inout` argument and checks that the completed schema has
at least one hit group or _Miss_ shader using that payload type. The concrete stage contexts keep
`input.payload` strongly typed.

A completed schema may contain at most one semantically empty payload type. The payload-free
overload is available when that one type exists. It still identifies a partition, but shader code
may not explicitly pass or access its value.

Runtime selectors remain runtime data. The compiler cannot prove that `sbtOffset`, `sbtStride`, and
`missIndex` select records from the same payload partition as the trace argument. As with native
D3D, Vulkan, and OptiX SBTs, constructing that mapping correctly is a host responsibility.

_Callable_ dispatch is similarly record-based:

```slang
tracer.callShader<MyCallableContext>(callableRecordIndex, descriptor, callableData);
```

All _Callable_ entries in one completed schema must use exactly the same `CallableData` type because
the schema has one callable table and one native callable-data ABI. Record types may differ by
entry. Slang checks both the completed schema and each `callShader` operation.

### 2.3 Stage lowering

Stage `invoke` methods represent entry points; they are not ordinary calls and must not depend on a
source call-graph edge for retention. Structural discovery retains the selected stage methods and
synthesizes target entry points before dead-code elimination can remove them.

A stage can also be compiled without a schema. For example,
`-entry RadianceClosestHit -stage closesthit` selects the struct by source type name and synthesizes
that stage alone. A simple top-level struct keeps its struct name as the native entry-point name.
Reflection is authoritative for qualified, specialized, reserved, or otherwise encoded names.
If one struct implements more than one executable stage interface, `-stage` is mandatory; selecting
only its entry name is ambiguous and is diagnosed.

#### 2.3.1 Hit-group combinations and Metal candidate dispatch

The primitive fixes which source stages are legal:

| Primitive | _Intersection_ | _AnyHit_ | _ClosestHit_ |
| --- | --- | --- | --- |
| Triangle | Not allowed; hardware tests the triangle | Optional | Optional |
| Curve | Not allowed; Metal tests the built-in curve | Optional | Optional |
| Procedural bounding box | Required | Optional | Optional |

Curve groups are Metal-only. Empty optional stages use the canonical placeholders.

The lowering is:

| Source hit group | D3D / Vulkan / OptiX | Metal |
| --- | --- | --- |
| Triangle, no _AnyHit_ | Native triangle intersection | No source candidate logic; if the payload uses a shared dispatcher, this group's arm accepts the candidate |
| Triangle + _AnyHit_ | Native _AnyHit_ stage | The triangle candidate-dispatch arm runs the source _AnyHit_ logic |
| Curve, no _AnyHit_ | Rejected by capability | Built-in curve intersection; a shared dispatcher arm accepts when another group makes the payload use an IFT |
| Curve + _AnyHit_ | Rejected by capability | The curve candidate-dispatch arm runs the source _AnyHit_ logic |
| Bounding box + _Intersection_ | Native _Intersection_; `reportHit` uses the target operation | The bounding-box arm runs the source _Intersection_ logic |
| Bounding box + _Intersection_ + _AnyHit_ | Each native `reportHit` may invoke native _AnyHit_ | The bounding-box arm composes both stages at each `reportHit` |

_ClosestHit_ always runs after traversal commits the final hit. On Metal it is dispatched through
the payload partition's visible function table using the selected record's function index.

Metal candidate logic is not installed as one intersection-function-table entry per hit group or
per SBT record. For each payload partition that needs candidate logic, Slang generates one
dispatcher per primitive kind at fixed table indices:

| IFT index | Primitive dispatcher |
| --- | --- |
| 0 | Triangle |
| 1 | Bounding box |
| 2 | Curve, when this payload partition contains a curve group |

Triangle and bounding-box entries exist whenever the shared IFT is used; an unavailable kind uses
a reject-all implementation. The acceleration structure selects the primitive-kind dispatcher.
That dispatcher computes the portable physical hit-record index, reads its function index, and
selects the hit group's candidate arm. Thus _AnyHit_ selection still follows the runtime SBT record,
including its ray-type and instance contributions.

For a procedural primitive, `IntersectionInput.reportHit(distance, attributes)` and
`IntersectionInput.reportHit(distance, hitKind, attributes)` have the native multi-candidate
meaning. An _Intersection_ shader may call either overload zero, one, or several times. On D3D,
Vulkan, and OptiX, it lowers to the target report-intersection operation, which applies the ray
interval and invokes native _AnyHit_ behavior when required.

Metal cannot transfer control from a report-intersection intrinsic to a separate _AnyHit_ stage, so
Slang implements the same contract inside the generated candidate arm:

1. Test each reported distance against the current ray interval.
2. Run the group's source _AnyHit_ logic when present.
3. Return `false` from `reportHit` when that candidate is rejected.
4. When accepted, preserve its distance, attributes, and hit kind and shorten the current maximum.
5. Return the closest accepted candidate from that native intersection-function invocation.

`AnyHitInput.ignoreHit()` rejects the current candidate.
`AnyHitInput.acceptHitAndEndSearch()` accepts it and requests traversal termination; Metal's
generated control path preserves that short-circuit behavior.

This is ABI adaptation required to preserve native `reportHit` semantics. Slang does not invent an
additional material or candidate-selection policy.

### 2.4 Acceleration structures, motion, and ray flags

`ITraceContext.AccelerationStructure` names the topology explicitly:

| Type | Meaning | Availability |
| --- | --- | --- |
| `AccelerationStructure` | Portable two-level TLAS-to-BLAS, or Metal IAS-to-primitive-AS | D3D, Vulkan, OptiX, Metal |
| `MultiLevelAccelerationStructure<1>` | Direct Metal primitive-AS traversal, with no instance level | Metal |
| `MultiLevelAccelerationStructure<N>` for `N >= 2` | Metal multilevel traversal with at most `N` acceleration-structure levels | Metal 3.1 capability |

`N` counts all acceleration-structure levels on the path, including the primitive-AS leaf, and must
be in `1..32`. `N == 1` therefore has no instance level and omits the `instancing` and `max_levels`
tags. A normal two-level
`AccelerationStructure` uses `instancing` without `max_levels`.

For the portable `AccelerationStructure`, Metal obtains the record contribution from a flat table
indexed by scalar `instance_id`. Every `MultiLevelAccelerationStructure<N>` with `N >= 2` uses the
native outer-to-inner `instance_id` array and walks the record-buffer trie, including `N == 2`.
Candidate and committed-hit dispatch use the same lookup. Intermediate values are word offsets
relative to the trie root; the leaf is the instance contribution later added to
`geometryIndex * sbtStride + sbtOffset`.

`ITraceContext.Motion` selects one sealed motion contract:

| Motion type | Meaning | Availability |
| --- | --- | --- |
| `NoMotion` | No ray-time ABI | All targets |
| `InstanceMotion` | Moving acceleration-structure instances | Vulkan motion extension, OptiX, and Metal |
| `PrimitiveMotion` | Metal primitive motion | Metal |
| `PrimitiveAndInstanceMotion` | Both Metal motion modes | Metal |

D3D motion blur is not part of this design. A motion-enabled context exposes `input.time` to
_ClosestHit_, _AnyHit_, _Intersection_, and _Miss_ stages; `CallableInput` has no time property. A
no-motion context exposes no stage time.

`RayTraversalDesc.rayFlags` remains a runtime cross-target flag word. D3D and Vulkan lower it to
their native trace operation. OptiX supports its representable subset; version one cannot represent
`RAY_FLAG_SKIP_TRIANGLES`. Metal uses a generated helper that conditionally calls the
intersector setters for opacity, first-hit acceptance, face culling, geometry culling, and related
options. Constant flags fold normally. Skip-_ClosestHit_ is honored when Slang performs Metal's
post-trace dispatch.

### 2.5 Metal tag-list inference

Metal requires the intersector and its reachable intersection functions to agree on a tag list.
Shader authors do not write this list directly. Slang first derives one shared requirement set for
each `(schema, payload partition)` from the topology and motion contract, reachable stage-property
uses, and selected target capabilities. Each generated primitive dispatcher then adds exactly one
primitive selector to that shared set.

#### 2.5.1 Combination and validation rule

The primitive selector is chosen separately for each generated dispatcher, never unioned across
triangle, bounding-box, and curve groups. Topology and motion each come from one sealed associated
type. All remaining inferred tags are compatible optional requirements.

Slang normalizes the inferred set and diagnoses an invalid combination or a missing target
capability during compilation. Consequently the source API cannot form a function tag list with
conflicting primitive selectors or motion/topology modes.

#### 2.5.2 Inference sources

| Metal tag | Axis | Inference source |
| --- | --- | --- |
| `triangle`, `bounding_box`, or `curve` | Per-function primitive selector | Primitive kind of the generated dispatcher |
| `instancing` | Acceleration-structure topology | `AccelerationStructure`, or `MultiLevelAccelerationStructure<N>` with `N >= 2` |
| `max_levels<N>` | Acceleration-structure topology | `MultiLevelAccelerationStructure<N>` with `N >= 2` |
| `primitive_motion` | Motion | `PrimitiveMotion` or `PrimitiveAndInstanceMotion` |
| `instance_motion` | Motion | `InstanceMotion` or `PrimitiveAndInstanceMotion` |
| `triangle_data` | Optional triangle data | Reachable use of `input.triangle.barycentricCoord`, `input.triangle.frontFacing`, or triangle `input.hitKind` |
| `curve_data` | Optional curve data | Reachable use of `input.curve.parameter` |
| `world_space_data` | Optional transformed data | Candidate-stage use of `worldSpaceOrigin` or `worldSpaceDirection`; _ClosestHit_ use of `objectSpaceRay`; or hit-stage use of `objectToWorld` or `worldToObject` |
| `extended_limits` | Target traversal requirement | Selected `metal_raytracing_extended_limits` capability |
| `intersection_function_buffer` | Future lowering | Not inferred in the first version |
| `user_data` | Future function-buffer record data | Not inferred in the first version |

_ClosestHit_ and _Miss_ world-space origin and direction can be reconstructed from the original ray,
so those uses alone do not require `world_space_data`. Candidate stages need Metal-provided
world-space data. Transform and object-space reconstruction also require an instance context;
Slang diagnoses those uses with direct primitive-AS traversal.

## 3. Reflection and Host Construction

### 3.1 Reflection contract

Schema reflection exposes:

- the schema's exact name, type, trace context, and whether each section is open;
- payload partitions, their ordinary type layout, and their target-native payload ABI size;
- hit groups and _Miss_ shaders in per-payload function-index order;
- _Callable_ shaders in schema-wide function-index order;
- each entry's context, record type and layout, and linked/listed origin;
- each hit group's primitive, attributes, and stage composition;
- the maximum target-native hit-attribute size;
- on Metal, record header and section strides, descriptor-resource bindings, generated IFT
  functions, geometry kinds, fixed IFT indices, and exact exported names; and
- a finalized Metal IFT signature in target metadata, keyed by exact schema name and payload index.

Target symbols follow the target's actual binding unit. D3D, Vulkan, and OptiX expose native stage
symbols. On Metal, source _AnyHit_ and _Intersection_ stages have no separately bindable symbol
because their logic is folded into a candidate dispatcher. A `NoClosestHit` group instead exposes
the shared synthesized no-op visible-function symbol through the group-level _ClosestHit_ query.
Every populated IFT entry in this lowering is an exported generated dispatcher and exposes its exact
entry-point name.

The target metadata is produced after target lowering because capability and reachable-operation
analysis determine the final Metal tag signature. A Metal host must use that metadata rather than
trying to reproduce the tag list from ordinary source reflection.

Schema-free reflection can enumerate visible concrete hit-group, _Miss_-shader, and
_Callable_-shader declarations. Those catalog entries have no schema-assigned function index, do
not retain otherwise-unused code, and do not have schema-specific Metal symbols. It does not form a
separate catalog of standalone _ClosestHit_, _AnyHit_, or _Intersection_ structs. Hosts should query
a finalized schema when building a pipeline.

`getNativePayloadSize()` is a target ray-transport ABI requirement and is distinct from the
ordinary payload `TypeLayoutReflection`. It returns zero on Metal because Metal has no corresponding
host pipeline payload-size setting. Metal target metadata reports the finalized intersection-
function signature tag mask, not a payload byte size. Likewise, schema record-stride queries
describe only the compiler-owned Metal record buffer and return zero on other targets.

### 3.2 Host workflow

The host:

1. Finds the finalized schema in reflection.
2. Creates the target pipeline entries and function tables from reflected stage symbols and
   function indices.
3. On Metal, sets each geometry descriptor's `intersectionFunctionTableOffset` to the reflected
   fixed primitive-kind index, keeps instance IFT offsets at zero, and installs each reflected
   exported dispatcher at its reported index.
4. Chooses the physical record counts and record ordering required by the scene.
5. Writes each record's target shader identifier or reflected Metal function index, followed by
   application record data in the reflected layout.
6. Builds the instance-contribution mapping and uses the same ray-type convention for
   `sbtOffset`, `sbtStride`, and `missIndex`.
7. Binds the reflected descriptor resources on Metal; on other targets it binds the native SBT.

Consider 10,000 materials that all use `OpaqueHitGroup`:

```text
schema entry
    OpaqueHitGroup -> function index 3

runtime records
    record 0    -> function 3 + MaterialRecord for material 0
    record 1    -> function 3 + MaterialRecord for material 1
    ...
    record 9999 -> function 3 + MaterialRecord for material 9999
```

The shader declares `OpaqueHitGroup` once. The host creates 10,000 records because record count and
data are scene properties. On D3D, Vulkan, and OptiX, the records repeat the same native shader
identifier. On Metal, they repeat function index 3. `input.record` observes the data from the
particular record that selected the stage.

Physical layouts can be irregular. If an application places one entry at records 1 and 4, it writes
that entry's identifier into those two records and arranges its runtime selectors accordingly. No
shader declaration needs to enumerate the unused positions.

### 3.3 Responsibility boundary

The compiler verifies the shader-owned contract:

- every listed or linked entry satisfies its structural interface;
- all stages in a hit group use the same hit context;
- every entry's trace context matches the schema;
- a trace payload is served by the schema;
- topology, motion, primitive, property, and target capabilities are compatible; and
- payload, non-void record, callable-data, and custom-attribute types satisfy their native plain-
  data requirements. Callable data and custom attributes may not be `void`; `Record = void` is
  valid.

The host owns facts that exist only at runtime:

- record count and order;
- the entry identifier and application data written to each record;
- acceleration-structure instance contributions; and
- whether runtime selectors for a trace choose records from its payload partition.

Reflection supplies the information needed to enforce those host invariants without duplicating
the shader schema.

## 4. Compilation and Diagnostics

The module is activated only by an explicit `import slang.raytracing` and an experimental-feature
compiler flag. Core does not source-depend on the ray-tracing module. Loading the module registers
its compiler-known declarations after core is available.

Stage interfaces and related structural contracts map directly to compiler-recognized AST/IR
forms. Existing native ray-tracing IR operations are reused where they already express the
semantics. Structural-only IR records the schema, property uses, and Metal dispatch information that
ordinary entry-point IR cannot represent.

The compiler discovers and completes selected schemas after linking, retains their stage methods,
collects reachable property and capability requirements, and then generates target adapters before
ordinary dead-code elimination, legalization, and emission. These steps form one structural
ray-tracing subsystem; they do not require a separate compiler pass for every bullet.

Executable structural stage structs must be stateless and compiler-created, and their `invoke`
methods cannot be called directly. Schema, group, list, and related structural metadata types do not
become runtime values. A stage-input view may flow only as a direct, read-only by-value parameter
within its matching stage; it cannot be stored, returned, passed as `out`/`inout`/`ref`, or escape
through a generic container. Descriptors and tracers remain ordinary typed API values subject to
their operation-specific contracts.

The stage-interface capability requirements also check the body of each `invoke` method against its
native stage. For example, implementing `IClosestHitShader` does not permit intrinsics that are
unavailable in a _ClosestHit_ stage.

Mixing legacy and structural pipeline ray tracing in the same module is diagnosed in the front
end. After linking, the compiler also diagnoses selected reachable programs that mix the two
models across modules. Merely importing the module, or using ordinary ray-query or hit-object APIs,
does not count as structural pipeline use.

Other required diagnostics include invalid schema entries, trace-context disagreement, duplicate
entries, invalid open-section tags, unsupported primitive/stage combinations, unsupported target
capabilities, invalid data types, unavailable stage properties, invalid empty-payload use, and a
runtime type escaping its structural context.

## 5. Target Coverage and Initial Scope

| Capability | D3D | Vulkan | OptiX | Metal |
| --- | --- | --- | --- | --- |
| Native trace, payload, SBT record, and callable lowering | Yes | Yes | Yes | Synthesized structural dispatch |
| Triangle and procedural AABB groups | Yes | Yes | Yes | Yes |
| Built-in curve groups | No | No | No | Capability-gated |
| Instance motion | No in this design | Capability-gated | Yes | Capability-gated |
| Primitive motion | No | No | No | Capability-gated |
| Multilevel acceleration structures | No | No | No | Capability-gated |

The first version covers pipeline ray tracing. Existing ray-query and hit-object APIs continue to
coexist and are not replaced. Shader Execution Reordering is excluded.

Metal intersection-function-buffer arguments, `[[user_data]]`, ordinary global shader parameters
inside generated candidate functions, and `callShader` reachable from structural _AnyHit_ or
_Intersection_ logic are excluded. Candidate data should be carried in the reflected per-record
type.

Open sections are completed by Slang linking before target generation. Linking independently
compiled Metal binaries into an already generated structural dispatch kernel is not part of the
contract.

The compiler cannot validate host-written record contents or runtime selectors. This limitation is
the same fundamental boundary present in native SBT APIs and is made explicit through reflection
rather than hidden behind a false source-level slot model.
