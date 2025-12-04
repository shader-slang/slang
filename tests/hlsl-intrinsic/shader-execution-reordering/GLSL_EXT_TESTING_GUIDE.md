# GLSL EXT Shader Invocation Reorder - Testing Guide

This guide explains how to test the GL_EXT_shader_invocation_reorder implementation comprehensively.

## Test Files Created

### 1. glsl-ext-spirv-emission-test.slang
**Purpose**: Validates EXT functions generate correct SPIRV opcodes in both emission paths
**Tests**: Core hit object operations (MakeNop, MakeMiss, MakeHit, TraceRay, SBT operations)
**Emission Paths**:
- `-emit-spirv-directly` (default SPIRV generation)
- `-emit-spirv-via-glsl` (SPIRV via GLSL translation)

### 2. glsl-ext-comprehensive-test.slang
**Purpose**: Comprehensive test of all 38 EXT functions
**Coverage**: All hitObjectEXT and reorderThreadEXT functions
**Note**: Requires updates for proper API usage

### 3. glsl-ext-standalone-functions.slang
**Purpose**: Tests GLSL standalone EXT functions from glsl.meta.slang
**Focus**: Public GLSL API functions

### 4. glsl-ext-fused-trace-functions.slang
**Purpose**: Tests the 3 new Batch 8 fused functions
**Functions**:
- hitObjectTraceReorderExecuteEXT (no hint)
- hitObjectTraceReorderExecuteEXT (with hint)
- hitObjectTraceMotionReorderExecuteEXT

## Running Tests

### Method 1: Using slang-test (Recommended)

```bash
# Run all SER tests
./build/Debug/bin/slang-test tests/hlsl-intrinsic/shader-execution-reordering/

# Run specific EXT test
./build/Debug/bin/slang-test tests/hlsl-intrinsic/shader-execution-reordering/glsl-ext-spirv-emission-test.slang

# Run with verbose output
./build/Debug/bin/slang-test -v tests/hlsl-intrinsic/shader-execution-reordering/glsl-ext-spirv-emission-test.slang
```

### Method 2: Using slangc directly

```bash
# Direct SPIRV emission
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/glsl-ext-spirv-emission-test.slang \
    -target spirv \
    -entry rayGenerationMain \
    -stage raygeneration \
    -emit-spirv-directly \
    -o test-output-direct.spv

# SPIRV via GLSL
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/glsl-ext-spirv-emission-test.slang \
    -target spirv \
    -entry rayGenerationMain \
    -stage raygeneration \
    -emit-spirv-via-glsl \
    -o test-output-via-glsl.spv

# Verify SPIRV is valid
spirv-val test-output-direct.spv
spirv-val test-output-via-glsl.spv
```

### Method 3: Generate and inspect SPIRV assembly

```bash
# Generate SPIRV assembly for inspection
./build/Debug/bin/slangc tests/hlsl-intrinsic/shader-execution-reordering/glsl-ext-spirv-emission-test.slang \
    -target spirv-asm \
    -entry rayGenerationMain \
    -stage raygeneration \
    -o test-output.spvasm

# Check for EXT opcodes
grep "OpHitObject.*EXT" test-output.spvasm
grep "OpReorderThread.*EXT" test-output.spvasm
```

## What Each Test Verifies

### SPIRV Opcode Generation
All tests use `filecheck` directives to verify correct SPIRV opcodes are emitted:

```c++
// CHECK: OpCapability ShaderInvocationReorderEXT
// CHECK: OpExtension "SPV_EXT_shader_invocation_reorder"
// CHECK: OpHitObjectTraceRayEXT
// CHECK: OpHitObjectIsHitEXT
```

### Both Emission Paths
Tests run with both:
1. **Direct emission** (`-emit-spirv-directly`): Slang → SPIRV
2. **Via GLSL** (`-emit-spirv-via-glsl`): Slang → GLSL → SPIRV (via glslang)

This ensures EXT functions work correctly in both code paths.

### API Correctness
Tests verify:
- Correct parameter types
- Proper capability requirements
- Valid function overloads
- Correct return types

## Expected SPIRV Instructions

Tests should generate all 36 EXT SPIRV instructions:

### Type and Reorder Instructions (3)
- `OpTypeHitObjectEXT`
- `OpReorderThreadWithHintEXT`
- `OpReorderThreadWithHitObjectEXT`

### Trace and Record Instructions (6)
- `OpHitObjectTraceRayEXT`
- `OpHitObjectTraceRayMotionEXT`
- `OpHitObjectRecordFromQueryEXT`
- `OpHitObjectRecordMissEXT`
- `OpHitObjectRecordMissMotionEXT`
- `OpHitObjectRecordEmptyEXT`

