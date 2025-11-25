# Shader Execution Reordering (SER) - HitObject API Comparison

This document compares HitObject methods across different APIs: DXR (DirectX Raytracing), NVAPI, SPIRV, and CUDA/OptiX.

## Legend

| Symbol | Meaning |
|--------|---------|
| Y | Supported |
| N | Not supported |
| P | Partial (different signature) |

---

## Static Methods

| API | Signature | DXR | NVAPI | SPIRV | CUDA |
|-----|-----------|-----|-------|-------|------|
| `TraceRay` | `static HitObject TraceRay<payload_t>(RaytracingAccelerationStructure AccelerationStructure, uint RayFlags, uint InstanceInclusionMask, uint RayContributionToHitGroupIndex, uint MultiplierForGeometryContributionToHitGroupIndex, uint MissShaderIndex, RayDesc Ray, inout payload_t Payload)` | Y | Y | Y | Y |
| `TraceMotionRay` | `static HitObject TraceMotionRay<payload_t>(RaytracingAccelerationStructure AccelerationStructure, uint RayFlags, uint InstanceInclusionMask, uint RayContributionToHitGroupIndex, uint MultiplierForGeometryContributionToHitGroupIndex, uint MissShaderIndex, RayDesc Ray, float CurrentTime, inout payload_t Payload)` | N | Y | Y | Y |
| `FromRayQuery` | `static HitObject FromRayQuery(RayQuery Query)` | Y | N | N | N |
| `FromRayQuery` | `static HitObject FromRayQuery<attr_t>(RayQuery Query, uint CommittedCustomHitKind, attr_t CommittedCustomAttribs)` | Y | N | N | N |
| `MakeHit` (computed index) | `static HitObject MakeHit<attr_t>(RaytracingAccelerationStructure AccelerationStructure, uint InstanceIndex, uint GeometryIndex, uint PrimitiveIndex, uint HitKind, uint RayContributionToHitGroupIndex, uint MultiplierForGeometryContributionToHitGroupIndex, RayDesc Ray, attr_t attributes)` | N | Y | Y | Y |
| `MakeHit` (explicit index) | `static HitObject MakeHit<attr_t>(uint HitGroupRecordIndex, RaytracingAccelerationStructure AccelerationStructure, uint InstanceIndex, uint GeometryIndex, uint PrimitiveIndex, uint HitKind, RayDesc Ray, attr_t attributes)` | N | Y | Y | Y |
| `MakeMotionHit` (computed index) | `static HitObject MakeMotionHit<attr_t>(RaytracingAccelerationStructure AccelerationStructure, uint InstanceIndex, uint GeometryIndex, uint PrimitiveIndex, uint HitKind, uint RayContributionToHitGroupIndex, uint MultiplierForGeometryContributionToHitGroupIndex, RayDesc Ray, float CurrentTime, attr_t attributes)` | N | Y | Y | Y |
| `MakeMotionHit` (explicit index) | `static HitObject MakeMotionHit<attr_t>(uint HitGroupRecordIndex, RaytracingAccelerationStructure AccelerationStructure, uint InstanceIndex, uint GeometryIndex, uint PrimitiveIndex, uint HitKind, RayDesc Ray, float CurrentTime, attr_t attributes)` | N | N | Y | Y |
| `MakeMiss` (DXR) | `static HitObject MakeMiss(uint RayFlags, uint MissShaderIndex, RayDesc Ray)` | Y | N | N | N |
| `MakeMiss` (NVAPI) | `static HitObject MakeMiss(uint MissShaderIndex, RayDesc Ray)` | N | Y | Y | Y |
| `MakeMotionMiss` | `static HitObject MakeMotionMiss(uint MissShaderIndex, RayDesc Ray, float CurrentTime)` | N | Y | Y | Y |
| `MakeNop` | `static HitObject MakeNop()` | Y | Y | Y | Y |
| `Invoke` (DXR) | `static void Invoke<payload_t>(HitObject HitOrMiss, inout payload_t Payload)` | Y | N | N | N |
| `Invoke` (NVAPI) | `static void Invoke<payload_t>(RaytracingAccelerationStructure AccelerationStructure, HitObject HitOrMiss, inout payload_t Payload)` | N | Y | Y | Y |

---

## Instance Methods - Query State

| API | Signature | DXR | NVAPI | SPIRV | CUDA |
|-----|-----------|-----|-------|-------|------|
| `IsMiss` | `bool IsMiss()` | Y | Y | Y | Y |
| `IsHit` | `bool IsHit()` | Y | Y | Y | Y |
| `IsNop` | `bool IsNop()` | Y | Y | Y | Y |

---

## Instance Methods - Ray Properties

| API | Signature | DXR | NVAPI | SPIRV | CUDA |
|-----|-----------|-----|-------|-------|------|
| `GetRayFlags` | `uint GetRayFlags()` | Y | N | N | N |
| `GetRayTMin` | `float GetRayTMin()` | Y | N | N | N |
| `GetRayTCurrent` | `float GetRayTCurrent()` | Y | N | N | N |
| `GetWorldRayOrigin` | `float3 GetWorldRayOrigin()` | Y | N | N | N |
| `GetWorldRayDirection` | `float3 GetWorldRayDirection()` | Y | N | N | N |
| `GetRayDesc` | `RayDesc GetRayDesc()` | N | Y | Y | Y |
| `GetObjectRayOrigin` | `float3 GetObjectRayOrigin()` | Y | N | Y | Y |
| `GetObjectRayDirection` | `float3 GetObjectRayDirection()` | Y | N | Y | Y |
| `GetCurrentTime` | `float GetCurrentTime()` | N | N | Y | N |

