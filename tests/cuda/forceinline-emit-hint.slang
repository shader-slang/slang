// On CUDA, a user `[ForceInline]` is emitted as a separate `__forceinline__ __device__` function
// (letting NVRTC do the inlining) rather than being pre-inlined by Slang's own inliner, which makes
// NVRTC's front-end time blow up on large bodies. See shader-slang/slang#12623.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -profile cs_6_0

// On CUDA the helper survives as a separate device function carrying the `__forceinline__` hint,
// and the kernel calls it rather than inlining it.
// CUDA: __forceinline__ __device__ {{.*}}forceInlinedHelper
// CUDA: computeMain
// CUDA: forceInlinedHelper

// On non-CUDA targets (HLSL) behavior is unchanged: Slang still inlines the helper away, so no
// separate function definition appears.
// HLSL-NOT: forceInlinedHelper

RWStructuredBuffer<float> outputBuffer;

[ForceInline]
float forceInlinedHelper(float x)
{
    float y = x * x + 1.0f;
    return y * 2.0f - x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = forceInlinedHelper(outputBuffer[tid.x]);
}
