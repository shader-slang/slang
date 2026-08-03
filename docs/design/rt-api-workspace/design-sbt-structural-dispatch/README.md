# SBT Structural Dispatch Design

This directory sketches a Metal-first pipeline ray tracing API that still maps
to D3D/Vulkan's native shader binding table model.

Start with [PROPOSAL.md](PROPOSAL.md) for the formal draft design, including
problem diagrams, API sketches, and migration examples. Use
[TUTORIAL.md](TUTORIAL.md) for a step-by-step user-facing walkthrough.
[SESSION_CONTEXT.md](SESSION_CONTEXT.md) records the compact context and
candidate-selection rationale for the draft PR.

[CALLABLE_SHADER_CONCERNS.md](CALLABLE_SHADER_CONCERNS.md) records open
concerns about fitting D3D/Vulkan callable shaders into the `TraceProgram`
model. It is intentionally separate from the main proposal while that part of
the design is still being evaluated.

## Problem

D3D and Vulkan have native pipeline ray tracing stages. A trace call can select
a miss shader, any-hit shader, intersection shader, and closest-hit shader
through SBT indexing. The shader author writes stage entry points, and the
driver/hardware performs the dispatch.

Metal is different. Metal traversal can call custom intersection functions
through an `intersection_function_table` or `intersection_function_buffer`, but
Metal does not have native closest-hit or miss shader stages. A Metal
`intersector::intersect(...)` call returns an `intersection_result`, and the
caller shader must decide what to do next.

That creates the central portability problem:

```text
D3D/Vulkan:
    TraceRay(...) dispatches closest-hit for us.

Metal:
    intersector.intersect(...) returns a result.
    Slang must synthesize closest-hit and miss dispatch after the trace.
```

We need an API that lets Slang reconstruct the missing Metal closest-hit/miss
dispatch without imposing a new host-side model that blocks existing D3D/Vulkan
users from migrating.

## Design Choice

There are two possible directions.

### Option 1: Analyze User-Written Dispatch Code

The source could let users write ordinary shader logic after `trace(...)`:

```slang
let result = tracer.trace(...);
if (result.isNone)
    shadeMiss(payload);
else
    switch (result.geometryIndex)
    {
    case 0: shadeTriangle(payload, result); break;
    case 1: shadeCurve(payload, result); break;
    }
```

This is natural for Metal. However, it makes D3D/Vulkan difficult because the
compiler would need to analyze arbitrary user code, infer the dispatch behavior,
and expose enough information for the host to reconstruct native SBT records.
That analysis is brittle, hard to explain, and easy to break with dynamic code.

### Option 2: Declare Structural Hit/Miss Groups

The current design asks the user to declare the grouping explicitly:

```slang
struct PrimaryProgram : rt::TraceProgram
{
    typealias TraceContext = PrimaryTraceContext;

    typealias MissGroups = rt::MissGroupList<...>;
    typealias HitGroups = rt::HitGroupList<...>;
}
```

Each group is assigned a slot:

```slang
typealias PrimaryTriangleHitSlot = rt::HitGroupSlot<0>;

rt::HitGroup<
    TraceContext,
    PrimaryTriangleHitSlot,
    PrimaryTriangleContext,
    PrimaryTriangleClosestHit,
    PrimaryTriangleAnyHit,
    rt::NoAttributes,
    rt::NoIntersection<PrimaryTriangleContext>>
```

This gives the compiler a finite, explicit list of logical hit groups. D3D and
Vulkan can map those groups to native SBT records. Metal can synthesize a
post-`intersect(...)` switch over the same slots.

This is the chosen direction for this sketch.

## Slot Model

A slot is a compiled shader-group index. It is not a geometry ID, material ID,
or primitive ID.

In the example:

```slang
typealias PrimaryTriangleHitSlot = rt::HitGroupSlot<0>;
typealias PrimaryCurveHitSlot = rt::HitGroupSlot<1>;
typealias PrimarySphereHitSlot = rt::HitGroupSlot<2>;
```

means:

```text
hit slot 0 -> triangle hit group
hit slot 1 -> curve hit group
hit slot 2 -> sphere hit group
```

The slot is selected by the same SBT-style formula on all targets:

```text
slot = instanceContribution
     + geometryContribution * dispatch.sbtStride
     + dispatch.sbtOffset
```

The API spells this as:

```slang
public struct RayDispatch
{
    public uint sbtOffset;
    public uint sbtStride;
    public uint missIndex;
}
```

and:

```slang
public uint getHitGroupSlot(
    RayDispatch dispatch,
    uint geometryContribution,
    uint instanceContribution)
{
    return instanceContribution + geometryContribution * dispatch.sbtStride +
        dispatch.sbtOffset;
}
```

This formula is the load-bearing contract of the design.

## Metal Lowering

On Metal, Slang lowers:

```slang
rt::RayTracer<PrimaryProgram> tracer;
tracer.trace(desc, scene, dispatch, payload);
```

into the shape:

```slang
metalIntersector.set_geometry_multiplier(dispatch.sbtStride);
metalIntersector.set_base_id(dispatch.sbtOffset);
result = metalIntersector.intersect(desc.ray, scene, ...);

if (result.type == none)
{
    switch (dispatch.missIndex)
    {
    case 0: PrimaryMiss()(missInput); break;
    }
}
else
{
    uint geometryContribution =
        __rtGetGeometryContribution<PrimaryProgram>(result);
    uint instanceContribution =
        __rtGetInstanceContribution<PrimaryProgram>(result);
    uint slot = getHitGroupSlot(
        dispatch, geometryContribution, instanceContribution);

    switch (slot)
    {
    case 0: PrimaryTriangleClosestHit()(triangleInput); break;
    case 1: PrimaryCurveClosestHit()(curveInput); break;
    case 2: PrimarySphereClosestHit()(sphereInput); break;
    }
}
```

Metal still dispatches any-hit/intersection logic during traversal through the
function table or function buffer. Slang only synthesizes the missing
closest-hit/miss dispatch after traversal.

The key invariant is:

```text
the slot used for Metal any-hit/intersection dispatch
must be the same slot used for synthesized closest-hit dispatch
```

For `intersection_function_buffer`, this maps cleanly:

```text
dispatch.sbtStride -> intersector::set_geometry_multiplier(...)
dispatch.sbtOffset -> intersector::set_base_id(...)
```

The IFB record index and the synthesized closest-hit slot are then the same
number.

For ordinary `intersection_function_table`, Metal does not expose shader-side
`base_id` or `geometry_multiplier`. That path only matches the general slot
model when the acceleration structure's function-table offsets are already
baked to the same slot numbers, or when the trace uses the restricted layout
that those baked offsets represent.

## D3D/Vulkan Lowering

D3D and Vulkan already implement this kind of slot selection natively.

Slang lowers:

```slang
tracer.trace(desc, scene, dispatch, payload);
```

to native trace parameters:

```text
dispatch.sbtOffset -> ray contribution / SBT record offset
dispatch.sbtStride -> geometry contribution multiplier / SBT record stride
dispatch.missIndex -> miss shader index
```

The native target dispatches the selected SBT record. Slang does not synthesize
the closest-hit switch on D3D/Vulkan.

## Host-Side Reflection Model

The host should be able to query `TraceProgram` reflection data and build the
target-specific dispatch resources from it.

Conceptually, reflection for a trace program exposes:

```text
TraceProgram
    TraceContext
        Payload type
        Acceleration-structure kind
        Motion mode
        Metal data tags / max levels

    MissGroups[]
        slot index
        miss shader symbol

    HitGroups[]
        slot index
        hit context
        primitive kind
        closest-hit shader symbol
        any-hit shader symbol, or none
        intersection shader symbol, or none
        intersection attribute type
        required Metal intersection tags
```

### D3D/Vulkan Host Flow

1. Compile or synthesize native entry points for each reflected miss group and
   hit group.
2. Build the native ray tracing pipeline from those entry points.
3. Build SBT records so reflected slot `N` corresponds to native hit/miss record
   `N`.
4. Use `RayDispatch.sbtOffset`, `RayDispatch.sbtStride`, and
   `RayDispatch.missIndex` in trace calls exactly as the existing engine uses
   native SBT indices.

### Metal Host Flow

1. Compile the generated Metal shader that contains the post-trace miss and
   closest-hit switches.
2. For hit groups with any-hit or intersection functions, populate the Metal
   `intersection_function_buffer` or `intersection_function_table` using the
   reflected slot numbers.
3. Build acceleration-structure geometry and instance metadata so the Metal
   function index contributions match `geometryContribution` and
   `instanceContribution`.
4. For IFB, Slang lowering sets `geometry_multiplier` from
   `RayDispatch.sbtStride` and `base_id` from `RayDispatch.sbtOffset`.
5. The host uses the same slot layout for Metal function records and for
   D3D/Vulkan SBT records.

This reflection model is intended to avoid a Metal-only host-side concept. The
host still thinks in terms of trace programs and hit/miss groups; only the
target backend decides whether those groups become SBT records or generated
Metal switch cases.

## Open Issues

- Define precisely how `__rtGetGeometryContribution` and
  `__rtGetInstanceContribution` are represented on Metal for primitive AS,
  instance AS, ordinary function tables, and IFB.
- Decide whether ordinary `intersection_function_table` should be a restricted
  mode or whether the baseline should require IFB for fully general
  `sbtOffset/sbtStride`.
- Define the exact reflection API shape for enumerating `TraceProgram`,
  `HitGroups`, and `MissGroups`.
- Define how generated D3D/Vulkan entry-point names are exposed in reflection.
