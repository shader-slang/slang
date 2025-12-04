# GLSL EXT Shader Invocation Reorder - Implementation Verification

## Overview
This document verifies the completeness of our GLSL EXT implementation against official specifications:
- **GLSL Spec**: `GLSL_EXT_shader_invocation_reorder.txt`
- **SPIRV Spec**: `SPV_EXT_shader_invocation_reorder.asciidoc`

## Summary

| Category | Total Required | Implemented | Missing | Status |
|----------|----------------|-------------|---------|--------|
| Core Functions | 38 | 34 | 4 | 🟡 Incomplete |
| SPIRV Instructions | 33 | 29 | 4 | 🟡 Incomplete |

## Detailed Function Checklist

### ✅ Batch 1: Core Trace Functions (5/5)
| Function | Spec Line | Implementation Line | SPIRV Op | Status |
|----------|-----------|---------------------|----------|--------|
| `hitObjectTraceRayEXT()` | GLSL:275 | glsl.meta.slang:6377 | OpHitObjectTraceRayEXT | ✅ |
| `hitObjectTraceRayMotionEXT()` | GLSL:310 | glsl.meta.slang:6440 | OpHitObjectTraceRayMotionEXT | ✅ |
| `hitObjectRecordMissEXT()` | GLSL:386 | glsl.meta.slang:6507 | OpHitObjectRecordMissEXT | ✅ |
| `hitObjectRecordMissMotionEXT()` | GLSL:406 | glsl.meta.slang:6546 | OpHitObjectRecordMissMotionEXT | ✅ |
| `hitObjectRecordEmptyEXT()` | GLSL:428 | glsl.meta.slang:6588 | OpHitObjectRecordEmptyEXT | ✅ |

### ✅ Batch 2: Execute & Query Functions (5/5)
| Function | Spec Line | Implementation Line | SPIRV Op | Status |
|----------|-----------|---------------------|----------|--------|
| `hitObjectExecuteShaderEXT()` | GLSL:436 | glsl.meta.slang:6617 | OpHitObjectExecuteShaderEXT | ✅ |
| `hitObjectIsEmptyEXT()` | GLSL:460 | glsl.meta.slang:6645 | OpHitObjectIsEmptyEXT | ✅ |
| `hitObjectIsMissEXT()` | GLSL:467 | glsl.meta.slang:6656 | OpHitObjectIsMissEXT | ✅ |
| `hitObjectIsHitEXT()` | GLSL:474 | glsl.meta.slang:6667 | OpHitObjectIsHitEXT | ✅ |
| `hitObjectGetRayTMinEXT()` | GLSL:481 | glsl.meta.slang:6678 | OpHitObjectGetRayTMinEXT | ✅ |

### ✅ Batch 3: Ray Property Getters (5/5)
| Function | Spec Line | Implementation Line | SPIRV Op | Status |
|----------|-----------|---------------------|----------|--------|
| `hitObjectGetRayTMaxEXT()` | GLSL:489 | glsl.meta.slang:6691 | OpHitObjectGetRayTMaxEXT | ✅ |
| `hitObjectGetRayFlagsEXT()` | GLSL:496 | glsl.meta.slang:6702 | OpHitObjectGetRayFlagsEXT | ✅ |
| `hitObjectGetWorldRayOriginEXT()` | GLSL:516 | glsl.meta.slang:6724 | OpHitObjectGetWorldRayOriginEXT | ✅ |
| `hitObjectGetWorldRayDirectionEXT()` | GLSL:523 | glsl.meta.slang:6735 | OpHitObjectGetWorldRayDirectionEXT | ✅ |
| `hitObjectGetObjectRayOriginEXT()` | GLSL:503 | glsl.meta.slang:6746 | OpHitObjectGetObjectRayOriginEXT | ✅ |

### ✅ Batch 4: Transform & Instance Getters (5/5)
| Function | Spec Line | Implementation Line | SPIRV Op | Status |
|----------|-----------|---------------------|----------|--------|
| `hitObjectGetObjectRayDirectionEXT()` | GLSL:510 | glsl.meta.slang:6759 | OpHitObjectGetObjectRayDirectionEXT | ✅ |
| `hitObjectGetObjectToWorldEXT()` | GLSL:530 | glsl.meta.slang:6770 | OpHitObjectGetObjectToWorldEXT | ✅ |
| `hitObjectGetWorldToObjectEXT()` | GLSL:536 | glsl.meta.slang:6781 | OpHitObjectGetWorldToObjectEXT | ✅ |
| `hitObjectGetInstanceIdEXT()` | GLSL:556 | glsl.meta.slang:6792 | OpHitObjectGetInstanceIdEXT | ✅ |
| `hitObjectGetInstanceCustomIndexEXT()` | GLSL:549 | glsl.meta.slang:6803 | OpHitObjectGetInstanceCustomIndexEXT | ✅ |

