# Tutorial: Building a Ray Tracer with the SBT Structural Dispatch API

This tutorial explains how an application would use the API sketched in this
directory to build a pipeline ray tracer.

The central idea is:

```text
Declare the hit/miss groups in shader source.
Use RayTraversalDesc to select SBT-style slots at trace time.

D3D/Vulkan:
    native SBT dispatch uses those slots.

Metal:
    Slang synthesizes equivalent post-intersect visible-function dispatch.
```

The complete example is in
[`rt-structural-dispatch-table-example.slang`](rt-structural-dispatch-table-example.slang).

## Step 1: Import the Module

Use the current prototype module:

```slang
import rt_pipeline;
```

The module is split into basic ray-tracing types in
[`rt_basic_types.slang`](rt_basic_types.slang), stage contracts in
[`rt_stage_contracts.slang`](rt_stage_contracts.slang), and structural
dispatch-table/descriptor types in
[`rt_structural_dispatch_table.slang`](rt_structural_dispatch_table.slang).

### Difference from the Old Model

Shader code:
Old Slang ray tracing code typically uses target-shaped stage entry points such
as `[shader("closesthit")]`, `[shader("miss")]`, and `[shader("intersection")]`.
The new API imports a Slang-level library and writes stage logic as typed
stage structs.

Host code:
The host no longer discovers only free-standing entry points. It can also query
an `ITraceProgramLayout` that describes the groups and slots those entry points
belong to.

## Step 2: Define the Payload

The payload is ordinary user data carried by the ray.

```slang
struct RadiancePayload
{
    float4 color;
}
```

### Difference from the Old Model

Shader code:
The payload is still the application-defined ray payload. The main difference is
that stage functions access it through input objects:

```slang
input.payload.color = ...;
```

instead of receiving target-native stage parameters directly.

Host code:
The host still needs to know the payload layout for pipeline compilation and
validation. Reflection should expose the `TraceContext.Payload` type through the
trace program layout.

## Step 3: Define the Trace Context

The trace context names the payload type and target-independent traversal shape.

```slang
struct PrimaryTraceContext : rt::ITraceContext
{
    typealias Payload = RadiancePayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}
```

Primitive-specific Metal data tags are not declared in the trace context. The compiler infers
`triangle_data` and `curve_data` from reachable uses of properties such as `input.triangle` and
`input.curve`.

### Difference from the Old Model

Shader code:
Old code often relies on target builtins and entry-point signatures to imply payload and
acceleration-structure shape. The new API makes the shared shape explicit in a type, while
primitive-specific data requirements are inferred from stage code.

Host code:
The host can use `TraceContext` reflection to validate that the bound
acceleration structure, payload layout, and target capabilities match the trace
program.

## Step 4: Define Group Contexts

Each hit group has a hit context. A hit context connects a trace context to a
primitive kind and a per-record data type. Miss and callable groups use parallel
context types.

```slang
struct PrimaryTriangleContext : rt::IHitContext
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = PrimaryHitRecord;
}

struct PrimarySphereContext : rt::IHitContext
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<SphereHitAttributes>;
    typealias Record = PrimaryHitRecord;
}

struct PrimaryMissContext : rt::IMissGroupContext
{
    typealias TraceContext = PrimaryTraceContext;
    typealias Record = PrimaryMissRecord;
}
```

### Difference from the Old Model

Shader code:
Old code usually expresses this through hit-group setup on the host and through
different closest-hit/any-hit/intersection entry points. The new API also
expresses it in shader source, so the compiler can type-check stage inputs.

Host code:
The host can reflect the primitive kind per hit group instead of inferring it
only from external hit-group setup.

## Step 5: Write Stage Structs

Stages are ordinary structs that implement built-in stage contracts with an
`invoke(...)` method.

Miss:

```slang
struct PrimaryMiss : rt::IMissShader<PrimaryMissContext>
{
    void invoke(rt::MissInput<PrimaryMissContext> input)
    {
        input.payload.color = float4(0.0, 0.0, 0.0, 1.0);
    }
}
```

Closest-hit:

```slang
struct PrimaryTriangleClosestHit
    : rt::IClosestHitShader<PrimaryTriangleContext>
{
    void invoke(rt::ClosestHitInput<PrimaryTriangleContext> input)
    {
        input.payload.color =
            float4(input.triangle.barycentricCoord, input.distance, 1.0);
    }
}
```

Any-hit:

```slang
struct PrimaryTriangleAnyHit : rt::IAnyHitShader<PrimaryTriangleContext>
{
    void invoke(rt::AnyHitInput<PrimaryTriangleContext> input)
    {
        if (!input.triangle.frontFacing)
        {
            input.ignoreHit();
        }
    }
}
```

