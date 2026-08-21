# Existing Slang Ray-Payload Model

## Purpose

This document explains how the existing Slang ray-tracing API represents a ray payload. It answers:

1. Which source-level forms use a ray payload?
2. Is a payload mandatory?
3. Which payload types are valid?
4. What does `[raypayload]` mean?
5. Is a payload a host-bound resource?

The discussion distinguishes the existing pipeline API from the proposed structural-dispatch API.
The proposal may reuse the same semantics, but its interface-based shader stages need a different way
to expose the live payload reference.

## Short Answers

The existing API has **two payload representation styles**:

1. The portable, data-based Slang/HLSL style passes an ordinary value by `inout`.
2. The Vulkan GLSL style declares outgoing and incoming payload variables at a numbered location.

Within those styles, payloads appear in **five source-level roles**:

| # | Role | Source shape |
| --- | --- | --- |
| 1 | Start a pipeline trace | `TraceRay(..., inout payload)` or `TraceMotionRay(...)` |
| 2 | Receive the payload in a pipeline stage | `inout Payload payload` in *AnyHit*, *ClosestHit*, or *Miss* |
| 3 | Traverse into a `HitObject` | `HitObject::TraceRay(..., inout payload)` |
| 4 | Invoke deferred hit/miss shading | `HitObject::Invoke(..., inout payload)` |
| 5 | Use Vulkan GLSL payload locations directly | `rayPayloadEXT` and `rayPayloadInEXT` |

`[raypayload]` is **not a sixth payload channel**. It annotates a payload structure with DXR payload
access qualifiers. `SV_RayPayload` is likewise an explicit spelling for the stage-parameter role,
not a separate storage mechanism.

A payload argument is syntactically mandatory for the existing `TraceRay` and `HitObject` methods,
but useful payload data is not mandatory: an empty payload structure can express “no data.” Ray
queries do not use pipeline payloads at all.

A payload is shader-internal ray state. The host cannot bind a payload value through a descriptor or
shader record. The host may configure the maximum payload size, while shader code creates and passes
each actual payload value.

## 1. Mental Model

A payload is an application-defined value associated with one trace operation. It behaves like an
`inout` continuation record passed through shader-stage transitions:

```text
caller initializes P
        |
        v
TraceRay(..., P)
        |
        +-- zero or more AnyHit executions may update P
        |
        +-- committed hit --> ClosestHit may update P
        |
        `-- no committed hit --> Miss may update P
                                  |
                                  v
caller observes the final P
```

An *Intersection* shader does not receive the portable ray payload. A *Callable* shader receives
callable data, which is a separate mechanism. An *AnyHit* shader's payload writes are not rolled
back when it rejects the candidate with `IgnoreHit()`.

The payload is not the same as:

| Data | Owner and purpose |
| --- | --- |
| Ray payload | Mutable state associated with one trace and its invoked stages |
| Hit attributes | Geometry data produced by intersection and consumed by hit stages |
| Callable data | The independent `inout` argument of `CallShader` and *Callable* stages |
| Shader-record data | Per-SBT-record data selected by dispatch |
| Bound resources | Buffers, textures, and samplers supplied by the host |

## 2. Representation Style A: Data-Based `inout` Payloads

This is the ordinary cross-target Slang API. The payload starts as a local shader value, and the
generic API passes it by `inout`.

### 2.1 `TraceRay`: start a pipeline trace

The existing declaration is equivalent to:

```slang
__generic<Payload>
void TraceRay(
    RaytracingAccelerationStructure accelerationStructure,
    uint rayFlags,
    uint instanceMask,
    uint rayContribution,
    uint geometryMultiplier,
    uint missIndex,
    RayDesc ray,
    inout Payload payload);
```

Example:

```slang
struct RadiancePayload
{
    float3 radiance;
    uint recursionDepth;
}

RaytracingAccelerationStructure scene;
RWTexture2D<float4> output;

[shader("raygeneration")]
void rayGeneration()
{
    RayDesc ray = makePrimaryRay();

    RadiancePayload payload;
    payload.radiance = 0.0;
    payload.recursionDepth = 0;

    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload);

    output[DispatchRaysIndex().xy] = float4(payload.radiance, 1.0);
}
```

`TraceMotionRay` has the same payload model and adds a motion-time argument. It is a target-specific
motion-blur variant, not another kind of payload.

### 2.2 Stage entry-point payload

The payload becomes an `inout` parameter of the *AnyHit*, *ClosestHit*, and *Miss* entry points:

```slang
[shader("anyhit")]
void anyHit(
    inout RadiancePayload payload,
    BuiltInTriangleIntersectionAttributes attributes)
{
    if (shouldReject(attributes))
    {
        payload.recursionDepth += 1;
        IgnoreHit();
    }
}

