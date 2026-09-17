// The CUDA counterpart of tests/autodiff/forceinline-diff-func-still-inlined.slang. On CUDA a
// `[ForceInline] [Differentiable]` function is deferred to NVRTC (shader-slang/slang#12623), and
// its synthesized derivative must carry the hint too: `copyOriginalDecorations` clones the user
// force-inline marker onto the derivative, so both the primal and the forward-derivative survive as
// separate `__forceinline__ __device__` functions rather than being pre-inlined by Slang.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

RWStructuredBuffer<float> outputBuffer;

[ForceInline]
[Differentiable]
float square(float x)
{
    return x * x;
}

// Both the primal `square` and its forward-derivative survive as deferred `__forceinline__
// __device__` functions carrying the hint.
// CHECK-DAG: __forceinline__ __device__ {{.*}}square
// CHECK-DAG: __forceinline__ __device__ {{.*}}s_fwd_square
// CHECK: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    var dp = diffPair(float(tid.x), 1.0f);
    var result = fwd_diff(square)(dp);
    outputBuffer[tid.x] = result.d + square(float(tid.x));
}
