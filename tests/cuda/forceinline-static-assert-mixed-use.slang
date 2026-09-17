// A single (non-generic) user `[ForceInline]` helper used BOTH inside a `static_assert` condition
// AND at runtime must be handled per call site: the static_assert use is inlined so the condition
// folds, while the runtime call is still deferred to NVRTC as a separate `__forceinline__
// __device__` function. Keying the never-defer rule on the callee *function* (rather than the call
// site) would inline every call to `classify`, reintroducing the shader-slang/slang#12623 blow-up
// at the runtime site. This is the regression test for that finding.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

RWStructuredBuffer<int> outputBuffer;

[ForceInline]
int classify(int x)
{
    return x > 0 ? 1 : 0;
}

int checkPositive<let K : int>(int v)
{
    // Static-assert use: `classify(K)` is inlined here so the condition folds to a constant.
    static_assert(classify(K) == 1, "K must be positive");
    return v + K;
}

// The runtime `classify(int(tid.x))` call is a distinct call site, so it stays deferred: `classify`
// survives as a `__forceinline__ __device__` function that the kernel calls rather than inlines.
// CHECK: __forceinline__ __device__ {{.*}}classify
// CHECK: computeMain
// CHECK: classify

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = checkPositive<7>(int(tid.x)) + classify(int(tid.x));
}