[shader("closesthit")]
void closestHit(
    inout RadiancePayload payload,
    BuiltInTriangleIntersectionAttributes attributes)
{
    payload.radiance = shade(attributes);
}

[shader("miss")]
void miss(inout RadiancePayload payload)
{
    payload.radiance = skyColor(WorldRayDirection());
}
```

The entry points do not call a `GetCurrentPayload()` intrinsic. Slang recognizes an `out` or
`inout` parameter of these three stages as `LayoutResourceKind::RayPayload`. The parameter is
therefore a live reference to the backend payload representation.

Slang also classifies an `out` parameter as a write-only payload for layout purposes. The portable
DXR spelling is nevertheless `inout`; using `out` here relies on Slang target legalization rather
than the native DXR signature.

The payload can be marked explicitly with the semantic:

```slang
[shader("closesthit")]
void closestHit(
    inout RadiancePayload payload : SV_RayPayload,
    BuiltInTriangleIntersectionAttributes attributes : SV_IntersectionAttributes)
{
    payload.radiance = shade(attributes);
}
```

For the HLSL-style stage signature, `SV_RayPayload` does not create a second payload. Direction and
shader stage already let Slang classify the `inout` parameter as the payload. The semantic is an
explicit spelling of the same role.

An *Intersection* entry point receives hit attributes and ray built-ins, but not a ray payload:

```slang
[shader("intersection")]
void intersection()
{
    ProceduralAttributes attributes;
    // ReportHit(..., attributes), but no incoming ray payload parameter.
}
```

### 2.3 `HitObject::TraceRay`: traverse now, shade later

`HitObject::TraceRay` performs traversal, including *Intersection* and *AnyHit*, but defers
*ClosestHit* or *Miss* execution. It still carries an `inout` payload during traversal:

```slang
RadiancePayload payload = makeInitialPayload();

HitObject hit = HitObject::TraceRay(
    scene,
    RAY_FLAG_NONE,
    0xff,
    0,
    1,
    0,
    ray,
    payload);

// AnyHit may already have modified payload. ClosestHit/Miss has not run yet.
MaybeReorderThread(hit);
```

The `HitObject` stores the deferred hit-or-miss information. It does not replace the payload value;
the caller continues to own `payload` separately.

`HitObject::TraceMotionRay` is the corresponding motion variant and uses the same payload model.

### 2.4 `HitObject::Invoke`: execute deferred shading

After optional shader-execution reordering, `HitObject::Invoke` passes a payload to the deferred
*ClosestHit* or *Miss* shader:

```slang
// NVAPI, SPIR-V, and CUDA form.
HitObject::Invoke(scene, hit, payload);

// The DXR native form omits the acceleration-structure argument:
// HitObject::Invoke(hit, payload);

useFinalPayload(payload);
```

The logical deferred flow is:

```text
payload P
    |
HitObject::TraceRay(..., P)  -- Intersection/AnyHit traversal
    |
optional ReorderThread(hit)
    |
HitObject::Invoke(..., P)    -- ClosestHit or Miss
    |
final P
```

The payload type passed to traversal and invocation must agree with the shaders that can be selected.

## 3. Representation Style B: Vulkan GLSL Payload Locations

Slang also parses the native GLSL representation. The calling shader declares an outgoing payload
at a numeric location:

```glsl
struct RadiancePayload
{
    vec3 radiance;
    uint recursionDepth;
};

layout(location = 0) rayPayloadEXT RadiancePayload outgoingPayload;

void main()
{
    outgoingPayload.radiance = vec3(0.0);
    outgoingPayload.recursionDepth = 0;

    traceRayEXT(
        scene,
        gl_RayFlagsNoneEXT,
        0xff,
        0,
        1,
        0,
        origin,
        tMin,
        direction,
        tMax,
        0); // Payload location.
}
```

The invoked shader declares a matching incoming payload location:

```glsl
layout(location = 0) rayPayloadInEXT RadiancePayload incomingPayload;

void main()
{
    incomingPayload.radiance = skyColor(gl_WorldRayDirectionEXT);
}
```

The location connects the outgoing `RayPayloadKHR` variable to an
`IncomingRayPayloadKHR` variable. Their types must match for the selected shader path.

Ordinary Slang `TraceRay` hides this representation. Its Vulkan lowering conceptually generates:

```slang
[__vulkanRayPayload]
static Payload outgoing;