Procedural intersection:

```slang
struct SphereHitAttributes
{
    float2 uv;
}

struct PrimarySphereIntersection
    : rt::IIntersectionShader<PrimarySphereContext>
{
    void invoke(rt::IntersectionInput<PrimarySphereContext> input)
    {
        SphereHitAttributes attributes;
        attributes.uv = float2(0.25, 0.75);
        input.reportHit(1.0, attributes);
    }
}
```

`reportHit` submits one candidate to traversal and returns whether it was accepted. An
*Intersection* may call it zero, one, or multiple times. If the hit group contains *AnyHit*, that
stage evaluates every non-opaque reported candidate.

### Difference from the Old Model

Shader code:
Old code declares stage entry points directly. The new API declares stage
structs. D3D/Vulkan lowering can still synthesize or expose native stage entry
points from these structs. Metal lowering can call the same `invoke(...)`
methods from generated code.

Host code:
The host should not need to manually guess which function belongs to which hit
group. Reflection over the trace program layout provides that association.

## Step 6: Declare Groups

A slot is the compiled shader-group index used by dispatch. In the current API
sketch, the slot is implicit: it is the zero-based position of a group in
`MissGroupList`, `HitGroupList`, or `CallableGroupList`.

```slang
struct PrimaryMissGroup : rt::IMissGroup
{
    typealias Context = PrimaryMissContext;
    typealias Miss = PrimaryMiss;
}

struct PrimaryTriangleGroup : rt::IHitGroup
{
    typealias Context = PrimaryTriangleContext;
    typealias ClosestHit = PrimaryTriangleClosestHit;
    typealias AnyHit = PrimaryTriangleAnyHit;
    typealias Intersection = rt::NoIntersection<PrimaryTriangleContext>;
}

struct PrimarySphereGroup : rt::IHitGroup
{
    typealias Context = PrimarySphereContext;
    typealias ClosestHit = PrimarySphereClosestHit;
    typealias AnyHit = rt::NoAnyHit<PrimarySphereContext>;
    typealias Intersection = PrimarySphereIntersection;
}
```

These numbers are not geometry IDs. They are logical SBT/function-buffer slots:

```text
slot 0 -> triangle hit group
slot 1 -> sphere hit group
```

### Difference from the Old Model

Shader code:
Old shader code usually does not declare the SBT slot numbers. They are mostly
host-side knowledge. The new API makes the slot-to-shader-group association
visible through group list order in shader source.

Host code:
The host still builds SBT records or Metal function records, but it can now use
the reflected group list positions from shader code as the common contract.

## Step 7: Declare the Trace Program Layout

The trace program layout groups miss and hit shaders by slot.

```slang
struct PrimaryTraceProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = PrimaryTraceContext;

    typealias MissGroups = rt::MissGroupList<
        TraceContext,
        PrimaryMissGroup>;       // miss[0]

    typealias HitGroups = rt::HitGroupList<
        TraceContext,
        PrimaryTriangleGroup,    // hitGroup[0]
        PrimarySphereGroup>;     // hitGroup[1]

    typealias CallableGroups = rt::NoCallableGroups<TraceContext>;
}

rt::TraceProgramDescriptor<PrimaryTraceProgramLayout> gPrimaryDescriptor;
```

### Difference from the Old Model

Shader code:
Old code lets the host assemble hit groups externally. The new API asks shader
source to declare the group structure explicitly.

Host code:
The host can build native target objects from reflected `ITraceProgramLayout`
data: miss group `0`, hit group `0`, hit group `1`, and their associated
shaders. This should reduce target-specific guesswork.

## Step 8: Trace Rays

Ray generation code creates a traversal description. It carries both the ray and
the normalized SBT-style indices.

```slang
[shader("raygeneration")]
void rayGenMain()
{
    rt::RayTraversalDesc desc;
    desc.ray = makePrimaryRay(DispatchRaysIndex().xy);
    desc.instanceMask = 0xff;
    desc.sbtOffset = 0;
    desc.sbtStride = 2;
    desc.missIndex = 0;

    RadiancePayload payload;
    payload.color = float4(0.0, 0.0, 0.0, 1.0);

    rt::RayTracer<PrimaryTraceProgramLayout> tracer;
    tracer.trace(desc, gScene, gPrimaryDescriptor, payload);
}
```

The hit slot formula is:

```text
slot = instanceContribution
     + geometryContribution * desc.sbtStride
     + desc.sbtOffset
```

### Difference from the Old Model

Shader code:
This is close to D3D/Vulkan `TraceRay` arguments, but the names are normalized:

```text
sbtOffset
sbtStride
missIndex
```

