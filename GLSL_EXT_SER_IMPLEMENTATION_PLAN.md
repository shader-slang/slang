# GLSL_EXT_shader_invocation_reorder Implementation Plan

## Overview
Implement support for GL_EXT_shader_invocation_reorder extension in Slang to complement the existing GL_NV_shader_invocation_reorder support.

## Current State
- **Existing**: GL_NV_shader_invocation_reorder (NVIDIA-specific)
  - GLSL functions: `hitObject*NV()`
  - SPIRV ops: `OpHitObject*NV`
  - Extension: `SPV_NV_shader_invocation_reorder`

- **New**: GL_EXT_shader_invocation_reorder (cross-vendor standard)
  - GLSL functions: `hitObject*EXT()`
  - SPIRV ops: `OpHitObject*EXT`
  - Extension: `SPV_EXT_shader_invocation_reorder` (TBD - check SPIRV registry)

## Key Differences: NV vs EXT

### Function Naming
| Slang Method | GLSL NV | GLSL EXT |
|-------------|---------|----------|
| `MakeNop()` | `hitObjectRecordEmptyNV` | `hitObjectRecordEmptyEXT` |
| `TraceRay()` | N/A | `hitObjectTraceRayEXT` |
| `TraceMotionRay()` | N/A | `hitObjectTraceRayMotionEXT` |
| `MakeMiss()` | `hitObjectRecordMissNV` | `hitObjectRecordMissEXT` |
| `Invoke()` | `hitObjectExecuteShaderNV` | `hitObjectExecuteShaderEXT` |
| `IsMiss()` | `hitObjectIsMissNV` | `hitObjectIsMissEXT` |
| `IsHit()` | `hitObjectIsHitNV` | `hitObjectIsHitEXT` |
| `IsNop()` | `hitObjectIsEmptyNV` | `hitObjectIsEmptyEXT` |
| `GetRayTMin()` | `hitObjectGetRayTMinNV` | `hitObjectGetRayTMinEXT` |
| `GetRayTMax()` | N/A | `hitObjectGetRayTMaxEXT` |
| `GetRayFlags()` | N/A | `hitObjectGetRayFlagsEXT` |
| `GetWorldRayOrigin()` | `hitObjectGetWorldRayOriginNV` | `hitObjectGetWorldRayOriginEXT` |
| `GetWorldRayDirection()` | `hitObjectGetWorldRayDirectionNV` | `hitObjectGetWorldRayDirectionEXT` |
| `GetObjectRayOrigin()` | N/A | `hitObjectGetObjectRayOriginEXT` |
| `GetObjectRayDirection()` | `hitObjectGetObjectRayDirectionNV` | `hitObjectGetObjectRayDirectionEXT` |
| `GetObjectToWorld()` | N/A | `hitObjectGetObjectToWorldEXT` |
| `GetWorldToObject()` | N/A | `hitObjectGetWorldToObjectEXT` |
| `GetInstanceIdEXT()` | N/A | `hitObjectGetInstanceIdEXT` |
| `GetInstanceCustomIndex()` | N/A | `hitObjectGetInstanceCustomIndexEXT` |
| `GetGeometryIndex()` | N/A | `hitObjectGetGeometryIndexEXT` |
| `GetPrimitiveIndex()` | N/A | `hitObjectGetPrimitiveIndexEXT` |
| `GetHitKind()` | N/A | `hitObjectGetHitKindEXT` |
| `GetCurrentTime()` | N/A | `hitObjectGetCurrentTimeEXT` |
| `GetShaderTableIndex()` | N/A | `hitObjectGetShaderBindingTableRecordIndexEXT` |
| `SetShaderTableIndex()` | N/A | `hitObjectSetShaderBindingTableRecordIndexEXT` |
| `GetShaderRecordBufferHandle()` | N/A | `hitObjectGetShaderRecordBufferHandleEXT` |
| `ReorderThread()` | `NvReorderThreadNV` | `reorderThreadEXT` |

### New EXT-Only Features
1. **FromRayQuery** - Populate hitObject from rayQuery
   - `hitObjectRecordFromQueryEXT(hitObjectEXT, rayQueryEXT, uint sbtRecordIndex, int attributeLocation)`

2. **Attribute Management with Locations**
   - `hitObjectGetAttributesEXT(hitObjectEXT, int attributeLocation)`
   - `hitObjectAttributeEXT` storage qualifier for attributes
   - Location-based attribute selection

3. **Fused Operations** (convenience functions)
   - `hitObjectReorderExecuteEXT()` - reorder + execute
   - `hitObjectTraceReorderExecuteEXT()` - trace + reorder + execute
   - `hitObjectTraceMotionReorderExecuteEXT()` - trace motion + reorder + execute