---

## Instance Methods - Transform Matrices

| API | Signature | DXR | NVAPI | SPIRV | CUDA |
|-----|-----------|-----|-------|-------|------|
| `GetObjectToWorld3x4` | `float3x4 GetObjectToWorld3x4()` | Y | N | N | N |
| `GetObjectToWorld4x3` | `float4x3 GetObjectToWorld4x3()` | Y | N | N | N |
| `GetWorldToObject3x4` | `float3x4 GetWorldToObject3x4()` | Y | N | N | N |
| `GetWorldToObject4x3` | `float4x3 GetWorldToObject4x3()` | Y | N | N | N |
| `GetObjectToWorld` | `float4x3 GetObjectToWorld()` | N | Y | Y | N |
| `GetWorldToObject` | `float4x3 GetWorldToObject()` | N | Y | Y | N |

---

## Instance Methods - Hit Information

| API | Signature | DXR | NVAPI | SPIRV | CUDA |
|-----|-----------|-----|-------|-------|------|
| `GetInstanceIndex` | `uint GetInstanceIndex()` | Y | Y | Y | Y |
| `GetInstanceID` | `uint GetInstanceID()` | Y | Y | Y | Y |
| `GetGeometryIndex` | `uint GetGeometryIndex()` | Y | Y | Y | Y |
| `GetPrimitiveIndex` | `uint GetPrimitiveIndex()` | Y | Y | Y | Y |
| `GetHitKind` | `uint GetHitKind()` | Y | Y | Y | Y |
| `GetAttributes` | `attr_t GetAttributes<attr_t>()` | Y | Y | Y | Y |
| `GetClusterID` | `int GetClusterID()` | N | Y | Y | Y |

---

## Instance Methods - Shader Table

| API | Signature | DXR | NVAPI | SPIRV | CUDA |
|-----|-----------|-----|-------|-------|------|
| `GetShaderTableIndex` | `uint GetShaderTableIndex()` | Y | Y | Y | Y |
| `SetShaderTableIndex` | `uint SetShaderTableIndex(uint RecordIndex)` | Y | Y | N | Y |
| `LoadLocalRootTableConstant` | `uint LoadLocalRootTableConstant(uint RootConstantOffsetInBytes)` | Y | Y | N | Y |
| `GetShaderRecordBufferHandle` | `uint2 GetShaderRecordBufferHandle()` | N | N | Y | N |

---

## Instance Methods - LSS/Sphere (Linear Swept Spheres)

| API | Signature | DXR | NVAPI | SPIRV | CUDA |
|-----|-----------|-----|-------|-------|------|
| `GetSpherePositionAndRadius` | `float4 GetSpherePositionAndRadius()` | N | Y | Y | Y |
| `GetLssPositionsAndRadii` | `float2x4 GetLssPositionsAndRadii()` | N | Y | Y | Y |
| `IsSphereHit` | `bool IsSphereHit()` | N | Y | Y | Y |
| `IsLssHit` | `bool IsLssHit()` | N | Y | Y | Y |

---

## Standalone Functions (MaybeReorderThread / ReorderThread)

| API | Signature | DXR | NVAPI | SPIRV | CUDA |
|-----|-----------|-----|-------|-------|------|
| `MaybeReorderThread` | `void MaybeReorderThread(HitObject Hit)` | Y | Y | Y | Y |
| `MaybeReorderThread` | `void MaybeReorderThread(uint CoherenceHint, uint NumCoherenceHintBitsFromLSB)` | Y | Y | Y | Y |
| `MaybeReorderThread` | `void MaybeReorderThread(HitObject HitOrMiss, uint CoherenceHint, uint NumCoherenceHintBitsFromLSB)` | Y | Y | Y | Y |

---

## Summary - DXR Methods Not in Current Slang Implementation

| Method | Signature | Notes |
|--------|-----------|-------|
| `FromRayQuery` | `static HitObject FromRayQuery(RayQuery Query)` | Construct HitObject from RayQuery committed hit |
| `FromRayQuery` | `static HitObject FromRayQuery<attr_t>(RayQuery Query, uint CommittedCustomHitKind, attr_t CommittedCustomAttribs)` | With custom attributes for procedural primitives |
| `MakeMiss` (DXR variant) | `static HitObject MakeMiss(uint RayFlags, uint MissShaderIndex, RayDesc Ray)` | DXR version includes RayFlags parameter |
| `Invoke` (DXR variant) | `static void Invoke<payload_t>(HitObject HitOrMiss, inout payload_t Payload)` | DXR version omits AccelerationStructure parameter |
| `GetRayFlags` | `uint GetRayFlags()` | Returns ray flags associated with hit object |
| `GetRayTMin` | `float GetRayTMin()` | Returns parametric starting point |
| `GetRayTCurrent` | `float GetRayTCurrent()` | Returns parametric ending point (T value) |
| `GetWorldRayOrigin` | `float3 GetWorldRayOrigin()` | Returns world-space ray origin |
| `GetWorldRayDirection` | `float3 GetWorldRayDirection()` | Returns world-space ray direction |
| `GetObjectToWorld3x4` | `float3x4 GetObjectToWorld3x4()` | Object-to-world matrix (3x4) |
| `GetObjectToWorld4x3` | `float4x3 GetObjectToWorld4x3()` | Object-to-world matrix (4x3) - transposed |
| `GetWorldToObject3x4` | `float3x4 GetWorldToObject3x4()` | World-to-object matrix (3x4) |
| `GetWorldToObject4x3` | `float4x3 GetWorldToObject4x3()` | World-to-object matrix (4x3) - transposed |