outgoing = payload;
__traceRay(..., __rayPayloadLocation(outgoing));
payload = outgoing;
```

`[__vulkanRayPayload]` and `__rayPayloadLocation` are compiler-facing implementation details, not
the recommended portable user API.

## 4. Is a Payload Mandatory?

The answer depends on which operation is being discussed.

### 4.1 Pipeline trace calls: an argument is mandatory

The existing `TraceRay`, `TraceMotionRay`, `HitObject::TraceRay`, and `HitObject::Invoke` signatures
all contain a payload parameter. There is no data-based overload that omits it.

No useful data needs to be carried, however. An empty structure provides a portable source-level
way to satisfy the signature:

```slang
struct EmptyPayload
{
}

EmptyPayload payload;
TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload);
```

Slang has backend legalization for empty payload structures and may add a dummy field when a target
does not accept a zero-sized payload.

### 4.2 Invoked shader stages: portable DXR requires the parameter

DXR requires *AnyHit*, *ClosestHit*, and *Miss* shaders to declare a matching payload structure,
even when a particular shader never reads it. A portable Slang stage should therefore keep the
parameter:

```slang
[shader("miss")]
void miss(inout EmptyPayload payload)
{
}
```

The current Slang compiler accepts a payload-less *Miss* function when targeting SPIR-V, because
Vulkan can omit an unused incoming payload interface variable. That form is not portable to DXR and
should not define the cross-target API contract.

### 4.3 Ray queries: no pipeline payload

`RayQuery.TraceRayInline` performs inline traversal in the calling shader. It does not dispatch the
pipeline *AnyHit*, *ClosestHit*, or *Miss* stages, so it has no pipeline payload parameter:

```slang
RayQuery<RAY_FLAG_NONE> query;
query.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);

float visibility = 1.0;
while (query.Proceed())
{
    // visibility is an ordinary local variable, not a ray payload.
}
```

### 4.4 Metal's native intersector: payload overloads are optional

Metal provides both payload-free and payload-bearing `intersector::intersect` overloads. A payload
is optional in native Metal traversal. Existing Slang `TraceRay` does not target Metal; the proposed
structural API must decide whether to expose an explicit empty payload or select a payload-free Metal
overload when no stage uses payload data.

## 5. Payload Type Restrictions

### 5.1 What the current Slang signature enforces

Very little is expressed in the generic signature:

```slang
__generic<Payload>
void TraceRay(..., inout Payload payload);
```

There is no `where` constraint or payload interface. Existing tests demonstrate structure,
scalar/vector, and empty-structure payloads. For HLSL, Slang wraps a non-structure value in a
generated structure and copies it back after `TraceRay`.

This source-level permissiveness does not mean every Slang type is a legal native payload. For
example, resource objects and opaque handles cannot be assumed to survive all target payload ABIs.
The current generic declaration can allow such a type to reach backend legalization before failing,
instead of diagnosing it at the API boundary.

### 5.2 Required type agreement

The payload type is part of the trace ABI:

- The value passed to `TraceRay` must match every reachable *AnyHit*, *ClosestHit*, and *Miss*
  payload parameter.
- Vulkan outgoing and incoming variables connected through one location must have matching types.
- Payloads used with `HitObject` traversal and invocation must match the selected shaders.

The native pipeline or driver may perform part of this validation when shaders are compiled or
linked separately. A structural Slang API can provide a stronger compile-time guarantee by using
one associated payload type for the whole trace program.

### 5.3 `[raypayload]` type restrictions

`[raypayload]` can only be attached to a `struct`. Its members carry DXR payload access qualifiers.
The DXR PAQ model supports scalar, vector, matrix, array, and structure value data. Nested payload
structures carry their own member qualifiers.

Slang currently checks that every direct field of an explicitly annotated structure has at least a
`read` or `write` qualifier and checks that qualifier stage names are one of:

```text
caller, anyhit, closesthit, miss
```

### 5.4 Target size and layout

Payload size is part of native pipeline configuration and affects performance:

- D3D pipeline configuration traditionally supplies `MaxPayloadSizeInBytes`. With enabled DXR
  payload access qualifiers, the driver can derive more precise per-field lifetimes.
- Vulkan pipeline interfaces specify `maxPipelineRayPayloadSize`.
- Metal has no equivalent host-bound payload buffer; the templated value is copied through
  `thread` and `ray_data` storage.
- OptiX may use payload registers for small supported values and a pointer-backed representation
  for larger values in Slang's CUDA lowering.

Even where the API permits a large payload, keeping it small reduces register pressure and spill
risk.

### 5.5 Recommended portable subset

Until Slang defines and validates an exact cross-target payload constraint, portable payloads should
be structures composed of:

- scalar values;
- vectors and matrices of ordinary numeric scalar types;
- fixed-size arrays of valid value types; and
- nested structures composed of the same types.

Portable payloads should avoid:

- textures, samplers, acceleration structures, and other opaque resource handles;
- runtime-sized arrays;
- stored references or pointers;
- atomics and synchronization objects; and
- non-copyable types.

Metal natively permits some device/constant pointers and references in `ray_data`, but including
them in the portable subset would not map uniformly to D3D and Vulkan.

## 6. What `[raypayload]` Means

`[raypayload]` declares that a structure uses DXR payload access qualifiers (PAQs):

```slang
[raypayload]
struct RadiancePayload
{
    float3 radiance
        : read(caller)
        : write(closesthit, miss);

