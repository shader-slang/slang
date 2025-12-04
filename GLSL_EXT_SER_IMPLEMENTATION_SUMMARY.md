# GL_EXT_shader_invocation_reorder Implementation Summary

## Completed Work

### 1. Capability System Updates ✅
**File**: `source/slang/slang-capabilities.capdef`

Added support for the cross-vendor EXT shader invocation reorder extension:

```capdef
/// SPIRV extension for shader invocation reorder (EXT)
def SPV_EXT_shader_invocation_reorder : _spirv_1_4 + SPV_KHR_ray_tracing;

/// SPIRV capability for shader invocation reorder (EXT)
def spvShaderInvocationReorderEXT : SPV_EXT_shader_invocation_reorder;

/// GLSL extension atom
def _GL_EXT_shader_invocation_reorder : _GLSL_460;

/// GL_EXT_shader_invocation_reorder capability (combined GLSL + SPIRV)
alias GL_EXT_shader_invocation_reorder =
    _GL_EXT_shader_invocation_reorder + _GL_EXT_buffer_reference_uvec2
    | spvShaderInvocationReorderEXT;

/// Updated SER alias to include EXT path
alias ser = raytracing + GL_NV_shader_invocation_reorder
    | raytracing + GL_EXT_shader_invocation_reorder
    | ser_nvapi
    | ser_dxr
    | cuda;
```

### 2. HitObject Method Updates ✅
**File**: `source/slang/hlsl.meta.slang`

Updated all HitObject methods from NV to EXT:

#### GLSL Function Names Updated:
- `hitObjectIsMissNV` → `hitObjectIsMissEXT`
- `hitObjectIsHitNV` → `hitObjectIsHitEXT`
- `hitObjectIsEmptyNV` → `hitObjectIsEmptyEXT`
- `hitObjectRecordEmptyNV` → `hitObjectRecordEmptyEXT`
- `hitObjectRecordMissNV` → `hitObjectRecordMissEXT`
- `hitObjectRecordMissMotionNV` → `hitObjectRecordMissMotionEXT`
- `hitObjectTraceRayNV` → `hitObjectTraceRayEXT`
- `hitObjectTraceRayMotionNV` → `hitObjectTraceRayMotionEXT`
- `hitObjectExecuteShaderNV` → `hitObjectExecuteShaderEXT`
- `hitObjectGetInstanceIdNV` → `hitObjectGetInstanceIdEXT`
- `hitObjectGetInstanceCustomIndexNV` → `hitObjectGetInstanceCustomIndexEXT`
- `hitObjectGetGeometryIndexNV` → `hitObjectGetGeometryIndexEXT`
- `hitObjectGetPrimitiveIndexNV` → `hitObjectGetPrimitiveIndexEXT`
- `hitObjectGetHitKindNV` → `hitObjectGetHitKindEXT`
- `hitObjectGetWorldToObjectNV` → `hitObjectGetWorldToObjectEXT`
- `hitObjectGetObjectToWorldNV` → `hitObjectGetObjectToWorldEXT`
- `hitObjectGetCurrentTimeNV` → `hitObjectGetCurrentTimeEXT`
- `hitObjectGetObjectRayOriginNV` → `hitObjectGetObjectRayOriginEXT`
- `hitObjectGetObjectRayDirectionNV` → `hitObjectGetObjectRayDirectionEXT`
- `hitObjectGetWorldRayDirectionNV` → `hitObjectGetWorldRayDirectionEXT`
- `hitObjectGetWorldRayOriginNV` → `hitObjectGetWorldRayOriginEXT`
- `hitObjectGetRayTMaxNV` → `hitObjectGetRayTMaxEXT`
- `hitObjectGetRayTMinNV` → `hitObjectGetRayTMinEXT`
- `hitObjectGetAttributesNV` → `hitObjectGetAttributesEXT`
- `hitObjectGetShaderRecordBufferHandleNV` → `hitObjectGetShaderRecordBufferHandleEXT`
- `hitObjectGetShaderBindingTableRecordIndexNV` → `hitObjectGetShaderBindingTableRecordIndexEXT`
- `reorderThreadNV` → `reorderThreadEXT`

#### SPIRV Operations Updated:
- `OpHitObjectIsMissNV` → `OpHitObjectIsMissEXT`
- `OpHitObjectIsHitNV` → `OpHitObjectIsHitEXT`
- `OpHitObjectIsEmptyNV` → `OpHitObjectIsEmptyEXT`
- `OpHitObjectRecordEmptyNV` → `OpHitObjectRecordEmptyEXT`
- `OpHitObjectRecordMissNV` → `OpHitObjectRecordMissEXT`
- `OpHitObjectRecordMissMotionNV` → `OpHitObjectRecordMissMotionEXT`
- `OpHitObjectTraceRayNV` → `OpHitObjectTraceRayEXT`
- `OpHitObjectTraceRayMotionNV` → `OpHitObjectTraceRayMotionEXT`
- `OpHitObjectExecuteShaderNV` → `OpHitObjectExecuteShaderEXT`
- `OpHitObjectGetShaderBindingTableRecordIndexNV` → `OpHitObjectGetShaderBindingTableRecordIndexEXT`
- `OpHitObjectGetInstanceIdNV` → `OpHitObjectGetInstanceIdEXT`
- `OpHitObjectGetInstanceCustomIndexNV` → `OpHitObjectGetInstanceCustomIndexEXT`
- `OpHitObjectGetGeometryIndexNV` → `OpHitObjectGetGeometryIndexEXT`
- `OpHitObjectGetPrimitiveIndexNV` → `OpHitObjectGetPrimitiveIndexEXT`
- `OpHitObjectGetHitKindNV` → `OpHitObjectGetHitKindEXT`
- `OpHitObjectGetWorldToObjectNV` → `OpHitObjectGetWorldToObjectEXT`
- `OpHitObjectGetObjectToWorldNV` → `OpHitObjectGetObjectToWorldEXT`
- `OpHitObjectGetCurrentTimeNV` → `OpHitObjectGetCurrentTimeEXT`
- `OpHitObjectGetObjectRayOriginNV` → `OpHitObjectGetObjectRayOriginEXT`
- `OpHitObjectGetObjectRayDirectionNV` → `OpHitObjectGetObjectRayDirectionEXT`
- `OpHitObjectGetWorldRayDirectionNV` → `OpHitObjectGetWorldRayDirectionEXT`
- `OpHitObjectGetWorldRayOriginNV` → `OpHitObjectGetWorldRayOriginEXT`
- `OpHitObjectGetRayTMaxNV` → `OpHitObjectGetRayTMaxEXT`
- `OpHitObjectGetRayTMinNV` → `OpHitObjectGetRayTMinEXT`
- `OpHitObjectGetAttributesNV` → `OpHitObjectGetAttributesEXT`
- `OpHitObjectGetShaderRecordBufferHandleNV` → `OpHitObjectGetShaderRecordBufferHandleEXT`
- `OpReorderThreadWithHintNV` → `OpReorderThreadWithHintEXT`
- `OpReorderThreadWithHitObjectNV` → `OpReorderThreadWithHitObjectEXT`