### ✅ Batch 5: Geometry & Hit Property Getters (5/5)
| Function | Spec Line | Implementation Line | SPIRV Op | Status |
|----------|-----------|---------------------|----------|--------|
| `hitObjectGetGeometryIndexEXT()` | GLSL:563 | glsl.meta.slang:6816 | OpHitObjectGetGeometryIndexEXT | ✅ |
| `hitObjectGetPrimitiveIndexEXT()` | GLSL:571 | glsl.meta.slang:6827 | OpHitObjectGetPrimitiveIndexEXT | ✅ |
| `hitObjectGetHitKindEXT()` | GLSL:579 | glsl.meta.slang:6838 | OpHitObjectGetHitKindEXT | ✅ |
| `hitObjectGetCurrentTimeEXT()` | GLSL:641 | glsl.meta.slang:6849 | OpHitObjectGetCurrentTimeEXT | ✅ |
| `hitObjectGetAttributesEXT()` | GLSL:587 | **NOT IMPLEMENTED** | OpHitObjectGetAttributesEXT | ❌ |

### 🟡 Batch 6: Shader Binding Table Functions (5/5)
| Function | Spec Line | Implementation Line | SPIRV Op | Status |
|----------|-----------|---------------------|----------|--------|
| `hitObjectGetShaderBindingTableRecordIndexEXT()` | GLSL:625 | glsl.meta.slang:6873 | OpHitObjectGetShaderBindingTableRecordIndexEXT | ✅ |
| `hitObjectSetShaderBindingTableRecordIndexEXT()` | GLSL:633 | glsl.meta.slang:6884 | OpHitObjectSetShaderBindingTableRecordIndexEXT | ✅ |
| `hitObjectGetShaderRecordBufferHandleEXT()` | GLSL:613 | glsl.meta.slang:6908 | OpHitObjectGetShaderRecordBufferHandleEXT | ✅ |
| `reorderThreadEXT(uint, uint)` | GLSL:665 | glsl.meta.slang:6919 | OpReorderThreadWithHintEXT | ✅ |
| `reorderThreadEXT(hitObjectEXT)` | GLSL:683 | glsl.meta.slang:6943 | OpReorderThreadWithHitObjectEXT | ✅ |

### ✅ Batch 7: Advanced Functions (5/5)
| Function | Spec Line | Implementation Line | SPIRV Op | Status |
|----------|-----------|---------------------|----------|--------|
| `reorderThreadEXT(hitObjectEXT, uint, uint)` | GLSL:692 | glsl.meta.slang:6968 | OpReorderThreadWithHintEXT | ✅ |
| `hitObjectRecordFromQueryEXT()` | GLSL:354 | glsl.meta.slang:6992 | OpHitObjectRecordFromQueryEXT | ✅ |
| `hitObjectGetIntersectionTriangleVertexPositionsEXT()` | GLSL:542 | glsl.meta.slang:7016 | OpHitObjectGetIntersectionTriangleVertexPositionsEXT | ✅ |
| `hitObjectReorderExecuteEXT(hitObjectEXT, int)` | GLSL:715 | glsl.meta.slang:7040 | OpHitObjectReorderExecuteShaderEXT | ✅ |
| `hitObjectReorderExecuteEXT(hitObjectEXT, uint, uint, int)` | GLSL:725 | glsl.meta.slang:7066 | OpHitObjectReorderExecuteShaderEXT | ✅ |

### ❌ Batch 8: Fused Trace Functions (0/3) - MISSING
| Function | Spec Line | Implementation Line | SPIRV Op | Status |
|----------|-----------|---------------------|----------|--------|
| `hitObjectTraceReorderExecuteEXT()` (no hint) | GLSL:735 | **NOT IMPLEMENTED** | OpHitObjectTraceReorderExecuteEXT | ❌ |
| `hitObjectTraceReorderExecuteEXT()` (with hint) | GLSL:756 | **NOT IMPLEMENTED** | OpHitObjectTraceReorderExecuteEXT | ❌ |
| `hitObjectTraceMotionReorderExecuteEXT()` | GLSL:779 | **NOT IMPLEMENTED** | OpHitObjectTraceMotionReorderExecuteEXT | ❌ |