### Query Instructions (9)
- `OpHitObjectIsEmptyEXT`
- `OpHitObjectIsMissEXT`
- `OpHitObjectIsHitEXT`
- `OpHitObjectGetRayTMinEXT`
- `OpHitObjectGetRayTMaxEXT`
- `OpHitObjectGetRayFlagsEXT`
- `OpHitObjectGetWorldRayOriginEXT`
- `OpHitObjectGetWorldRayDirectionEXT`
- `OpHitObjectGetObjectRayOriginEXT`

### Property Getter Instructions (15)
- `OpHitObjectGetObjectRayDirectionEXT`
- `OpHitObjectGetObjectToWorldEXT`
- `OpHitObjectGetWorldToObjectEXT`
- `OpHitObjectGetIntersectionTriangleVertexPositionsEXT`
- `OpHitObjectGetInstanceIdEXT`
- `OpHitObjectGetInstanceCustomIndexEXT`
- `OpHitObjectGetGeometryIndexEXT`
- `OpHitObjectGetPrimitiveIndexEXT`
- `OpHitObjectGetHitKindEXT`
- `OpHitObjectGetCurrentTimeEXT`
- `OpHitObjectGetAttributesEXT`
- `OpHitObjectGetShaderBindingTableRecordIndexEXT`
- `OpHitObjectSetShaderBindingTableRecordIndexEXT`
- `OpHitObjectGetShaderRecordBufferHandleEXT`
- `OpHitObjectExecuteShaderEXT`

### Fused Instructions (3)
- `OpHitObjectReorderExecuteShaderEXT`
- `OpHitObjectTraceReorderExecuteEXT`
- `OpHitObjectTraceMotionReorderExecuteEXT`

## Troubleshooting

### Issue: Capability errors
```
error 36107: entrypoint uses features not available in stage
```
**Solution**: Add capability flag: `-capability spirv` or use slang-test which handles this automatically

### Issue: L-value errors for payload
```
error 30047: argument passed to parameter must be l-value
```
**Solution**: Declare payload as a variable, don't pass temporary:
```c++
// Wrong:
hitObj = HitObject::TraceRay(scene, ..., Payload());

// Correct:
Payload p;
hitObj = HitObject::TraceRay(scene, ..., p);
```

### Issue: Method not found
```
error 30027: 'MethodName' is not a member of 'HitObject'
```
**Solution**: Check API - some functions are static methods:
```c++
// Correct usage:
HitObject::Invoke(scene, hitObj, payload);  // Static method
bool isHit = hitObj.IsHit();                // Instance method
```

## Validating Complete Coverage

To verify all 38 EXT functions are tested:

```bash
# Extract all CHECK directives from tests
grep -h "// CHECK.*Op.*EXT" tests/hlsl-intrinsic/shader-execution-reordering/glsl-ext-*.slang | \
    sed 's/.*CHECK.*: //' | sort -u

# Should list all 36 SPIRV EXT instructions
```

## Continuous Integration

Add these tests to CI pipeline:

```bash
# In CI script
./build/Release/bin/slang-test \
    tests/hlsl-intrinsic/shader-execution-reordering/glsl-ext-*.slang \
    -use-test-server \
    -server-count 4
```

## Performance Testing

For performance validation of fused operations:

```bash
# Profile direct SPIRV emission
time ./build/Release/bin/slangc test.slang -target spirv -emit-spirv-directly

# Profile via GLSL emission
time ./build/Release/bin/slangc test.slang -target spirv -emit-spirv-via-glsl

# Compare generated SPIRV instruction counts
spirv-dis output-direct.spv | wc -l
spirv-dis output-via-glsl.spv | wc -l
```

## Next Steps

1. **Run all tests**: `./build/Debug/bin/slang-test tests/hlsl-intrinsic/shader-execution-reordering/glsl-ext-*.slang`
2. **Fix any failures**: Update tests to match actual API
3. **Verify SPIRV**: Check generated SPIRV assembly for correct opcodes
4. **Add to CI**: Include in automated test suite
5. **Document results**: Note any platform-specific behaviors

## References

- GLSL Spec: `GLSL_EXT_shader_invocation_reorder.txt`
- SPIRV Spec: `SPV_EXT_shader_invocation_reorder.asciidoc`
- Implementation: `source/slang/glsl.meta.slang` (lines 6366-7259)
- Verification: `FINAL_100_PERCENT_PROOF.md`