    uint seed
        : read(anyhit, closesthit, miss, caller)
        : write(caller, anyhit, closesthit);
}
```

The annotation describes field traffic across stage boundaries:

| Qualifier | Meaning |
| --- | --- |
| `write(caller)` | The caller's initial field value is supplied to the trace |
| `read(caller)` | The trace's final field value is returned to the caller |
| `read(anyhit)` | *AnyHit* receives the last preserved value of the field |
| `write(anyhit)` | An executed *AnyHit* publishes the field for later stages |
| `read(closesthit)` / `write(closesthit)` | Corresponding *ClosestHit* input/output traffic |
| `read(miss)` / `write(miss)` | Corresponding *Miss* input/output traffic |

PAQs are lifetime and data-transfer declarations. They let a DXR implementation avoid preserving
fields in stages that cannot consume them, reducing register pressure or spilling.

They are not ordinary access-control rules inside a function. A local payload structure still acts
like an ordinary local value. If code accesses a field inconsistently with its declared PAQs, values
can become undefined or writes can be discarded at stage transitions.

In particular, a field with `write(closesthit)` but not `read(closesthit)` is a write-only output of
that stage. If *ClosestHit* executes, it must fully initialize the field; otherwise the published
value is undefined. A field that is conditionally updated normally needs both `read(closesthit)` and
`write(closesthit)` so that the previous value is preserved along the branch that does not assign it.

### 6.1 Unannotated payload

An ordinary structure remains a payload when it is passed to `TraceRay`:

```slang
struct Payload
{
    float3 color;
}
```

For HLSL Shader Model 6.7+, Slang can mark the inferred payload structure with `[raypayload]` and
conservatively add every stage as both a reader and writer:

```slang
float3 color
    : read(caller, anyhit, closesthit, miss)
    : write(caller, anyhit, closesthit, miss);
```

This is correct but prevents the optimizations provided by precise annotations.

DXR introduced PAQs as an opt-in feature in Shader Model 6.6 and enables them by default in Shader
Model 6.7 and later. The annotations do not define Vulkan payload locations and have no direct Metal
equivalent; those targets use their own payload representation.

### 6.2 `[raypayload]` does not identify a particular variable

These concepts must remain separate:

```text
inout parameter / Vulkan payload variable
    identifies the live payload storage for one trace

[raypayload] on a struct type
    describes DXR field access across stage transitions
