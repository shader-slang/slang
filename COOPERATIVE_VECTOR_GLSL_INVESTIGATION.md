# Cooperative Vector/Matrix Test Failures with -emit-spirv-via-glsl

## Reproduction Summary

Successfully reproduced all 5 test failures mentioned in issue shader-slang/slang#[issue_number]:

### Tests from expected-failure-linux-gpu.txt (2 tests):
- `tests/cooperative-vector/load-store-arbitrary-array-vec.slang (vk)`
- `tests/cooperative-vector/load-store-arbitrary-array.slang (vk)`

### Tests from expected-glsl-failure-github.txt (3 tests):
- `tests/cooperative-vector/matrix-mul-bias-structuredbuffer-packed.slang (vk)`
- `tests/cooperative-vector/matrix-mul-structuredbuffer-packed.slang (vk)`
- `tests/cooperative-vector/outer-product-structuredbuffer.slang (vk)`

## Test Results

### Without `-emit-spirv-via-glsl` (Direct SPIRV):
```bash
./build/Debug/bin/slang-test tests/cooperative-vector/load-store-arbitrary-array.slang
# Result: PASSED ✓
```

### With `-emit-spirv-via-glsl`:
```bash
./build/Debug/bin/slang-test -emit-spirv-via-glsl tests/cooperative-vector/load-store-arbitrary-array.slang
# Result: FAILED ✗
```

## Error Analysis

**Error Message:**
```
error 36107: entrypoint 'computeMain' uses features that are not available in 'compute' stage for 'glsl' compilation target.
```

**Root Cause:**
The cooperative vector/matrix operations use `[require(spirv, cooperative_vector)]` attribute decorations in `source/slang/hlsl.meta.slang`. When compiling with `-emit-spirv-via-glsl`, the compiler treats the target as `glsl` instead of `spirv`, causing the capability check to fail.

### Affected Functions in hlsl.meta.slang:

1. **Groupshared array operations** (lines 28481-28505):
   - `CoopVec::storeAny<U, let M : int>` - Store to groupshared array of any type
   - `CoopVec::storeAny<U, let M : int, let L : int>` - Store to groupshared vector array
   - `CoopVec::loadAny<U, let M : int>` - Load from groupshared array of any type
   - `CoopVec::loadAny<U, let M : int, let L : int>` - Load from groupshared vector array

2. **Buffer-based matrix operations** (lines 30838-31011):
   - `__coopVecMatMulPacked_impl` - Internal matrix multiplication with packed inputs
   - `coopVecMatMulPacked` - Matrix multiplication with packed inputs
   - `coopVecMatMul` - Matrix multiplication with non-packed inputs
   - `__coopVecMatMulAddPacked_impl` - Matrix multiply-add with packed inputs
   - `coopVecMatMulAddPacked` - Matrix multiply-add with packed inputs
   - `coopVecMatMulAdd` - Matrix multiply-add with non-packed inputs

3. **Structured buffer operations** (lines 31053-31095):
   - `coopVecOuterProductAccumulate` (StructuredBuffer overload)
   - `coopVecReduceSumAccumulate` (StructuredBuffer overload)

4. **Pointer-based operations** (lines 29918-31315):
   - `coopVecLoad<T*>` - Load from pointer
   - Various pointer-based matrix operations
   - Pointer-based outer product operations

## Technical Limitations

### Why These Functions Can't Work with GLSL Emission:

1. **SPIRV Assembly Direct Usage:**
   - Functions use `spirv_asm { ... }` blocks that directly emit SPIRV instructions
   - Examples: `OpCooperativeVectorLoadNV`, `OpCooperativeVectorStoreNV`, `OpCooperativeVectorMatrixMulNV`
   - These can only be generated in direct SPIRV emission mode

2. **Pointer Operations:**
   - Functions like `__getStructuredBufferPtr()` are SPIRV-specific
   - GLSL doesn't have equivalent pointer semantics for these operations

3. **Target Switch Limitations:**
   - Functions use `__target_switch { case spirv: ... }` without GLSL cases
   - When target is `glsl`, there's no code generation path defined

## Investigation Conclusions

This is **NOT** a driver-specific issue with NVIDIA driver 580+. Instead, it's a fundamental limitation of the current implementation:

### Issue Classification:
**Missing GLSL code generation support for cooperative operations**

### Why Not Driver-Related:
- Error occurs during compilation, not execution
- Error is capability checking failure (error 36107), not runtime failure
- Direct SPIRV emission works fine on the same driver

### Why Not glslang Translation Issue:
- glslang never receives the code because Slang's capability checking prevents compilation
- The issue is in Slang's frontend, not the glslang backend

### Current Status:
- Tests are correctly listed in `tests/expected-failure-via-glsl.txt`
- This is an **expected limitation** of the GLSL emission path
- The failure is by design until GLSL code generation support is added

## Potential Solutions (Not Implemented)

To support these operations through GLSL, one would need to:

1. **Add GLSL extension declarations:**
   - `GL_NV_cooperative_vector` and `GL_NV_cooperative_vector_training`
   - Ensure glslang recognizes and translates these to SPIRV

2. **Add GLSL code generation paths:**
   - Implement `case glsl:` in `__target_switch` blocks
   - Use GLSL builtin functions instead of `spirv_asm`

3. **Update capability requirements:**
   - Change `[require(spirv, ...)]` to `[require(glsl_spirv, ...)]` where appropriate
   - Only for functions that have proper GLSL code generation

4. **Complex workaround:**
   - This would require significant changes to support GLSL builtins for cooperative operations
   - May not be worthwhile if direct SPIRV emission works fine

## Recommendation

These tests should **remain** in the expected failure list for GLSL emission path. The cooperative vector/matrix operations are correctly working through direct SPIRV emission, which is the primary path for Vulkan targets.
