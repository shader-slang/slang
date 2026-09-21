// A `[ForceInline]` function that also carries a `[__target_intrinsic]` keeps its existing handling
// on CUDA instead of being deferred: `ForceInliningPass::shouldInline` excludes target intrinsics.
// A target intrinsic may have a fallback body, so such a function reaches `shouldInline` with a
// body; when the user force-inline is resolved in Slang, that fallback body is inlined at the call
// site. See shader-slang/slang#12623.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

RWStructuredBuffer<float> outputBuffer;

[ForceInline]
__target_intrinsic(cuda, "myIntrin($0)")
float ti(float x)
{
    return x * 2.0f;
}

// CHECK-NOT: __forceinline__ __device__{{.*}}ti
// CHECK: void computeMain
// CHECK: 2.0f{{.*}}2.0f
// CHECK-NOT: myIntrin

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = ti(float(tid.x)) + ti(outputBuffer[tid.x]);
}
