# Tutorial: Declaring a Structural Ray-Tracing Program

This tutorial builds a minimal triangle hit/miss program. The same shader schema maps to native
D3D, Vulkan, and OptiX SBTs and to generated Metal function-table dispatch.

## 1. Import the module

```slang
import slang.raytracing;
```

Compile with the experimental-feature flag. The module is loaded only after this explicit import.

## 2. Define traversal and stage contexts

The trace context contains facts shared by the entire schema:

```slang
struct SceneTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}
```

Payload and record types belong to stage contexts, so one schema can serve several payloads and
record shapes:

```slang
struct RadiancePayload
{
    float3 color;
}

struct MaterialRecord
{
    float3 albedo;
}

struct TriangleHitContext : rt::IHitContext
{
    typealias TraceContext = SceneTraceContext;
    typealias Payload = RadiancePayload;
    typealias Record = MaterialRecord;
    typealias Primitive = rt::TrianglePrimitive;
}

struct RadianceMissContext : rt::IPayloadContext
{
    typealias TraceContext = SceneTraceContext;
    typealias Payload = RadiancePayload;
    typealias Record = void;
}
```

`rt::AccelerationStructure` is the portable two-level instanced-scene model. Metal-only programs
can select `rt::MultiLevelAccelerationStructure<N>` when they need direct primitive-AS or
multilevel traversal.

## 3. Write the stage structs

Implement _ClosestHit_ and _Miss_ as interface-conforming structs:

```slang
struct ShadeTriangle : rt::IClosestHitShader
{
    typealias Context = TriangleHitContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        float2 uv = input.triangle.barycentricCoord;
        input.payload.color = input.record.albedo * float3(uv, 1.0);
    }
}

struct ShadeMiss : rt::IMissShader
{
    typealias Context = RadianceMissContext;

    void invoke(rt::MissInput<Context> input)
    {
        input.payload.color = float3(0.0);
    }
}
```

Each input is a compiler-provided, zero-storage property view. A property maps to native stage state
or a structural intrinsic. Reachable optional-property uses contribute additional native entry
parameters or Metal tags; mandatory payload and native hit-attribute parameters remain present.

## 4. Declare the hit group

One hit group associates the stages that a native hit record binds:

```slang
struct TriangleHitGroup : rt::IHitGroup
{
    typealias Context = TriangleHitContext;
    typealias ClosestHit = ShadeTriangle;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}
```

The placeholders say that this group has no source _AnyHit_ or _Intersection_ behavior. There is no
shader-side physical slot.

For a procedural primitive, set `Context.Primitive` to
`rt::BoundingBoxPrimitive<CustomAttributes>` and provide an `IIntersectionShader`. Its
`input.reportHit(distance, attributes)` calls may report zero, one, or several candidates. An
optional _AnyHit_ stage accepts or rejects each reported candidate.

## 5. Assemble the schema

```slang
struct SceneSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = SceneTraceContext;
    typealias HitGroups = rt::HitGroupList<TriangleHitGroup>;
    typealias MissShaders = rt::MissShaderList<ShadeMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}
```

The schema declares executable entries, not SBT records. Slang assigns `TriangleHitGroup` and
`ShadeMiss` dense function indices within the `RadiancePayload` partition and exposes them through
reflection.

A host can reuse `TriangleHitGroup` in any number of records with different `MaterialRecord`
values. A second hit or miss context using `ShadowPayload` can be added to the same lists; Slang
then creates a second payload partition without requiring a second schema.

## 6. Bind the descriptor and trace

```slang
struct FrameParameters
{
    rt::AccelerationStructure scene;
    rt::TraceProgramDescriptor<SceneSchema> program;
}

ParameterBlock<FrameParameters> frame;

[shader("raygeneration")]
void rayGen()
{
    rt::RayTraversalDesc desc = {};
    desc.ray.origin = float3(0.0, 0.0, 0.0);
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 1000.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;
    desc.sbtOffset = 0;
    desc.sbtStride = 1;
    desc.missIndex = 0;

    RadiancePayload payload = {};
    rt::RayTracer<SceneSchema> tracer;
    tracer.trace(desc, frame.scene, frame.program, payload);
}
```

The payload type is inferred from the `inout` argument. Slang verifies that `SceneSchema` serves
that payload. The runtime SBT selectors remain ordinary runtime values.

On D3D, Vulkan, and OptiX, the trace becomes the existing native operation and the descriptor is
erased. On Metal, it becomes intersector traversal followed by generated _ClosestHit_ or _Miss_
visible-function dispatch.

Runtime ray flags remain portable. Metal emits a helper that configures the intersector from the
runtime flag word before traversal.

## 7. Construct runtime records from reflection

After linking the program, the host finds `SceneSchema` and then its `RadiancePayload` partition.
Reflection gives `TriangleHitGroup` a function index, its constituent stage symbols, and its
`MaterialRecord` layout. A portable host uses those stage symbols to create the native hit-group
identifier.

Suppose the host deliberately places that group at physical hit records 1 and 4:

```text
hit record 1 -> TriangleHitGroup + red MaterialRecord
hit record 4 -> TriangleHitGroup + blue MaterialRecord
```

On D3D, Vulkan, or OptiX, both records use the same native group identifier followed by their
different record bytes. On Metal, both records store the reflected function index in the compiler-
owned 16-byte header, followed by the different `MaterialRecord` values. The host chooses
`sbtOffset`, `sbtStride`, geometry indices, and instance contributions that select records 1 and 4.

The important programming rule is: resolve names, indices, record layouts, resource bindings, and
Metal IFT functions from reflection after every link. Function indices are not persistent IDs.

On Metal, schema reflection additionally provides the record strides and descriptor resources. The
final IFT signature is target metadata keyed by the exact schema name and payload index. Bind using
the reflected Metal argument-buffer `[[id]]`, not resource enumeration order. Each geometry's
`intersectionFunctionTableOffset` uses the reflected primitive-kind index, and each populated IFT
entry supplies the exact exported per-kind dispatcher name to install. A payload without candidate
logic reports no IFT entries; when an IFT is required, fixed triangle or bounding-box entries may be
generated reject-all dispatchers for primitive kinds absent from that payload.

## 8. Compile a stage by itself

A stage struct does not require a schema or descriptor for standalone compilation:

```text
slangc shader.slang -experimental-feature \
    -entry ShadeTriangle -stage closesthit -target spirv
```

The source struct name is the entry-point name for this simple case. Slang synthesizes the native
signature from its associated context and reachable input properties. Query reflection for the
exact target name when the type is qualified, specialized, or requires name encoding.

## 9. Extend a schema at link time

A closed list contains exactly its declared entries. To accept hit groups contributed by other
Slang modules, use an open section:

```slang
interface IMaterialHitGroup : rt::IHitGroup {}

struct ExtensibleSceneSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = SceneTraceContext;
    typealias HitGroups = rt::OpenHitGroups<IMaterialHitGroup, TriangleHitGroup>;
    typealias MissShaders = rt::MissShaderList<ShadeMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}
```

Every concrete linked type conforming to `IMaterialHitGroup` joins the finalized hit section.
Linked entries are reflected after the explicitly listed entries. Adding modules may renumber
function indices, so the host still resolves entries by reflected name.

Complete executable cases live under
[`tests/ray-tracing-2/runtime/shaders`](../../../../tests/ray-tracing-2/runtime/shaders), with
focused target, reflection, and diagnostic coverage in the rest of `tests/ray-tracing-2`.
