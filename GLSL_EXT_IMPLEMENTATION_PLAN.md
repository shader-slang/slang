# GLSL_EXT_shader_invocation_reorder Implementation Plan

## Overview
Add missing EXT GLSL standalone functions to `source/slang/glsl.meta.slang` to support cross-vendor shader invocation reordering.

## Current Status
- ✅ **hlsl.meta.slang**: Dual SPIRV-API support complete (spirv_nv + spirv)
- ✅ **glsl.meta.slang**: NV functions implemented (17 functions)
- ❌ **glsl.meta.slang**: EXT functions MISSING (~30+ functions)

## Implementation Strategy

### Pattern to Follow
```glsl
__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_EXT_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[ForceInline]
[require(glsl, capability)]
[require(spirv, capability)]
public void functionNameEXT(
    inout hitObjectEXT hitObject,  // Note: hitObjectEXT type
    /* parameters */
)
{
    __target_switch
    {
    case glsl:
    {
        HitObject::__glslFunctionNameEXT(...);
    }
    case spirv:  // Generic spirv target
    {
        spirv_asm
        {
            OpExtension "SPV_EXT_shader_invocation_reorder";
            OpCapability ShaderInvocationReOrderEXT;
            OpFunctionNameEXT ...;
        };
    }
    }
}
```

## Functions to Implement

### Batch 1: Core Trace Functions (5 functions)
1. `hitObjectTraceRayEXT()` - Trace ray into hit object
2. `hitObjectTraceRayMotionEXT()` - Trace ray with motion blur
3. `hitObjectRecordMissEXT()` - Record miss without tracing
4. `hitObjectRecordMissMotionEXT()` - Record miss with motion blur
5. `hitObjectRecordEmptyEXT()` - Record empty hit object

**Compile & Test**

### Batch 2: Execute & Query Functions (5 functions)
6. `hitObjectExecuteShaderEXT()` - Execute shader from hit object
7. `hitObjectIsEmptyEXT()` - Query if empty
8. `hitObjectIsMissEXT()` - Query if miss
9. `hitObjectIsHitEXT()` - Query if hit
10. `hitObjectGetRayTMinEXT()` - Get ray TMin

**Compile & Test**

### Batch 3: Ray Property Getters (5 functions)
11. `hitObjectGetRayTMaxEXT()` - Get ray TMax
12. `hitObjectGetRayFlagsEXT()` - Get ray flags
13. `hitObjectGetWorldRayOriginEXT()` - Get world-space ray origin
14. `hitObjectGetWorldRayDirectionEXT()` - Get world-space ray direction
15. `hitObjectGetObjectRayOriginEXT()` - Get object-space ray origin

**Compile & Test**

### Batch 4: Transform & Instance Getters (5 functions)
16. `hitObjectGetObjectRayDirectionEXT()` - Get object-space ray direction
17. `hitObjectGetObjectToWorldEXT()` - Get object-to-world transform
18. `hitObjectGetWorldToObjectEXT()` - Get world-to-object transform
19. `hitObjectGetInstanceIdEXT()` - Get instance ID
20. `hitObjectGetInstanceCustomIndexEXT()` - Get instance custom index

**Compile & Test**

### Batch 5: Geometry & Hit Property Getters (5 functions)
21. `hitObjectGetGeometryIndexEXT()` - Get geometry index
22. `hitObjectGetPrimitiveIndexEXT()` - Get primitive index
23. `hitObjectGetHitKindEXT()` - Get hit kind
24. `hitObjectGetCurrentTimeEXT()` - Get current time (motion blur)
25. `hitObjectGetAttributesEXT()` - Get hit attributes

**Compile & Test**

### Batch 6: Shader Binding Table Functions (5 functions)
26. `hitObjectGetShaderBindingTableRecordIndexEXT()` - Get SBT record index
27. `hitObjectSetShaderBindingTableRecordIndexEXT()` - Set SBT record index (NEW to EXT)
28. `hitObjectGetShaderRecordBufferHandleEXT()` - Get shader record buffer handle
29. `reorderThreadEXT(uint, uint)` - Reorder with hint
30. `reorderThreadEXT(hitObjectEXT)` - Reorder with hit object

**Compile & Test**

### Batch 7: Advanced Functions (5 functions)
31. `reorderThreadEXT(hitObjectEXT, uint, uint)` - Reorder with hit object and hint
32. `hitObjectRecordFromQueryEXT()` - Record from ray query (NEW to EXT)
33. `hitObjectGetIntersectionTriangleVertexPositionsEXT()` - Get triangle vertices (NEW to EXT)
34. `hitObjectReorderExecuteEXT(hitObjectEXT, int)` - Fused reorder+execute (NEW to EXT)
35. `hitObjectReorderExecuteEXT(hitObjectEXT, uint, uint, int)` - Fused with hint (NEW to EXT)

**Compile & Test**

### Batch 8: Fused Trace Functions (3 functions)
36. `hitObjectTraceReorderExecuteEXT()` - Fused trace+reorder+execute (NEW to EXT)
37. `hitObjectTraceReorderExecuteEXT()` with hint - Fused with hint (NEW to EXT)
38. `hitObjectTraceMotionReorderExecuteEXT()` - Fused with motion blur (NEW to EXT)

**Compile & Test**

## Key Differences from NV Implementation

1. **Type**: Uses `hitObjectEXT` instead of `hitObjectNV`
2. **Target**: Uses `case spirv:` (generic) instead of `case spirv_nv:`
3. **Extension**: Uses `SPV_EXT_shader_invocation_reorder` instead of `SPV_NV_shader_invocation_reorder`
4. **Capability**: Uses `ShaderInvocationReorderEXT` instead of `ShaderInvocationReorderNV`
5. **Operations**: Uses `OpFunctionNameEXT` instead of `OpFunctionNameNV`

## New Functions (Not in NV)

These functions are **only** in the EXT specification:
- `hitObjectRecordFromQueryEXT()` - Create hit object from RayQuery
- `hitObjectGetIntersectionTriangleVertexPositionsEXT()` - Get triangle vertex positions
- `hitObjectSetShaderBindingTableRecordIndexEXT()` - Set SBT record index
- `hitObjectGetRayTMinEXT()`, `hitObjectGetRayTMaxEXT()`, `hitObjectGetRayFlagsEXT()` - Ray property queries
- `hitObjectGetWorldRayOriginEXT()`, `hitObjectGetWorldRayDirectionEXT()` - World-space ray queries
- All fused functions: `hitObjectReorderExecuteEXT()`, `hitObjectTraceReorderExecuteEXT()`, `hitObjectTraceMotionReorderExecuteEXT()`

## Testing Strategy

After each batch:
1. Compile with `cmake.exe --build --preset debug`
2. Verify no errors
3. Check that EXT functions are available in capability system

## Notes

- These are **GLSL standalone functions**, not methods on HitObject struct
- They complement the existing dual-API support in `hlsl.meta.slang`
- Used by GLSL shader writers targeting Vulkan with cross-vendor SER support
- Not related to DXR 1.3 (which uses HLSL)

## Location in File

Add EXT functions after existing NV functions in `source/slang/glsl.meta.slang`, starting around line 6400+