```

The same annotated structure can also be used as an ordinary local value or ordinary helper-function
parameter. The attribute does not turn every instance of that type into an active ray payload.

## 7. Is a Payload a Host-Bound Resource?

No. Slang internally gives payload parameters a resource-layout category, but that is compiler
classification, not a descriptor binding.

| Property | Ray payload | Group-shared memory | Bound buffer/texture |
| --- | --- | --- | --- |
| Created by | Shader trace call/runtime | Shader workgroup invocation | Host application |
| Visibility | One trace and its selected shader stages | Threads in one workgroup | Shaders with the binding |
| Host binds the value | No | No | Yes |
| Descriptor/register slot | No | No | Yes |
| Lifetime | Trace and stage transitions | Workgroup execution | Resource allocation |
| Typical implementation | Registers, stack, or private implementation storage | On-chip/shared memory | Device memory |

The payload is therefore shader-internal like group-shared memory only in the limited sense that the
host does not bind its contents. Its ownership and visibility are very different: it is per trace,
not shared among a thread group.

The host still has three indirect responsibilities:

1. Build a pipeline whose selected stages agree on the payload ABI.
2. Configure or accept the maximum native payload size where required.
3. Bind ordinary resources that shader code may use to initialize a payload or store its final
   result.

For example, the host can bind an input buffer, and ray-generation code can copy a buffer element
into a new payload. The buffer is host-bound; the resulting payload is not. Similarly, the shader
must copy the final payload result into a UAV or output buffer if the host needs to read it.

Slang reflection exposes `SLANG_PARAMETER_CATEGORY_RAY_PAYLOAD`, but ray-payload parameters are
excluded from ordinary descriptor-space allocation. The category tells the compiler and reflection
consumer what the entry-point parameter means; it does not produce a bindable slot.

## 8. Backend Mapping in Existing Slang

### 8.1 D3D/DXR

The source `inout` value maps closely to DXR's `TraceRay` argument and stage parameters. If the
source payload is not a structure, Slang creates a temporary wrapper structure. For Shader Model
6.7+, Slang emits `[raypayload]` and field access annotations, using conservative annotations when
the user did not provide precise ones.

### 8.2 Vulkan

At the trace call, Slang copies the ordinary local value into a generated `RayPayloadKHR` variable,
passes its location to `OpTraceRayKHR`, and copies the result back. At an invoked stage, Slang turns
the `inout` entry-point parameter into an `IncomingRayPayloadKHR` global variable.

If a Slang entry point declares several `out`/`inout` payload parameters, the Vulkan legalizer
consolidates them into one generated incoming payload structure. This is a Slang source convenience;
the Vulkan shader still has at most one incoming ray-payload object.

### 8.3 CUDA/OptiX

The same source-level entry parameter receives OptiX payload data. Slang's current lowering may use
inline payload registers for a small supported type or a packed pointer representation for a larger
or unsupported register representation.

### 8.4 Metal

Existing Slang pipeline `TraceRay` is not enabled for Metal, and the Metal layout family currently
does not supply ray-payload entry-parameter rules. Native Metal instead has optional
`intersector::intersect(..., thread T& payload)` overloads and exposes that value to an intersection
function as `ray_data T& [[payload]]`.

The proposed structural-dispatch API must add the Metal mapping rather than assuming the existing
pipeline-entry mechanism already handles it.

## 9. Consequences for the Structural-Dispatch Proposal

The current proposal's high-level choices are compatible with the existing model:

```slang
interface ITraceContext
{
    associatedtype Payload;
}

void trace(..., inout TraceContext.Payload payload);
```

Using one associated type guarantees that every structurally reachable shader group agrees on the
payload type. That is stronger than relying on native pipeline-link validation.

The stage-input implementation still needs correction. An ordinary stored field such as:

```slang
Payload payloadStorage;
```

is only a value copy when the stage input is passed by value. A `ref` property returning that field
does not automatically alias the original `trace(..., inout payload)` argument.

The compiler must instead do one of the following:

1. Back `input.payload` with a compiler-known reference to the generated native payload parameter.
2. Thread an explicit payload reference through the generated stage wrapper.
3. Use a generated copy-in/copy-out input passed as `inout` rather than by value.

The first option best preserves the proposed surface API. The compiler-generated DXR/Vulkan entry
point would still declare the conventional live `inout` payload parameter, while the Metal wrapper
would map the same logical reference to `ray_data [[payload]]` during traversal and `thread T&`
during synthesized post-trace dispatch.

The proposal should also define:

- a built-in empty payload type for programs that carry no data;
- compile-time validation of the portable payload type subset;
- payload-size reflection for host pipeline construction; and
- conservative PAQs initially, with possible field-use inference as a later optimization.

## 10. Source References

Relevant implementation points in this repository:

- `TraceRay`, Vulkan copy-in/copy-out, and HLSL temporary wrapping:
  [`source/slang/hlsl.meta.slang`](../../../../source/slang/hlsl.meta.slang)
- Ray-tracing entry-parameter classification:
  [`source/slang/slang-parameter-binding.cpp`](../../../../source/slang/slang-parameter-binding.cpp)
- Vulkan entry-parameter globalization and consolidation:
  [`source/slang/slang-ir-glsl-legalize.cpp`](../../../../source/slang/slang-ir-glsl-legalize.cpp)
- `[raypayload]` semantic validation:
  [`source/slang/slang-check-modifier.cpp`](../../../../source/slang/slang-check-modifier.cpp)
- HLSL payload wrapping and default PAQ generation:
  [`source/slang/slang-ir-hlsl-legalize.cpp`](../../../../source/slang/slang-ir-hlsl-legalize.cpp)
- Public reflection category:
  [`include/slang.h`](../../../../include/slang.h)
- Host pipeline maximum-payload setting:
  [`include/slang-gfx.h`](../../../../include/slang-gfx.h)

External specifications:

- [DirectX Raytracing functional specification](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html)
- [Vulkan ray-tracing pipeline extension](https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_ray_tracing_pipeline.html)
- [SPIR-V specification](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html)