---

## Summary - NVAPI/Extension Methods Not in DXR

| Method | Signature | Notes |
|--------|-----------|-------|
| `TraceMotionRay` | `static HitObject TraceMotionRay<payload_t>(...)` | Motion blur support |
| `MakeHit` | `static HitObject MakeHit<attr_t>(...)` | Create hit without tracing (2 overloads) |
| `MakeMotionHit` | `static HitObject MakeMotionHit<attr_t>(...)` | Create motion hit without tracing (2 overloads) |
| `MakeMotionMiss` | `static HitObject MakeMotionMiss(...)` | Create motion miss without tracing |
| `GetRayDesc` | `RayDesc GetRayDesc()` | Returns combined ray description |
| `GetCurrentTime` | `float GetCurrentTime()` | Motion blur time parameter |
| `GetClusterID` | `int GetClusterID()` | Cluster acceleration structure support |
| `GetSpherePositionAndRadius` | `float4 GetSpherePositionAndRadius()` | LSS geometry support |
| `GetLssPositionsAndRadii` | `float2x4 GetLssPositionsAndRadii()` | LSS geometry support |
| `IsSphereHit` | `bool IsSphereHit()` | LSS geometry support |
| `IsLssHit` | `bool IsLssHit()` | LSS geometry support |
| `GetShaderRecordBufferHandle` | `uint2 GetShaderRecordBufferHandle()` | Vulkan/SPIRV specific |

---

## Implementation Notes

### Signature Differences

1. **MakeMiss**: DXR includes `RayFlags` parameter, NVAPI omits it
2. **Invoke**: DXR omits `AccelerationStructure` parameter, NVAPI requires it
3. **Matrix accessors**: DXR provides both 3x4 and 4x3 variants explicitly named; NVAPI provides only 4x3

### DXR-Specific Features

- `FromRayQuery` - enables constructing HitObject from inline raytracing
- Individual ray component accessors (`GetRayTMin`, `GetRayTCurrent`, `GetWorldRayOrigin`, `GetWorldRayDirection`, `GetRayFlags`)

### NVAPI/Extension-Specific Features

- Motion blur support (`TraceMotionRay`, `MakeMotionHit`, `MakeMotionMiss`, `GetCurrentTime`)
- Hit construction without tracing (`MakeHit`)
- Cluster acceleration structure (`GetClusterID`)
- Linear Swept Spheres geometry (`GetSpherePositionAndRadius`, `GetLssPositionsAndRadii`, `IsSphereHit`, `IsLssHit`)
- Combined ray descriptor (`GetRayDesc`)

---

# Implementation Strategy: Dual-API Support (NVAPI + DXR 1.3)

This section describes how to implement HitObject with support for both the existing NVAPI backend and the new DXR 1.3 native backend without regression.

## 1. Capability System Design

### Current Capability Structure

```
// Current SER capability (NVAPI-based for HLSL)
alias ser = raytracing + GL_NV_shader_invocation_reorder | raytracing + hlsl_nvapi | cuda;
```

### Proposed New Capabilities

Add to `slang-capabilities.capdef`:

```capdef
/// DXR 1.3 native SER support (SM 6.9, no NVAPI required)
/// [EXT]
def _ser_hlsl_native : _sm_6_9;

/// Combined SER capability: NVAPI OR native DXR 1.3
/// [Compound]
alias ser_hlsl = hlsl_nvapi | _ser_hlsl_native;

/// Updated SER capability with DXR 1.3 support
/// [Compound]
alias ser = raytracing + GL_NV_shader_invocation_reorder
          | raytracing + ser_hlsl
          | cuda;

/// DXR 1.3 specific capability (native HLSL only, no NVAPI)
/// [Compound]
alias ser_dxr_1_3 = _ser_hlsl_native + raygen_closesthit_miss;

/// Stage-combined capabilities
alias ser_dxr_raygen_closesthit_miss = raygen_closesthit_miss + _ser_hlsl_native;
```

## 2. Method Implementation Strategy

### Key Pattern: Capability Atoms as Case Labels

Slang supports using **capability atoms as case labels** in `__target_switch`. This is the correct pattern for providing different implementations for the same target based on capabilities.

**Example from CooperativeVector (hlsl.meta.slang:25983-26001):**
```slang
__target_switch
{
case hlsl:                    // Native SM 6.9 implementation
    __intrinsic_asm "$0 = $1";
case hlsl_coopvec_poc:        // POC DXC implementation
    __intrinsic_asm ".CopyFrom";
case optix_coopvec:           // OptiX implementation
    __intrinsic_asm "optixCoopVecCvt<...>";
default:
    // Fallback
}
```

### 2.1 Methods with Identical Signatures

