// A user `[ForceInline]` lowers to `IRUserForceInlineDecoration`, which has its own stable IR id so
// that it survives module serialization (shader-slang/slang#12623). This test imports the helper
// from a precompiled .slang-module rather than from source: if serialization dropped the marker or
// read it back as the generic compiler-required `IRForceInlineDecoration`, CUDA would pre-inline
// the helper instead of emitting it as a separate `__forceinline__ __device__` function.

//TEST:COMPILE: tests/cuda/forceinline-module-helper.slang -o tests/cuda/forceinline-module-helper.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/cuda/forceinline-module-helper.slang-module -target cuda -entry computeMain -stage compute -line-directive-mode none

import forceinline_module_helper;

RWStructuredBuffer<float> outputBuffer;

// CHECK: __forceinline__ __device__ float serializedHelper
// CHECK: computeMain
// CHECK: serializedHelper{{.*}}(

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = serializedHelper(float(tid.x));
}