#### Extension Decorators Updated:
All methods changed from:
```slang
__glsl_extension(GL_NV_shader_invocation_reorder)
```
to:
```slang
__glsl_extension(GL_EXT_shader_invocation_reorder)
```

#### SPIRV Extension/Capability Updated:
All SPIRV asm blocks changed from:
```slang
OpExtension "SPV_NV_shader_invocation_reorder";
OpCapability ShaderInvocationReorderNV;
```
to:
```slang
OpExtension "SPV_EXT_shader_invocation_reorder";
OpCapability ShaderInvocationReorderEXT;
```

### 3. Build Verification ✅
Successfully rebuilt `slangc` with all changes. Build completed without errors.

## ⚠️ Critical Finding: NV-Only Operations

The following SPIRV operations **do not exist** in the EXT specification and remain NV-only:

- `OpHitObjectRecordHitNV`
- `OpHitObjectRecordHitMotionNV`
- `OpHitObjectRecordHitWithIndexNV`
- `OpHitObjectRecordHitWithIndexMotionNV`

These are used by the `MakeHit()` family of methods to manually construct hit objects without ray tracing. The EXT specification does not provide equivalent operations.

**Impact**: The `MakeHit()` methods will continue to use NV-specific SPIRV operations when targeting SPIRV. Users requiring cross-vendor support should use:
- `TraceRay()` - traces a ray and populates hit object
- Future: `FromRayQuery()` - creates hit object from RayQuery (when implemented)

## Remaining Work

### High Priority:
1. **Testing**: Create comprehensive test suite for GLSL EXT path
   - Basic operations (IsMiss, IsHit, IsEmpty)
   - Ray tracing (TraceRay, TraceMotionRay)
   - Query methods (all getters)
   - ReorderThread operations

2. **Validation**: Test generated GLSL/SPIRV with glslang and real drivers
   - Verify GLSL function names are correct
   - Verify SPIRV opcodes are correct
   - Test on NVIDIA drivers with EXT support

### Medium Priority:
3. **New EXT Features**: Implement EXT-specific functionality
   - `FromRayQuery()` - populate HitObject from RayQuery
   - `GetAttributes()` with location parameter
   - Fused operations (ReorderExecute, TraceReorderExecute, etc.)
   - Additional getters (GetRayFlags, GetIntersectionTriangleVertexPositions)

4. **Storage/Layout Qualifiers**:
   - `hitObjectAttributeEXT` storage qualifier
   - `hitobjectshaderrecordnv` layout qualifier

### Low Priority:
5. **Documentation**: Update SER.md with EXT vs NV comparison
6. **Cleanup**: Remove or update the now-obsolete `update_ser_to_ext.py` script

## Files Modified

1. `source/slang/slang-capabilities.capdef` - Added EXT capability definitions
2. `source/slang/hlsl.meta.slang` - Updated all HitObject methods and helper functions
3. `GLSL_EXT_SER_IMPLEMENTATION_PLAN.md` - Updated implementation status

## Files Created

1. `GLSL_EXT_shader_invocation_reorder.txt` - GLSL extension specification
2. `SPV_EXT_shader_invocation_reorder.asciidoc` - SPIRV extension specification
3. `GLSL_EXT_SER_IMPLEMENTATION_PLAN.md` - Implementation planning document
4. `GLSL_EXT_SER_IMPLEMENTATION_SUMMARY.md` - This summary document

## Verification Commands

To test the implementation:

```bash
# Compile a test shader targeting SPIRV with EXT
./build/Debug/bin/slangc.exe tests/hlsl-intrinsic/shader-execution-reordering/[test].slang \
    -target spirv-asm \
    -stage raygeneration \
    -entry main

# Run comprehensive SER tests
./build/Release/bin/slang-test.exe tests/hlsl-intrinsic/shader-execution-reordering/
```

## Summary

Successfully implemented basic GL_EXT_shader_invocation_reorder support by:
- Adding capability system support for the EXT extension
- Updating all HitObject methods to use EXT GLSL functions and SPIRV operations
- Maintaining NV-only operations where EXT equivalents don't exist (MakeHit family)
- Verifying compilation succeeds with all changes

The implementation follows the same pattern as the existing DXR 1.3 dual-API support, allowing Slang to target both NVIDIA-specific and cross-vendor shader invocation reordering extensions.