These methods exist in both APIs with identical signatures:
- `TraceRay<payload_t>` - same signature
- `MakeNop()` - same signature
- `IsMiss()`, `IsHit()`, `IsNop()` - same signature
- `GetInstanceIndex()`, `GetInstanceID()`, `GetGeometryIndex()`, `GetPrimitiveIndex()`, `GetHitKind()` - same
- `GetShaderTableIndex()`, `SetShaderTableIndex()`, `LoadLocalRootTableConstant()` - same
- `GetAttributes<attr_t>()` - same

**Implementation**: Use capability atoms as separate case labels:

```slang
// Remove [__requiresNVAPI] from function to support both paths
[ForceInline]
[require(cuda_glsl_hlsl_spirv, ser_raygen_closesthit_miss)]
static HitObject MakeNop()
{
    __target_switch
    {
    case hlsl:
        // Native DXR 1.3 (SM 6.9) - selected when _ser_hlsl_native is available
        __intrinsic_asm "($0 = dx::HitObject::MakeNop())";
    case hlsl_nvapi:
        // NVAPI path - selected when hlsl_nvapi is available
        __intrinsic_asm "($0 = NvMakeNop())";
    case glsl:
        __glslMakeNop(__return_val);
    case cuda:
        __intrinsic_asm "slangOptixMakeNopHitObject";
    case spirv:
        // ... existing spirv_asm
    }
}
```

**Note**: The compiler selects the most specific matching case. When both `hlsl` and `hlsl_nvapi` are available, `hlsl_nvapi` is more specific and will be selected. When only `hlsl + _sm_6_9` is available (without NVAPI), `case hlsl:` will be selected.

### 2.2 Methods with Different Signatures

#### `MakeMiss` - Signature Difference

| API | Signature |
|-----|-----------|
| DXR | `MakeMiss(uint RayFlags, uint MissShaderIndex, RayDesc Ray)` |
| NVAPI | `MakeMiss(uint MissShaderIndex, RayDesc Ray)` |

**Strategy**: These are different overloads (different parameter counts), so they can coexist:

```slang
// DXR 1.3 signature (3 params with RayFlags) - NEW
[ForceInline]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
static HitObject MakeMiss(
    uint RayFlags,
    uint MissShaderIndex,
    RayDesc Ray)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "($3 = dx::HitObject::MakeMiss($0, $1, $2))";
    }
}

// NVAPI/SPIRV/CUDA signature (2 params, no RayFlags) - EXISTING, updated
[ForceInline]
[require(cuda_glsl_hlsl_spirv, ser_raygen_closesthit_miss)]
static HitObject MakeMiss(
    uint MissShaderIndex,
    RayDesc Ray)
{
    __target_switch
    {
    case hlsl_nvapi:
        __intrinsic_asm "($2=NvMakeMiss($0,$1))";
    case glsl:
        __glslMakeMiss(__return_val, MissShaderIndex, Ray.Origin, Ray.TMin, Ray.Direction, Ray.TMax);
    case cuda:
        __intrinsic_asm "optixMakeMissHitObject";
    case spirv:
        // ... existing spirv_asm
    }
}
```

#### `Invoke` - Signature Difference

| API | Signature |
|-----|-----------|
| DXR | `Invoke<payload_t>(HitObject HitOrMiss, inout payload_t Payload)` |
| NVAPI | `Invoke<payload_t>(RaytracingAccelerationStructure, HitObject, inout payload_t)` |

**Strategy**: These are different overloads (different parameter counts), so they can coexist:

```slang
// DXR 1.3 signature (no AccelerationStructure) - NEW
[ForceInline]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
static void Invoke<payload_t>(
    HitObject HitOrMiss,
    inout payload_t Payload)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "dx::HitObject::Invoke($0, $1)";
    }
}

// NVAPI/SPIRV/CUDA signature (with AccelerationStructure) - EXISTING, updated
[ForceInline]
[require(cuda_glsl_hlsl_spirv, ser_raygen_closesthit_miss)]
static void Invoke<payload_t>(
    RaytracingAccelerationStructure AccelerationStructure,
    HitObject HitOrMiss,
    inout payload_t Payload)
{
    __target_switch
    {
    case hlsl_nvapi:
        __InvokeHLSL(AccelerationStructure, HitOrMiss,
            __forceVarIntoRayPayloadStructTemporarily(Payload));
    case glsl:
        // ... existing glsl implementation
    case cuda:
        __intrinsic_asm "optixInvoke";
    case spirv:
        // ... existing spirv_asm
    }
}
```

### 2.3 DXR 1.3-Only Methods (New)

These methods are DXR 1.3 specific and need to be added:

```slang
// FromRayQuery - DXR 1.3 only
[ForceInline]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
static HitObject FromRayQuery(RayQuery Query)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "($1 = dx::HitObject::FromRayQuery($0))";
    }
}

// FromRayQuery with attributes - DXR 1.3 only
[ForceInline]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
static HitObject FromRayQuery<attr_t>(
    RayQuery Query,
    uint CommittedCustomHitKind,
    attr_t CommittedCustomAttribs)
{
    __target_switch
    {
    case hlsl:
        __intrinsic_asm "($3 = dx::HitObject::FromRayQuery($0, $1, $2))";
    }
}

// Individual ray component accessors - DXR 1.3 only
[ForceInline]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
uint GetRayFlags()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm ".GetRayFlags";
    }
}

[ForceInline]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
float GetRayTMin()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm ".GetRayTMin";
    }
}

[ForceInline]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
float GetRayTCurrent()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm ".GetRayTCurrent";
    }
}

[ForceInline]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
float3 GetWorldRayOrigin()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm ".GetWorldRayOrigin";
    }
}

[ForceInline]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
float3 GetWorldRayDirection()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm ".GetWorldRayDirection";
    }
}

// Matrix accessors with explicit dimensions - DXR 1.3 only
[ForceInline]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
float3x4 GetObjectToWorld3x4()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm ".GetObjectToWorld3x4";
    }
}

[ForceInline]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
float4x3 GetObjectToWorld4x3()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm ".GetObjectToWorld4x3";
    }
}

[ForceInline]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
float3x4 GetWorldToObject3x4()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm ".GetWorldToObject3x4";
    }
}

[ForceInline]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
float4x3 GetWorldToObject4x3()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm ".GetWorldToObject4x3";
    }
}
```

### 2.4 MaybeReorderThread Functions (DXR 1.3 Naming)

DXR 1.3 uses `MaybeReorderThread` (existing Slang uses `ReorderThread`). Add aliases:

```slang
// DXR 1.3 naming - dx::MaybeReorderThread
[ForceInline]
[require(hlsl, ser_dxr_raygen)]
void MaybeReorderThread(HitObject Hit)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "dx::MaybeReorderThread($0)";
    }
}

[ForceInline]
[require(hlsl, ser_dxr_raygen)]
void MaybeReorderThread(uint CoherenceHint, uint NumCoherenceHintBitsFromLSB)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "dx::MaybeReorderThread($0, $1)";
    }
}

[ForceInline]
[require(hlsl, ser_dxr_raygen)]
void MaybeReorderThread(HitObject HitOrMiss, uint CoherenceHint, uint NumCoherenceHintBitsFromLSB)
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "dx::MaybeReorderThread($0, $1, $2)";
    }
}
```

## 3. HLSL Output Differences

### NVAPI Output (Current)
```hlsl
#include "nvHLSLExtns.h"
HitObject hit = NvMakeNop();
NvReorderThread(hit);
NvInvokeHitObject(accelStruct, hit, payload);
```

### DXR 1.3 Native Output (New)
```hlsl
dx::HitObject hit = dx::HitObject::MakeNop();
dx::MaybeReorderThread(hit);
dx::HitObject::Invoke(hit, payload);
```

## 4. Implementation Checklist

### Phase 1: Capability System
- [ ] Add `_ser_hlsl_native` capability for SM 6.9 native SER
- [ ] Add `ser_hlsl` alias combining NVAPI and native
- [ ] Add `ser_dxr_1_3` and stage-combined capabilities
- [ ] Update `ser` alias to include native path

### Phase 2: Common Methods (Both APIs)
- [ ] Update `MakeNop()` with dual HLSL paths
- [ ] Update `TraceRay()` with dual HLSL paths
- [ ] Update `IsMiss()`, `IsHit()`, `IsNop()` with dual paths
- [ ] Update `GetInstanceIndex()`, `GetInstanceID()`, etc.
- [ ] Update `GetShaderTableIndex()`, `SetShaderTableIndex()`
- [ ] Update `GetAttributes()`

### Phase 3: DXR 1.3-Only Methods
- [ ] Add `FromRayQuery()` (both overloads)
- [ ] Add `GetRayFlags()`
- [ ] Add `GetRayTMin()`, `GetRayTCurrent()`
- [ ] Add `GetWorldRayOrigin()`, `GetWorldRayDirection()`
- [ ] Add `GetObjectToWorld3x4()`, `GetObjectToWorld4x3()`
- [ ] Add `GetWorldToObject3x4()`, `GetWorldToObject4x3()`
- [ ] Update `GetObjectRayOrigin()`, `GetObjectRayDirection()` for HLSL

### Phase 4: Signature Variants
- [ ] Add DXR `MakeMiss(RayFlags, MissShaderIndex, Ray)` overload
- [ ] Add DXR `Invoke(HitObject, Payload)` overload (no AccelStruct)

### Phase 5: MaybeReorderThread
- [ ] Add `MaybeReorderThread(HitObject)` for DXR 1.3
- [ ] Add `MaybeReorderThread(CoherenceHint, NumBits)` for DXR 1.3
- [ ] Add `MaybeReorderThread(HitObject, CoherenceHint, NumBits)` for DXR 1.3

### Phase 6: Testing
- [ ] Test NVAPI path (ensure no regression)
- [ ] Test GLSL/SPIRV path (ensure no regression)
- [ ] Test CUDA path (ensure no regression)
- [ ] Test DXR 1.3 native path
- [ ] Add test files under `tests/hlsl-intrinsic/shader-execution-reordering/`

## 5. Capability Selection at Compile Time

The compiler selects the **most specific matching case** in `__target_switch`. This is handled by `isBetterForTarget` in `slang-ir-specialize-target-switch.cpp`.

### Selection Rules:

1. **More specific capability wins**: `case hlsl_nvapi:` is more specific than `case hlsl:` because `hlsl_nvapi` inherits from `hlsl`
2. **When NVAPI is available**: `case hlsl_nvapi:` is selected over `case hlsl:`
3. **When only SM 6.9 (no NVAPI)**: `case hlsl:` is selected

### Selection Flow:

```
User compiles with:         Case Selected:              Output:
-------------------         --------------              -------
-target hlsl -profile sm_6_9
  (no NVAPI)            --> case hlsl:              --> dx::HitObject::MakeNop()

-target hlsl -profile sm_6_5
  + NVAPI enabled       --> case hlsl_nvapi:        --> NvMakeNop()

-target hlsl -profile sm_6_9
  + NVAPI enabled       --> case hlsl_nvapi:        --> NvMakeNop()
                            (more specific wins)

-target glsl            --> case glsl:              --> hitObjectRecordEmptyNV()

-target spirv           --> case spirv:             --> OpHitObjectRecordEmptyNV

-target cuda            --> case cuda:              --> slangOptixMakeNopHitObject
```

### Important: Case Order Matters Less Than Specificity

The compiler doesn't just use the first matching case - it evaluates all cases and selects the **best match** based on capability specificity. This is why:
- `case hlsl_nvapi:` will be selected over `case hlsl:` when NVAPI is available
- `case hlsl:` serves as the fallback for non-NVAPI HLSL (SM 6.9 native)

## 6. Backward Compatibility

- Existing shaders using NVAPI SER will continue to work unchanged
- New shaders targeting SM 6.9 can use native DXR 1.3 syntax
- Slang abstracts the difference - same Slang code works on both paths
- Method overloads allow gradual adoption of DXR 1.3 specific features

---

## 7. Concrete Example: Updating `IsMiss()`

### Current Implementation (NVAPI-only for HLSL):

```slang
[__requiresNVAPI]
[ForceInline]
[require(cuda_glsl_hlsl_spirv, ser_raygen_closesthit_miss)]
bool IsMiss()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm ".IsMiss";
    case glsl: __intrinsic_asm "hitObjectIsMissNV($0)";
    case cuda: __intrinsic_asm "slangOptixHitObjectIsMiss";
    case spirv:
        return spirv_asm
        {
            OpExtension "SPV_NV_shader_invocation_reorder";
            OpCapability ShaderInvocationReorderNV;
            result:$$bool = OpHitObjectIsMissNV &this;
        };
    }
}
```

### Updated Implementation (NVAPI + DXR 1.3):

```slang
// Remove [__requiresNVAPI] to support both paths
[ForceInline]
[require(cuda_glsl_hlsl_spirv, ser_raygen_closesthit_miss)]
bool IsMiss()
{
    __target_switch
    {
    case hlsl:
        // DXR 1.3 native - same syntax, just different namespace expectation
        __intrinsic_asm ".IsMiss";
    case hlsl_nvapi:
        // NVAPI path - generates NvHitObject.IsMiss or similar
        __intrinsic_asm ".IsMiss";
    case glsl:
        __intrinsic_asm "hitObjectIsMissNV($0)";
    case cuda:
        __intrinsic_asm "slangOptixHitObjectIsMiss";
    case spirv:
        return spirv_asm
        {
            OpExtension "SPV_NV_shader_invocation_reorder";
            OpCapability ShaderInvocationReorderNV;
            result:$$bool = OpHitObjectIsMissNV &this;
        };
    }
}
```

