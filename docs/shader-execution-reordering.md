Shader Execution Reordering (SER)
=================================

Slang provides support for Shader Execution Reordering (SER) across multiple backends:

- **D3D12**: Via [NVAPI](nvapi-support.md) or native DXR 1.3 (SM 6.9)
- **Vulkan/SPIR-V**: Via [GL_NV_shader_invocation_reorder](https://github.com/KhronosGroup/GLSL/blob/master/extensions/nv/GLSL_NV_shader_invocation_reorder.txt) (NV) or [GL_EXT_shader_invocation_reorder](https://github.com/KhronosGroup/GLSL/blob/main/extensions/ext/GLSL_EXT_shader_invocation_reorder.txt) (cross-vendor EXT)
- **CUDA**: Via OptiX

## Platform Notes

### Vulkan (NV Extension)

With `GL_NV_shader_invocation_reorder`, `HitObject` variables have special allocation semantics with limitations around flow control and assignment.

### Vulkan (EXT Extension)

The cross-vendor `GL_EXT_shader_invocation_reorder` extension provides broader compatibility. Note that `MakeHit` and `MakeMotionHit` are **NV-only** and not available with the EXT extension.

### D3D12 (DXR 1.3)

Native DXR 1.3 support (SM 6.9) provides `HitObject` without requiring NVAPI.

## Links

* [SER white paper for NVAPI](https://developer.nvidia.com/sites/default/files/akamai/gameworks/ser-whitepaper.pdf)

# API Reference

The HitObject API provides cross-platform SER functionality. The API is based on the NvAPI/DXR 1.3 interface.

## Free Functions

* [ReorderThread](#reorder-thread)

## Fused Functions (Vulkan EXT only)

* [ReorderExecute](#reorder-execute)
* [TraceReorderExecute](#trace-reorder-execute)
* [TraceMotionReorderExecute](#trace-motion-reorder-execute)

--------------------------------------------------------------------------------
# `struct HitObject`

## Description

Immutable data type representing a ray hit or a miss. Can be used to invoke hit or miss shading,
or as a key in ReorderThread. Created by one of several methods described below. HitObject
and its related functions are available in raytracing shader types only.

## Methods

* [TraceRay](#trace-ray)
* [TraceMotionRay](#trace-motion-ray)
* [MakeMiss](#make-miss)
* [MakeHit](#make-hit)
* [MakeMotionHit](#make-motion-hit)
* [MakeMotionMiss](#make-motion-miss)
* [MakeNop](#make-nop)
* [FromRayQuery](#from-ray-query)
* [Invoke](#invoke)
* [IsMiss](#is-miss)
* [IsHit](#is-hit)
* [IsNop](#is-nop)
* [GetRayDesc](#get-ray-desc)
* [GetRayFlags](#get-ray-flags)
* [GetRayTMin](#get-ray-tmin)
* [GetRayTCurrent](#get-ray-tcurrent)
* [GetWorldRayOrigin](#get-world-ray-origin)
* [GetWorldRayDirection](#get-world-ray-direction)
* [GetShaderTableIndex](#get-shader-table-index)
* [SetShaderTableIndex](#set-shader-table-index)
* [GetInstanceIndex](#get-instance-index)
* [GetInstanceID](#get-instance-id)
* [GetGeometryIndex](#get-geometry-index)
* [GetPrimitiveIndex](#get-primitive-index)
* [GetHitKind](#get-hit-kind)
* [GetAttributes](#get-attributes)
* [GetTriangleVertexPositions](#get-triangle-vertex-positions)
* [GetWorldToObject](#get-world-to-object)
* [GetObjectToWorld](#get-object-to-world)
* [GetCurrentTime](#get-current-time)
* [GetObjectRayOrigin](#get-object-ray-origin)
* [GetObjectRayDirection](#get-object-ray-direction)
* [GetShaderRecordBufferHandle](#get-shader-record-buffer-handle)
* [GetClusterID](#get-cluster-id)
* [GetSpherePositionAndRadius](#get-sphere-position-and-radius)
* [GetLssPositionsAndRadii](#get-lss-positions-and-radii)
* [IsSphereHit](#is-sphere-hit)
* [IsLssHit](#is-lss-hit)
* [LoadLocalRootTableConstant](#load-local-root-table-constant)

--------------------------------------------------------------------------------
<a id="trace-ray"></a>
# `HitObject.TraceRay`

## Description

Executes ray traversal (including anyhit and intersection shaders) like TraceRay, but returns the
resulting hit information as a HitObject and does not trigger closesthit or miss shaders.

## Signature 

```
static HitObject HitObject.TraceRay<payload_t>(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                 RayFlags,
    uint                 InstanceInclusionMask,
    uint                 RayContributionToHitGroupIndex,
    uint                 MultiplierForGeometryContributionToHitGroupIndex,
    uint                 MissShaderIndex,
    RayDesc              Ray,
    inout payload_t      Payload);
```

--------------------------------------------------------------------------------
<a id="trace-motion-ray"></a>
# `HitObject.TraceMotionRay`

## Description

Executes motion ray traversal (including anyhit and intersection shaders) like TraceRay, but returns the
resulting hit information as a HitObject and does not trigger closesthit or miss shaders.

**Note**: Requires motion blur support. Available on Vulkan (NV/EXT) and CUDA.

## Signature 

```
static HitObject HitObject.TraceMotionRay<payload_t>(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                 RayFlags,
    uint                 InstanceInclusionMask,
    uint                 RayContributionToHitGroupIndex,
    uint                 MultiplierForGeometryContributionToHitGroupIndex,
    uint                 MissShaderIndex,
    RayDesc              Ray,
    float                CurrentTime,
    inout payload_t      Payload);
```


--------------------------------------------------------------------------------
<a id="make-hit"></a>
# `HitObject.MakeHit`

## Description

Creates a HitObject representing a hit based on values explicitly passed as arguments, without
tracing a ray. The primitive specified by AccelerationStructure, InstanceIndex, GeometryIndex,
and PrimitiveIndex must exist. The shader table index is computed using the formula used with
TraceRay. The computed index must reference a valid hit group record in the shader table. The
Attributes parameter must either be an attribute struct, such as
BuiltInTriangleIntersectionAttributes, or another HitObject to copy the attributes from.

**Note**: This function is **NV-only** and not available with the cross-vendor EXT extension.

## Signature 

```
static HitObject HitObject.MakeHit<attr_t>(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                 InstanceIndex,
    uint                 GeometryIndex,
    uint                 PrimitiveIndex,
    uint                 HitKind,
    uint                 RayContributionToHitGroupIndex,
    uint                 MultiplierForGeometryContributionToHitGroupIndex,
    RayDesc              Ray,
    attr_t               attributes);
static HitObject HitObject.MakeHit<attr_t>(
    uint                 HitGroupRecordIndex,
    RaytracingAccelerationStructure AccelerationStructure,
    uint                 InstanceIndex,
    uint                 GeometryIndex,
    uint                 PrimitiveIndex,
    uint                 HitKind,
    RayDesc              Ray,
    attr_t               attributes);
```

--------------------------------------------------------------------------------
<a id="make-motion-hit"></a>
# `HitObject.MakeMotionHit`

## Description

See MakeHit but handles Motion.

**Note**: This function is **NV-only** and not available with the cross-vendor EXT extension.

## Signature 

```
static HitObject HitObject.MakeMotionHit<attr_t>(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                 InstanceIndex,
    uint                 GeometryIndex,
    uint                 PrimitiveIndex,
    uint                 HitKind,
    uint                 RayContributionToHitGroupIndex,
    uint                 MultiplierForGeometryContributionToHitGroupIndex,
    RayDesc              Ray,
    float                CurrentTime,
    attr_t               attributes);
static HitObject HitObject.MakeMotionHit<attr_t>(
    uint                 HitGroupRecordIndex,
    RaytracingAccelerationStructure AccelerationStructure,
    uint                 InstanceIndex,
    uint                 GeometryIndex,
    uint                 PrimitiveIndex,
    uint                 HitKind,
    RayDesc              Ray,
    float                CurrentTime,
    attr_t               attributes);
```

--------------------------------------------------------------------------------
<a id="make-miss"></a>
# `HitObject.MakeMiss`

## Description

Creates a HitObject representing a miss based on values explicitly passed as arguments, without
tracing a ray. The provided shader table index must reference a valid miss record in the shader
table.

## Signature 

```
static HitObject HitObject.MakeMiss(
    uint                 MissShaderIndex,
    RayDesc              Ray);
```

--------------------------------------------------------------------------------
<a id="make-motion-miss"></a>
# `HitObject.MakeMotionMiss`

## Description

See MakeMiss but handles Motion. Available on Vulkan (NV and EXT extensions).

## Signature 

```
static HitObject HitObject.MakeMotionMiss(
    uint                 MissShaderIndex,
    RayDesc              Ray,
    float                CurrentTime);
```

--------------------------------------------------------------------------------
<a id="make-nop"></a>
# `HitObject.MakeNop`

## Description

Creates a HitObject representing “NOP” (no operation) which is neither a hit nor a miss. Invoking a
NOP hit object using HitObject::Invoke has no effect. Reordering by hit objects using
ReorderThread will group NOP hit objects together. This can be useful in some reordering
scenarios where future control flow for some threads is known to process neither a hit nor a
miss.

## Signature

```
static HitObject HitObject.MakeNop();
```

--------------------------------------------------------------------------------
<a id="from-ray-query"></a>
# `HitObject.FromRayQuery`

## Description

Creates a HitObject from a committed RayQuery hit. The RayQuery must have a committed hit
(either triangle or procedural). If the RayQuery has no committed hit, the resulting HitObject
will represent a miss or NOP depending on the query state.

**Note**: **DXR 1.3 only**. Also available on Vulkan EXT via `hitObjectRecordFromQueryEXT`.

## Signature

```
static HitObject HitObject.FromRayQuery<RayQuery_t>(
    RayQuery_t           Query);
static HitObject HitObject.FromRayQuery<RayQuery_t, attr_t>(
    RayQuery_t           Query,
    uint                 CommittedCustomHitKind,
    attr_t               CommittedCustomAttribs);
```

--------------------------------------------------------------------------------
<a id="invoke"></a>
# `HitObject.Invoke`

## Description

Invokes closesthit or miss shading for the specified hit object. In case of a NOP HitObject, no
shader is invoked.

## Signature

```
static void HitObject.Invoke<payload_t>(
    RaytracingAccelerationStructure AccelerationStructure,
    HitObject            HitOrMiss,
    inout payload_t      Payload);

// DXR 1.3 overload (without AccelerationStructure)
static void HitObject.Invoke<payload_t>(
    HitObject            HitOrMiss,
    inout payload_t      Payload);
```

--------------------------------------------------------------------------------
<a id="is-miss"></a>
# `HitObject.IsMiss`

## Description

Returns true if the HitObject encodes a miss, otherwise returns false.

## Signature 

```
bool HitObject.IsMiss();
```

--------------------------------------------------------------------------------
<a id="is-hit"></a>
# `HitObject.IsHit`

## Description

Returns true if the HitObject encodes a hit, otherwise returns false.

## Signature 

```
bool HitObject.IsHit();
```

--------------------------------------------------------------------------------
<a id="is-nop"></a>
# `HitObject.IsNop`

## Description

Returns true if the HitObject encodes a nop, otherwise returns false.

## Signature 

```
bool HitObject.IsNop();
```

--------------------------------------------------------------------------------
<a id="get-ray-desc"></a>
# `HitObject.GetRayDesc`

## Description

Queries ray properties from HitObject. Valid if the hit object represents a hit or a miss.

## Signature

```
RayDesc HitObject.GetRayDesc();
```

--------------------------------------------------------------------------------
<a id="get-ray-flags"></a>
# `HitObject.GetRayFlags`

## Description

Returns the ray flags used when tracing the ray. Valid if the hit object represents a hit or a miss.

**Note**: **DXR 1.3 and Vulkan EXT**.

## Signature

```
uint HitObject.GetRayFlags();
```

--------------------------------------------------------------------------------
<a id="get-ray-tmin"></a>
# `HitObject.GetRayTMin`

## Description

Returns the minimum T value of the ray. Valid if the hit object represents a hit or a miss.

**Note**: **DXR 1.3 and Vulkan EXT**.

## Signature

```
float HitObject.GetRayTMin();
```

--------------------------------------------------------------------------------
<a id="get-ray-tcurrent"></a>
# `HitObject.GetRayTCurrent`

## Description

Returns the current T value (hit distance) of the ray. Valid if the hit object represents a hit or a miss.

**Note**: **DXR 1.3 and Vulkan EXT** (called `GetRayTMax` in GLSL/SPIR-V).

## Signature

```
float HitObject.GetRayTCurrent();
```

--------------------------------------------------------------------------------
<a id="get-world-ray-origin"></a>
# `HitObject.GetWorldRayOrigin`

## Description

Returns the ray origin in world space. Valid if the hit object represents a hit or a miss.

**Note**: **DXR 1.3 and Vulkan EXT**.

## Signature

```
float3 HitObject.GetWorldRayOrigin();
```

--------------------------------------------------------------------------------
<a id="get-world-ray-direction"></a>
# `HitObject.GetWorldRayDirection`

## Description

Returns the ray direction in world space. Valid if the hit object represents a hit or a miss.

**Note**: **DXR 1.3 and Vulkan EXT**.

## Signature

```
float3 HitObject.GetWorldRayDirection();
```

--------------------------------------------------------------------------------
<a id="get-shader-table-index"></a>
# `HitObject.GetShaderTableIndex`

## Description

Queries shader table index from HitObject. Valid if the hit object represents a hit or a miss.

## Signature 

```
uint HitObject.GetShaderTableIndex();
```

--------------------------------------------------------------------------------
<a id="get-instance-index"></a>
# `HitObject.GetInstanceIndex`

## Description

Returns the instance index of a hit. Valid if the hit object represents a hit.

## Signature 

```
uint HitObject.GetInstanceIndex();
```

--------------------------------------------------------------------------------
<a id="get-instance-id"></a>
# `HitObject.GetInstanceID`

## Description

Returns the instance ID of a hit. Valid if the hit object represents a hit.

## Signature 

```
uint HitObject.GetInstanceID();
```

--------------------------------------------------------------------------------
<a id="get-geometry-index"></a>
# `HitObject.GetGeometryIndex`

## Description

Returns the geometry index of a hit. Valid if the hit object represents a hit.

## Signature 

```
uint HitObject.GetGeometryIndex();
```

--------------------------------------------------------------------------------
<a id="get-primitive-index"></a>
# `HitObject.GetPrimitiveIndex`

## Description

Returns the primitive index of a hit. Valid if the hit object represents a hit.

## Signature 

```
uint HitObject.GetPrimitiveIndex();
```

--------------------------------------------------------------------------------
<a id="get-hit-kind"></a>
# `HitObject.GetHitKind`

## Description

Returns the hit kind. Valid if the hit object represents a hit.

## Signature 

```
uint HitObject.GetHitKind();
```

--------------------------------------------------------------------------------
<a id="get-attributes"></a>
# `HitObject.GetAttributes`

## Description

Returns the attributes of a hit. Valid if the hit object represents a hit or a miss.

## Signature

```
attr_t HitObject.GetAttributes<attr_t>();
```

--------------------------------------------------------------------------------
<a id="get-triangle-vertex-positions"></a>
# `HitObject.GetTriangleVertexPositions`

## Description

Returns the world-space vertex positions of the triangle that was hit. Valid if the hit object represents a triangle hit.

**Note**: **Vulkan EXT only**. Requires `SPV_KHR_ray_tracing_position_fetch` capability.

## Signature

```
void HitObject.GetTriangleVertexPositions(out float3 positions[3]);
```

--------------------------------------------------------------------------------
<a id="load-local-root-table-constant"></a>
# `HitObject.LoadLocalRootTableConstant`

## Description

Loads a root constant from the local root table referenced by the hit object. Valid if the hit object
represents a hit or a miss. RootConstantOffsetInBytes must be a multiple of 4.

**Note**: **D3D12/HLSL only**.

## Signature 

```
uint HitObject.LoadLocalRootTableConstant(uint RootConstantOffsetInBytes);
```

--------------------------------------------------------------------------------
<a id="set-shader-table-index"></a>
# `HitObject.SetShaderTableIndex`

## Description

Sets the shader table index of the hit object. Used to modify which shader gets invoked during HitObject.Invoke. **EXT extension only** (not available with NV extension).

## Signature

```
void HitObject.SetShaderTableIndex(uint RecordIndex);
```

--------------------------------------------------------------------------------
<a id="get-world-to-object"></a>
# `HitObject.GetWorldToObject`

## Description

Returns the world-to-object transformation matrix. Valid if the hit object represents a hit.

## Signature

```
float4x3 HitObject.GetWorldToObject();

// DXR 1.3 layout variants
float3x4 HitObject.GetWorldToObject3x4();
float4x3 HitObject.GetWorldToObject4x3();
```

--------------------------------------------------------------------------------
<a id="get-object-to-world"></a>
# `HitObject.GetObjectToWorld`

## Description

Returns the object-to-world transformation matrix. Valid if the hit object represents a hit.

## Signature

```
float4x3 HitObject.GetObjectToWorld();

// DXR 1.3 layout variants
float3x4 HitObject.GetObjectToWorld3x4();
float4x3 HitObject.GetObjectToWorld4x3();
```

--------------------------------------------------------------------------------
<a id="get-current-time"></a>
# `HitObject.GetCurrentTime`

## Description

Returns the current time for motion blur. Valid if the hit object represents a motion hit or miss.

**Note**: Requires motion blur support. Available on Vulkan (NV/EXT).

## Signature 

```
float HitObject.GetCurrentTime();
```

--------------------------------------------------------------------------------
<a id="get-object-ray-origin"></a>
# `HitObject.GetObjectRayOrigin`

## Description

Returns the ray origin in object space. Valid if the hit object represents a hit.

## Signature 

```
float3 HitObject.GetObjectRayOrigin();
```

--------------------------------------------------------------------------------
<a id="get-object-ray-direction"></a>
# `HitObject.GetObjectRayDirection`

## Description

Returns the ray direction in object space. Valid if the hit object represents a hit.

## Signature 

```
float3 HitObject.GetObjectRayDirection();
```

--------------------------------------------------------------------------------
<a id="get-shader-record-buffer-handle"></a>
# `HitObject.GetShaderRecordBufferHandle`

## Description

Returns the shader record buffer handle. Valid if the hit object represents a hit or a miss.

## Signature 

```
uint2 HitObject.GetShaderRecordBufferHandle();
```

--------------------------------------------------------------------------------
<a id="get-cluster-id"></a>
# `HitObject.GetClusterID`

## Description

Returns the cluster ID for cluster acceleration structures. Valid if the hit object represents a hit.

**Note**: **NV-only** (requires `GL_NV_cluster_acceleration_structure`).

## Signature 

```
int HitObject.GetClusterID();
```

--------------------------------------------------------------------------------
<a id="get-sphere-position-and-radius"></a>
# `HitObject.GetSpherePositionAndRadius`

## Description

Returns the position and radius of a sphere primitive. Valid if the hit object represents a sphere hit.

**Note**: **NV-only**.

## Signature 

```
float4 HitObject.GetSpherePositionAndRadius();
```

--------------------------------------------------------------------------------
<a id="get-lss-positions-and-radii"></a>
# `HitObject.GetLssPositionsAndRadii`

## Description

Returns the positions and radii of a linear swept sphere primitive. Valid if the hit object represents an LSS hit.

**Note**: **NV-only**.

## Signature 

```
float2x4 HitObject.GetLssPositionsAndRadii();
```

--------------------------------------------------------------------------------
<a id="is-sphere-hit"></a>
# `HitObject.IsSphereHit`

## Description

Returns true if the HitObject represents a hit on a sphere primitive, otherwise returns false.

**Note**: **NV-only**.

## Signature 

```
bool HitObject.IsSphereHit();
```

--------------------------------------------------------------------------------
<a id="is-lss-hit"></a>
# `HitObject.IsLssHit`

## Description

Returns true if the HitObject represents a hit on a linear swept sphere primitive, otherwise returns false.

**Note**: **NV-only**.

## Signature 

```
bool HitObject.IsLssHit();
```

--------------------------------------------------------------------------------
<a id="reorder-thread"></a>
# `ReorderThread`

## Description

Reorders threads based on a coherence hint value. NumCoherenceHintBits indicates how many of
the least significant bits of CoherenceHint should be considered during reordering (max: 16).
Applications should set this to the lowest value required to represent all possible values in
CoherenceHint. For best performance, all threads should provide the same value for
NumCoherenceHintBits.
Where possible, reordering will also attempt to retain locality in the thread’s launch indices
(DispatchRaysIndex in DXR).

`ReorderThread(HitOrMiss)` is equivalent to

```
void ReorderThread( HitObject HitOrMiss, uint CoherenceHint, uint NumCoherenceHintBitsFromLSB );
```

With CoherenceHint and NumCoherenceHintBitsFromLSB as 0, meaning they are ignored.

## Signature 

```
void ReorderThread(
    uint                 CoherenceHint,
    uint                 NumCoherenceHintBitsFromLSB);
void ReorderThread(
    HitObject            HitOrMiss,
    uint                 CoherenceHint,
    uint                 NumCoherenceHintBitsFromLSB);
void ReorderThread(HitObject HitOrMiss);
```

--------------------------------------------------------------------------------
<a id="reorder-execute"></a>
# `ReorderExecute`

## Description

Fused operation that reorders threads by HitObject and then executes the shader. Equivalent to calling `ReorderThread` followed by `HitObject.Invoke`.

**Note**: **Vulkan EXT only**. Available via `hitObjectReorderExecuteEXT` in GLSL.

## Signature

```
// GLSL: hitObjectReorderExecuteEXT(hitObject, payload)
void ReorderExecute<payload_t>(
    HitObject            HitOrMiss,
    inout payload_t      Payload);

// GLSL: hitObjectReorderExecuteEXT(hitObject, hint, bits, payload)
void ReorderExecute<payload_t>(
    HitObject            HitOrMiss,
    uint                 CoherenceHint,
    uint                 NumCoherenceHintBitsFromLSB,
    inout payload_t      Payload);
```

--------------------------------------------------------------------------------
<a id="trace-reorder-execute"></a>
# `TraceReorderExecute`

## Description

Fused operation that traces a ray, reorders threads by the resulting HitObject, and executes the shader. Equivalent to calling `HitObject.TraceRay`, `ReorderThread`, and `HitObject.Invoke` in sequence.

**Note**: **Vulkan EXT only**. Available via `hitObjectTraceReorderExecuteEXT` in GLSL.

## Signature

```
// GLSL: hitObjectTraceReorderExecuteEXT(...)
void TraceReorderExecute<payload_t>(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                 RayFlags,
    uint                 InstanceInclusionMask,
    uint                 RayContributionToHitGroupIndex,
    uint                 MultiplierForGeometryContributionToHitGroupIndex,
    uint                 MissShaderIndex,
    RayDesc              Ray,
    inout payload_t      Payload);

// With coherence hint
void TraceReorderExecute<payload_t>(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                 RayFlags,
    uint                 InstanceInclusionMask,
    uint                 RayContributionToHitGroupIndex,
    uint                 MultiplierForGeometryContributionToHitGroupIndex,
    uint                 MissShaderIndex,
    RayDesc              Ray,
    uint                 CoherenceHint,
    uint                 NumCoherenceHintBitsFromLSB,
    inout payload_t      Payload);
```

--------------------------------------------------------------------------------
<a id="trace-motion-reorder-execute"></a>
# `TraceMotionReorderExecute`

## Description

Fused operation for motion blur that traces a motion ray, reorders threads, and executes the shader.

**Note**: **Vulkan EXT only**. Available via `hitObjectTraceMotionReorderExecuteEXT` in GLSL. Requires motion blur support.

## Signature

```
void TraceMotionReorderExecute<payload_t>(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                 RayFlags,
    uint                 InstanceInclusionMask,
    uint                 RayContributionToHitGroupIndex,
    uint                 MultiplierForGeometryContributionToHitGroupIndex,
    uint                 MissShaderIndex,
    RayDesc              Ray,
    float                CurrentTime,
    uint                 CoherenceHint,
    uint                 NumCoherenceHintBitsFromLSB,
    inout payload_t      Payload);
```