4. **Additional Query Methods**
   - `hitObjectGetRayFlagsEXT()`
   - `hitObjectGetRayTMaxEXT()` (vs just TMin in NV)
   - `hitObjectGetIntersectionTriangleVertexPositionsEXT()`

5. **Buffer Reference Support**
   - `hitobjectshaderrecordnv` layout qualifier (note: NV suffix but used with EXT)
   - `hitObjectGetShaderRecordBufferHandleEXT()` returns `uvec2` buffer handle

## Implementation Tasks

### 1. Capability System Updates
File: `source/slang/slang-capabilities.capdef`

```capdef
/// EXT shader invocation reorder (cross-vendor standard)
def GL_EXT_shader_invocation_reorder : GL_EXT_ray_tracing;

/// Combined SER capability for GLSL
alias ser_glsl = GL_NV_shader_invocation_reorder | GL_EXT_shader_invocation_reorder;
```

### 2. GLSL Emission Updates
File: `source/slang/hlsl.meta.slang`

For each HitObject method:
- Add `__glsl_extension(GL_EXT_shader_invocation_reorder)` decorator
- Update `case glsl:` to emit EXT-suffixed functions when appropriate
- May need to check active extension at runtime to choose NV vs EXT

Example pattern:
```slang
__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_EXT_shader_invocation_reorder)  // NEW
[require(glsl, ser_raygen_closesthit_miss)]
bool IsMiss()
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "hitObjectIsMissEXT($0)";  // Use EXT when GL_EXT active
    // ... other cases
    }
}
```

### 3. SPIRV Emission Updates
File: `source/slang/hlsl.meta.slang` (spirv_asm blocks)

Update SPIRV emission to use EXT ops when GL_EXT_shader_invocation_reorder is active:
- Check if we need `SPV_KHR_shader_invocation_reorder` or if NV extension is sufficient
- May need dual paths similar to HLSL (NV vs EXT ops)

Example:
```slang
case spirv:
    return spirv_asm
    {
        OpExtension "SPV_KHR_shader_invocation_reorder";  // or SPV_EXT?
        OpCapability ShaderInvocationReorderKHR;
        result:$$bool = OpHitObjectIsMissKHR &this;
    };
```

**NOTE**: Need to verify SPIRV extension names - check SPIRV registry!

### 4. New EXT-Specific Methods

#### a. FromRayQuery
```slang
[require(glsl, ser_raygen_closesthit_miss)]
static HitObject FromRayQuery(
    RayQuery Query,
    uint sbtRecordIndex,
    int attributeLocation)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "hitObjectRecordFromQueryEXT";
    case spirv:
        spirv_asm {
            OpExtension "SPV_KHR_shader_invocation_reorder";
            OpCapability ShaderInvocationReorderKHR;
            OpHitObjectRecordFromQueryEXT /**/ &__return_val
                /**/ $Query /**/ $sbtRecordIndex /**/ $attributeLocation;
        };
    }
}
```

#### b. GetAttributes with location
```slang
[require(glsl, ser_raygen_closesthit_miss)]
void GetAttributesEXT<attr_t>(int attributeLocation)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "hitObjectGetAttributesEXT($0, $1)";
    case spirv:
        spirv_asm {
            OpExtension "SPV_KHR_shader_invocation_reorder";
            OpCapability ShaderInvocationReorderKHR;
            OpHitObjectGetAttributesEXT &this $attributeLocation;
        };
    }
}
```

#### c. Fused Operations
```slang
[require(glsl, ser_raygen)]
static void ReorderExecute<payload_t>(
    HitObject hitObject,
    inout payload_t Payload)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "hitObjectReorderExecuteEXT($0, $1)";
    // Can be implemented as macro expansion:
    // ReorderThread(hitObject);
    // Invoke(hitObject, Payload);
    }
}
```

#### d. Additional Getters
- `GetRayTMax()` - already have TMin, add TMax
- `GetRayFlags()` - new
- `GetIntersectionTriangleVertexPositions()` - returns `vec3[3]`
- `GetShaderRecordBufferHandle()` - returns `uvec2`

### 5. Storage Qualifiers
File: `source/slang/slang-check-decl.cpp`, keyword handling

Add support for:
- `hitObjectAttributeEXT` storage qualifier
- Validation: only in raygen, closesthit, miss stages

### 6. Layout Qualifiers
File: `source/slang/slang-check-decl.cpp`

Add support for:
- `hitobjectshaderrecordnv` layout qualifier
- Only valid on buffer references