**Note**: glsl.meta.slang:7087 contains only a comment placeholder for Batch 8, no actual implementation.

## Missing Functions Analysis

### 1. hitObjectGetAttributesEXT() - Batch 5
**Spec Definition** (GLSL:587-609):
```glsl
void hitObjectGetAttributesEXT(hitObjectEXT hitObject, int attributeLocation);
```
**Purpose**: Extracts attributes encoded in hit object and writes to hitObjectAttributeEXT storage class variable.

**Why Important**: Required for accessing custom intersection attributes from hit objects created via:
- `hitObjectTraceRayEXT()` / `hitObjectTraceRayMotionEXT()` (intersection shader attributes)
- `hitObjectRecordFromQueryEXT()` (ray query attributes)

**SPIRV Mapping**: OpHitObjectGetAttributesEXT (SPV spec line 627-641)

### 2. hitObjectTraceReorderExecuteEXT() - No Hint Version
**Spec Definition** (GLSL:735-753):
```glsl
void hitObjectTraceReorderExecuteEXT(
    hitObjectEXT hitobject,
    accelerationStructureEXT topLevel,
    uint rayFlags,
    uint cullMask,
    uint sbtRecordOffset,
    uint sbtRecordStride,
    uint missIndex,
    vec3 origin,
    float Tmin,
    vec3 direction,
    float Tmax,
    int payload);
```
**Purpose**: Fused operation equivalent to:
```glsl
hitObjectTraceRayEXT(...);
reorderThreadEXT(hitObject);
hitObjectExecuteShaderEXT(hitObject, payload);
```

**SPIRV Mapping**: OpHitObjectTraceReorderExecuteEXT (SPV spec line 1128-1189)

### 3. hitObjectTraceReorderExecuteEXT() - With Hint Version
**Spec Definition** (GLSL:756-776):
```glsl
void hitObjectTraceReorderExecuteEXT(
    hitObjectEXT hitobject,
    accelerationStructureEXT topLevel,
    uint rayFlags,
    uint cullMask,
    uint sbtRecordOffset,
    uint sbtRecordStride,
    uint missIndex,
    vec3 origin,
    float Tmin,
    vec3 direction,
    float Tmax,
    uint hint,
    uint bits,
    int payload);
```
**Purpose**: Fused operation equivalent to:
```glsl
hitObjectTraceRayEXT(...);
reorderThreadEXT(hitObject, hint, bits);
hitObjectExecuteShaderEXT(hitObject, payload);
```

**SPIRV Mapping**: OpHitObjectTraceReorderExecuteEXT with optional hint/bits (SPV spec line 1128-1189)

### 4. hitObjectTraceMotionReorderExecuteEXT()
**Spec Definition** (GLSL:779-800):
```glsl
void hitObjectTraceMotionReorderExecuteEXT(
    hitObjectEXT hitobject,
    accelerationStructureEXT topLevel,
    uint rayFlags,
    uint cullMask,
    uint sbtRecordOffset,
    uint sbtRecordStride,
    uint missIndex,
    vec3 origin,
    float Tmin,
    vec3 direction,
    float Tmax,
    float currentTime,
    uint hint,
    uint bits,
    int payload);
```
**Purpose**: Motion blur variant of fused trace+reorder+execute.

**SPIRV Mapping**: OpHitObjectTraceMotionReorderExecuteEXT (SPV spec line 1193-1257)

## SPIRV Instruction Verification

### ✅ Implemented SPIRV Instructions (29/33)