**Note**: For `IsMiss()`, both NVAPI and DXR 1.3 use the same `.IsMiss` syntax - the difference is in how the HitObject type itself is declared (`dx::HitObject` vs NVAPI's type). The key change is removing `[__requiresNVAPI]` and adding `case hlsl_nvapi:` for the NVAPI-specific path when needed.

---

## 8. Migration Steps for Existing Methods

For each existing HitObject method in `hlsl.meta.slang`:

1. **Remove `[__requiresNVAPI]`** attribute from function (if present)
2. **Keep `[require(...)]`** with updated capability that includes both paths
3. **Add `case hlsl_nvapi:`** for NVAPI-specific HLSL code
4. **Add/update `case hlsl:`** for DXR 1.3 native code
5. **Keep other cases unchanged** (glsl, cuda, spirv)

### Methods Requiring Changes:

| Method | Change Type | Notes |
|--------|------------|-------|
| `MakeNop()` | Dual case | Same intrinsic, different type context |
| `TraceRay()` | Dual case | Complex, needs careful handling of payload |
| `IsMiss()` | Dual case | Same intrinsic syntax |
| `IsHit()` | Dual case | Same intrinsic syntax |
| `IsNop()` | Dual case | Same intrinsic syntax |
| `GetRayDesc()` | Keep NVAPI-only | DXR 1.3 uses individual accessors |
| `GetShaderTableIndex()` | Dual case | Same intrinsic syntax |
| `MakeMiss()` | New overload | DXR version has RayFlags param |
| `Invoke()` | New overload | DXR version omits AccelStruct |

### New Methods (DXR 1.3 Only):

| Method | Notes |
|--------|-------|
| `FromRayQuery()` | Two overloads |
| `GetRayFlags()` | New accessor |
| `GetRayTMin()` | New accessor |
| `GetRayTCurrent()` | New accessor |
| `GetWorldRayOrigin()` | New accessor |
| `GetWorldRayDirection()` | New accessor |
| `GetObjectToWorld3x4()` | Matrix variant |
| `GetObjectToWorld4x3()` | Matrix variant |
| `GetWorldToObject3x4()` | Matrix variant |
| `GetWorldToObject4x3()` | Matrix variant |

---

## 9. Practical Implementation Findings

This section documents the actual implementation experience and key discoveries.

### 9.1 Files Modified

| File | Changes |
|------|---------|
| `source/slang/slang-capabilities.capdef` | Added `ser_nvapi`, `_ser_hlsl_native`, `ser_dxr`, updated `ser` alias, added stage-combined capabilities |
| `source/slang/hlsl.meta.slang` | Updated HitObject methods with dual `case hlsl_nvapi:` / `case hlsl:` patterns, added DXR 1.3-only methods |
| `source/slang/slang-emit-hlsl.cpp` | Added capability-based HitObject type emission (`NvHitObject` vs `dx::HitObject`) |

### 9.2 Capability Definitions Added

```capdef
/// Capabilities needed for shader-execution-reordering (NVAPI path for HLSL)
alias ser_nvapi = raytracing + hlsl_nvapi;

/// DXR 1.3 native SER support (SM 6.9, no NVAPI required)
def _ser_hlsl_native : _sm_6_9;

/// Capabilities needed for shader-execution-reordering (native DXR 1.3 path)
alias ser_dxr = raytracing + _ser_hlsl_native;

/// Capabilities needed for shader-execution-reordering (all paths)
alias ser = raytracing + GL_NV_shader_invocation_reorder | ser_nvapi | ser_dxr | cuda;

/// Stage-combined capabilities for DXR 1.3
alias ser_dxr_raygen_closesthit_miss = raygen_closesthit_miss + ser_dxr;
alias ser_dxr_raygen = raygen + ser_dxr;
```

### 9.3 Critical Discovery: Capability vs Preprocessor Define

**`-DNV_SHADER_EXTN_SLOT=u0` is NOT the same as `-capability hlsl_nvapi`!**

| Flag | What it does | Affects target_switch? |
|------|-------------|----------------------|
| `-DNV_SHADER_EXTN_SLOT=u0` | Defines preprocessor macro, reserves register for NVAPI | **NO** |
| `-capability hlsl_nvapi` | Enables `hlsl_nvapi` capability for code generation | **YES** |

**Consequence**: Users must explicitly pass `-capability hlsl_nvapi` to get NVAPI code paths in `__target_switch`. The preprocessor define alone is insufficient.

### 9.4 Capability Inference Limitation

The compiler **infers** capabilities during semantic checking (you'll see warnings like):
```
warning 41012: entry point 'rayGenerationMain' uses additional capabilities that are not
part of the specified profile 'lib_6_5'. The profile setting is automatically updated to
include these capabilities: 'hlsl_nvapi + ser_hlsl_native'
```

**However**, these inferred capabilities are **NOT propagated** to `__target_switch` specialization. The target_switch uses the **original command-line capabilities** only.

This is a potential area for future improvement in the compiler.

### 9.5 HitObject Type Emission Fix

Added capability-based type emission in `slang-emit-hlsl.cpp`:

```cpp
case kIROp_HitObjectType:
{
    auto targetCaps = getTargetReq()->getTargetCaps();
    auto nvapiCapabilitySet = CapabilitySet(CapabilityName::hlsl_nvapi);
    auto sm69CapabilitySet = CapabilitySet(CapabilityName::_sm_6_9);

    if (targetCaps.implies(nvapiCapabilitySet))
    {
        // Explicit NVAPI: use NvHitObject
        m_writer->emit("NvHitObject");
    }
    else if (targetCaps.implies(sm69CapabilitySet))
    {
        // DXR 1.3 standard: use dx::HitObject namespace
        m_writer->emit("dx::HitObject");
    }
    else
    {
        // Fallback to legacy NVAPI
        m_writer->emit("NvHitObject");
    }
    return;
}
```

### 9.6 Target Switch Pattern for Dual-API Methods

For methods with different intrinsics between NVAPI and DXR 1.3:

```slang
[ForceInline]
[require(cuda_glsl_spirv, ser_raygen_closesthit_miss)]
[require(hlsl_nvapi, ser_raygen_closesthit_miss)]
[require(hlsl, ser_dxr_raygen_closesthit_miss)]
static HitObject MakeNop()
{
    __target_switch
    {
    case hlsl_nvapi:
        __intrinsic_asm "($0 = NvMakeNop())";
    case hlsl:
        // DXR 1.3 native path
        __intrinsic_asm "($0 = dx::HitObject::MakeNop())";
    case glsl:
        __glslMakeNop(__return_val);
    case cuda:
        __intrinsic_asm "slangOptixMakeNopHitObject";
    case spirv:
        // ... spirv_asm
    }
}
```

**Key points**:
1. Multiple `[require]` attributes for different capability paths
2. `case hlsl_nvapi:` for NVAPI-specific intrinsic
3. `case hlsl:` for DXR 1.3 native intrinsic
4. The compiler selects `hlsl_nvapi` when that capability is present (more specific)
5. Falls back to `hlsl` for SM 6.9 without NVAPI

### 9.7 Compilation Commands

```bash
# NVAPI path - requires explicit capability flag
./build/Debug/bin/slangc shader.slang -target hlsl -entry main -stage raygeneration \
  -capability hlsl_nvapi -DNV_SHADER_EXTN_SLOT=u0

# Output: NvHitObject, NvMakeNop(), NvReorderThread(), etc.

# DXR 1.3 path - SM 6.9 without NVAPI capability
./build/Debug/bin/slangc shader.slang -target hlsl -entry main -stage raygeneration \
  -profile sm_6_9

# Output: dx::HitObject, dx::HitObject::MakeNop(), dx::MaybeReorderThread(), etc.
```

### 9.8 Verification Results

| Path | Type Emitted | Intrinsic Emitted |
|------|--------------|-------------------|
| `-capability hlsl_nvapi` | `NvHitObject` ✓ | `NvMakeNop()` ✓ |
| `-profile sm_6_9` (no nvapi) | `dx::HitObject` ✓ | `dx::HitObject::MakeNop()` ✓ |
| SPIRV | N/A | `OpHitObjectRecordEmptyNV` ✓ |
| CUDA | N/A | `slangOptixMakeNopHitObject` ✓ |

### 9.9 Known Limitation: Capability Fall-Through

Earlier attempts to use case fall-through pattern failed:

```slang
// THIS DOES NOT WORK - causes capability conflict error
case hlsl_nvapi:
case hlsl:
    __intrinsic_asm ".IsMiss";
```

Error:
```
error 36120: the capability for case 'hlsl' conflicts with previous case...
In target_switch, if two cases belong to the same target, one capability set
has to be a subset of the other.
```

**Solution**: Use separate cases with separate intrinsics, or use single `case hlsl:` when the intrinsic syntax is identical for both paths (the type emission handles the namespace difference).

---

## 10. Testing Instructions

This section provides commands to verify the dual-API implementation for both DXR 1.3 and NVAPI paths.

### 10.1 Build

```bash
# Configure and build
cmake --preset default
cmake --build --preset debug --target slangc
```

### 10.2 Test Commands by Category

#### Static Methods

```bash
# DXR 1.3 Path - should emit dx::HitObject::TraceRay, dx::HitObject::MakeHit, etc.
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-static-methods-dxr.slang \
  -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration

# NVAPI Path - should emit NvTraceRayHitObject, NvMakeHit, etc.
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-static-methods-nvapi.slang \
  -target hlsl -capability hlsl_nvapi -entry rayGenerationMain -stage raygeneration
```

#### Query State (IsMiss, IsHit, IsNop)

```bash
# DXR 1.3 Path
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-query-state.slang \
  -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration

# NVAPI Path
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-query-state.slang \
  -target hlsl -capability hlsl_nvapi -entry rayGenerationMain -stage raygeneration
```

#### Ray Properties

```bash
# Common methods (GetObjectRayOrigin, GetObjectRayDirection)
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-ray-properties.slang \
  -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration

# DXR 1.3-only methods (GetRayFlags, GetRayTMin, GetRayTCurrent, GetWorldRayOrigin, GetWorldRayDirection)
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-ray-properties-dxr.slang \
  -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration

# NVAPI-only methods (GetRayDesc)
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-ray-properties-nvapi.slang \
  -target hlsl -capability hlsl_nvapi -entry rayGenerationMain -stage raygeneration
```

#### Transform Matrices

```bash
# DXR 1.3 (GetObjectToWorld3x4, GetObjectToWorld4x3, GetWorldToObject3x4, GetWorldToObject4x3)
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-transform-matrices-dxr.slang \
  -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration

# NVAPI (GetObjectToWorld, GetWorldToObject)
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-transform-matrices-nvapi.slang \
  -target hlsl -capability hlsl_nvapi -entry rayGenerationMain -stage raygeneration
```

#### Hit Information

```bash
# DXR 1.3 Path
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-hit-information.slang \
  -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration

# NVAPI Path
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-hit-information.slang \
  -target hlsl -capability hlsl_nvapi -entry rayGenerationMain -stage raygeneration
```

#### Shader Table

```bash
# DXR 1.3 Path
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-shader-table.slang \
  -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration

# NVAPI Path
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-shader-table.slang \
  -target hlsl -capability hlsl_nvapi -entry rayGenerationMain -stage raygeneration
```

#### Reordering (ReorderThread / MaybeReorderThread)

```bash
# ReorderThread with DXR 1.3 (emits dx::MaybeReorderThread)
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-reordering.slang \
  -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration

# ReorderThread with NVAPI (emits NvReorderThread)
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-reordering.slang \
  -target hlsl -capability hlsl_nvapi -entry rayGenerationMain -stage raygeneration

# MaybeReorderThread (DXR 1.3 naming)
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/ser-test-reordering-dxr.slang \
  -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration
```

#### FromRayQuery (DXR 1.3 only)

```bash
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/hit-object-from-rayquery.slang \
  -target hlsl -profile sm_6_9 -entry rayGenerationMain -stage raygeneration
```

### 10.3 Expected Output Differences

| Path | HitObject Type | Static Methods | Reorder Function |
|------|----------------|----------------|------------------|
| DXR 1.3 (`-profile sm_6_9`) | `dx::HitObject` | `dx::HitObject::*` | `dx::MaybeReorderThread` |
| NVAPI (`-capability hlsl_nvapi`) | `NvHitObject` | `Nv*` prefix | `NvReorderThread` |

### 10.4 Header Inclusion Check

- **DXR 1.3**: Should NOT have `#define SLANG_HLSL_ENABLE_NVAPI 1`
- **NVAPI**: Should have `#define SLANG_HLSL_ENABLE_NVAPI 1` and `#include "nvHLSLExtns.h"`