Metal shader code now also uses these concepts, because Slang needs them to
synthesize ClosestHit visible-function dispatch.

Host code:
Existing D3D/Vulkan engines already compute these values for native tracing.
The goal is that they can pass the same values through this API. Metal backends
use the same values to configure IFB indexing and generated ClosestHit
visible-function dispatch.

## Step 9: Host Setup for D3D/Vulkan

The host reflects `PrimaryTraceProgramLayout`.

Conceptual reflection:

```text
MissGroups:
    slot 0 -> PrimaryMiss

HitGroups:
    slot 0 -> PrimaryTriangleClosestHit, PrimaryTriangleAnyHit, no intersection
    slot 1 -> PrimarySphereClosestHit, no any-hit, PrimarySphereIntersection
```

Host flow:

1. Compile or generate native miss, closest-hit, any-hit, and intersection entry
   points from the reflected stage structs.
2. Build the D3D/Vulkan ray tracing pipeline with those entry points.
3. Build SBT records so slot `N` corresponds to native SBT record `N`.
4. Lower `RayTraversalDesc.sbtOffset`, `RayTraversalDesc.sbtStride`, and
   `RayTraversalDesc.missIndex` directly to native trace arguments.

### Difference from the Old Model

Shader code:
The shader provides group membership explicitly through `ITraceProgramLayout` instead
of relying on host-only hit-group construction.

Host code:
The host still builds a normal SBT. The difference is that the SBT layout can be
driven from reflection over `ITraceProgramLayout` instead of being a separate
source of truth.

## Step 10: Host Setup for Metal

Metal does not have native closest-hit/miss dispatch. The host and compiler
cooperate differently.

Host flow:

1. Compile the generated Metal shader that contains post-trace Miss and
   ClosestHit dispatch through generated visible-function tables.
2. Populate the generated Miss and ClosestHit visible-function tables from
   `ITraceProgramLayout` reflection.
3. For hit groups with any-hit or intersection shaders, populate the Metal
   `intersection_function_buffer` or `intersection_function_table`.
4. Use the reflected hit-group slots as the function-record indices.
5. Build acceleration-structure geometry and instance metadata so the Metal
   geometry and instance contributions match the slot formula.
6. For IFB, Slang lowering maps:

```text
desc.sbtStride -> intersector::set_geometry_multiplier(...)
desc.sbtOffset -> intersector::set_base_id(...)
```

Then the IFB record index and the generated closest-hit slot match.

### Difference from the Old Model

Shader code:
Metal users no longer write all post-intersect closest-hit dispatch manually.
They declare the same trace program layout groups used by other targets.

The generated Metal code should prefer visible-function dispatch for Miss and
ClosestHit. A literal switch is a useful semantic explanation, but it can force
all reachable stage bodies into one shader compilation unit and increase
register pressure for lightweight handlers when heavyweight handlers are also
present.

Host code:
Metal host setup must make the function table or function buffer slot layout
match `ITraceProgramLayout` reflection. For fully general `sbtOffset/sbtStride`, IFB is
the clean path because it supports `base_id` and `geometry_multiplier`.

## Step 11: Add More Ray Types or Materials

To add a new ray type, allocate more slots and choose `sbtOffset` per ray.

Example layout:

```text
geometry contribution 0:
    slot 0 -> primary triangle
    slot 1 -> shadow triangle

geometry contribution 1:
    slot 2 -> primary sphere
    slot 3 -> shadow sphere
```

Then:

```slang
desc.sbtStride = 2;
desc.sbtOffset = rayType; // 0 for primary, 1 for shadow
```

To add a new material, add another reflected hit group slot and make host-side
geometry/instance metadata select that slot through the standard formula.

### Difference from the Old Model

Shader code:
The shader must declare any new compiled hit groups in `ITraceProgramLayout`. Dynamic
scene data can still choose among those slots at runtime through AS metadata and
dispatch values.

Host code:
The host still controls which geometry/material/ray-type combination maps to
which slot. The difference is that the target-independent slot meaning comes
from `ITraceProgramLayout` reflection.

## Debugging Checklist

If the wrong closest-hit shader runs on Metal:

- Check `desc.sbtOffset`.
- Check `desc.sbtStride`.
- Check `__rtGetGeometryContribution`.
- Check `__rtGetInstanceContribution`.
- Check that Metal IFB uses the same `set_base_id` and
  `set_geometry_multiplier` values.
- Check that the host populated IFB/table entries using the reflected slot
  numbers.

If D3D/Vulkan works but Metal does not, suspect a mismatch between Metal
function index calculation and Slang's synthesized hit-group slot calculation.

If Metal works but D3D/Vulkan does not, suspect SBT construction or generated
native entry-point reflection.