All implemented functions correctly use their corresponding SPIRV instructions:
- OpTypeHitObjectEXT ✅
- OpReorderThreadWithHintEXT ✅
- OpReorderThreadWithHitObjectEXT ✅
- OpHitObjectTraceRayEXT ✅
- OpHitObjectTraceRayMotionEXT ✅
- OpHitObjectRecordFromQueryEXT ✅
- OpHitObjectRecordMissEXT ✅
- OpHitObjectRecordMissMotionEXT ✅
- OpHitObjectRecordEmptyEXT ✅
- OpHitObjectExecuteShaderEXT ✅
- OpHitObjectIsEmptyEXT ✅
- OpHitObjectIsMissEXT ✅
- OpHitObjectIsHitEXT ✅
- OpHitObjectGetRayTMinEXT ✅
- OpHitObjectGetRayTMaxEXT ✅
- OpHitObjectGetRayFlagsEXT ✅
- OpHitObjectGetWorldRayOriginEXT ✅
- OpHitObjectGetWorldRayDirectionEXT ✅
- OpHitObjectGetObjectRayOriginEXT ✅
- OpHitObjectGetObjectRayDirectionEXT ✅
- OpHitObjectGetObjectToWorldEXT ✅
- OpHitObjectGetWorldToObjectEXT ✅
- OpHitObjectGetIntersectionTriangleVertexPositionsEXT ✅
- OpHitObjectGetInstanceIdEXT ✅
- OpHitObjectGetInstanceCustomIndexEXT ✅
- OpHitObjectGetGeometryIndexEXT ✅
- OpHitObjectGetPrimitiveIndexEXT ✅
- OpHitObjectGetHitKindEXT ✅
- OpHitObjectGetCurrentTimeEXT ✅
- OpHitObjectGetShaderBindingTableRecordIndexEXT ✅
- OpHitObjectSetShaderBindingTableRecordIndexEXT ✅
- OpHitObjectGetShaderRecordBufferHandleEXT ✅
- OpHitObjectReorderExecuteShaderEXT ✅

### ❌ Missing SPIRV Instructions (4/33)
- OpHitObjectGetAttributesEXT ❌
- OpHitObjectTraceReorderExecuteEXT ❌ (used by 2 GLSL functions)
- OpHitObjectTraceMotionReorderExecuteEXT ❌

## Verification Against NV Implementation

### Key Differences (Correctly Implemented)
✅ Type naming: `hitObjectEXT` vs `hitObjectNV`
✅ Target: `case spirv:` (generic) vs `case spirv_nv:` (sub-target)
✅ Extension: `SPV_EXT_shader_invocation_reorder` vs `SPV_NV_shader_invocation_reorder`
✅ Capability: `ShaderInvocationReorderEXT` vs `ShaderInvocationReorderNV`
✅ Operations: `OpFunctionNameEXT` vs `OpFunctionNameNV`

### New Functions in EXT (Not in NV)
✅ `hitObjectRecordFromQueryEXT()` - Create from RayQuery
✅ `hitObjectGetIntersectionTriangleVertexPositionsEXT()` - Triangle vertices
✅ `hitObjectSetShaderBindingTableRecordIndexEXT()` - Set SBT index
✅ `hitObjectGetRayTMinEXT()` - Ray TMin query
✅ `hitObjectGetRayTMaxEXT()` - Ray TMax query
✅ `hitObjectGetRayFlagsEXT()` - Ray flags query
✅ `hitObjectGetWorldRayOriginEXT()` - World-space origin
✅ `hitObjectGetWorldRayDirectionEXT()` - World-space direction
✅ `hitObjectReorderExecuteEXT()` (2 overloads) - Fused reorder+execute
❌ `hitObjectTraceReorderExecuteEXT()` (2 overloads) - Fused trace+reorder+execute (MISSING)
❌ `hitObjectTraceMotionReorderExecuteEXT()` - Motion blur fused (MISSING)
❌ `hitObjectGetAttributesEXT()` - Get attributes (MISSING)

## Recommendations

### Priority 1: Critical for Attribute Handling
**Implement `hitObjectGetAttributesEXT()`**
- Required for accessing custom intersection attributes
- Used with `hitObjectAttributeEXT` storage class
- Essential for practical use of hit objects created from traces or queries

### Priority 2: Performance Optimization Functions
**Implement Batch 8 Fused Functions**
- `hitObjectTraceReorderExecuteEXT()` (2 overloads)
- `hitObjectTraceMotionReorderExecuteEXT()`
- These are convenience/optimization functions
- Can be manually composed from separate calls, but fused versions enable better compiler optimization

## Conclusion

Our EXT implementation is **89.5% complete** (34/38 functions).

**Critical Missing**: `hitObjectGetAttributesEXT()` is essential for real-world usage.
**Optional Missing**: The 3 fused trace functions are convenience/optimization features that can be worked around.

All implemented functions correctly follow the dual-API pattern and use proper SPIRV instructions.