### 7. Testing

#### Test Files to Create:
1. `tests/glsl-intrinsic/shader-execution-reordering/ser-test-glsl-ext-basic.slang`
   - Basic EXT functions: MakeNop, MakeMiss, IsMiss, IsHit, IsEmpty

2. `tests/glsl-intrinsic/shader-execution-reordering/ser-test-glsl-ext-trace.slang`
   - TraceRay, TraceMotionRay

3. `tests/glsl-intrinsic/shader-execution-reordering/ser-test-glsl-ext-query.slang`
   - All getter functions

4. `tests/glsl-intrinsic/shader-execution-reordering/ser-test-glsl-ext-fromrayquery.slang`
   - FromRayQuery functionality

5. `tests/glsl-intrinsic/shader-execution-reordering/ser-test-glsl-ext-attributes.slang`
   - Attribute get/set with locations

6. `tests/glsl-intrinsic/shader-execution-reordering/ser-test-glsl-ext-fused.slang`
   - Fused operations

7. `tests/glsl-intrinsic/shader-execution-reordering/ser-test-glsl-ext-buffer-handle.slang`
   - Buffer handle and shader record access

#### Test Approach:
```slang
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage raygeneration -entry main

#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_shader_invocation_reorder : enable

// CHECK: OpExtension "SPV_KHR_shader_invocation_reorder"
// CHECK: OpHitObjectRecordEmptyEXT

void main() {
    hitObjectEXT hObj;
    hitObjectRecordEmptyEXT(hObj);
}
```

### 8. Documentation Updates
- Update SER.md to include GLSL EXT vs NV comparison
- Document new EXT-specific features
- Add usage examples

## Confirmed SPIRV Information ✅

1. **SPIRV Extension Name**: `SPV_EXT_shader_invocation_reorder`
2. **SPIRV Capability**: `ShaderInvocationReorderEXT`
3. **SPIRV Op Names**: All use `EXT` suffix (e.g., `OpHitObjectTraceRayEXT`)
4. **New Storage Class**: `HitObjectAttributeEXT`
5. **New Decoration**: `HitObjectShaderRecordBufferEXT`

Source: SPV_EXT_shader_invocation_reorder.asciidoc (downloaded)

3. **Backward Compatibility**:
   - Should we keep NV functions working?
   - Can shaders use both NV and EXT in same file?

4. **Capability Implication**:
   - Does `GL_EXT_shader_invocation_reorder` require `GL_EXT_ray_tracing`?
   - Yes, per spec line 46

5. **Extension Precedence**:
   - If both NV and EXT are enabled, which do we emit?
   - Prefer EXT as it's the standard?

## Implementation Order

1. ✅ Download and analyze GLSL_EXT_shader_invocation_reorder spec
2. ✅ Research SPIRV extension naming (confirmed: SPV_EXT_shader_invocation_reorder)
3. ✅ Add capability definitions for GL_EXT_shader_invocation_reorder
4. ✅ Update existing HitObject methods to support EXT GLSL emission
5. ⏳ Add new EXT-specific methods (FromRayQuery, GetAttributes, etc.)
6. ⏳ Add fused operation functions
7. ⏳ Implement storage and layout qualifiers
8. ✅ Update SPIRV emission for EXT ops (except RecordHit - see note below)
9. ⏳ Create comprehensive test suite
10. ⏳ Update documentation
11. ⏳ Validate with real GLSL compiler (glslang)

## ⚠️ IMPORTANT: MakeHit Operations Remain NV-Only

**Discovery**: The following SPIRV operations do NOT exist in SPV_EXT_shader_invocation_reorder:
- `OpHitObjectRecordHitNV` (remains NV-only)
- `OpHitObjectRecordHitMotionNV` (remains NV-only)
- `OpHitObjectRecordHitWithIndexNV` (remains NV-only)
- `OpHitObjectRecordHitWithIndexMotionNV` (remains NV-only)

These operations are used by the `MakeHit()` family of methods to manually construct hit objects without tracing.

**Resolution**: These operations remain as NV-only in the SPIRV path. The cross-vendor EXT standard provides alternative approaches:
- `OpHitObjectTraceRayEXT` - traces a ray and populates hit object
- `OpHitObjectRecordFromQueryEXT` - creates hit object from RayQuery (new in EXT)

## References
- GLSL_EXT_shader_invocation_reorder.txt (downloaded)
- SER.md (Slang implementation notes)
- DXR13_COMPREHENSIVE_TEST_RESULTS.md (test coverage baseline)
- SPIRV Registry: https://github.com/KhronosGroup/SPIRV-Registry
