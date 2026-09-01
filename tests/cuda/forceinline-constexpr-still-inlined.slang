// A user `[ForceInline]` on a function with a `constexpr` parameter must still be inlined by Slang
// on CUDA, NOT deferred to NVRTC: the constexpr value must reach the body (it cannot survive to
// emit as a runtime parameter). The user hint co-exists with a compiler correctness-inline here, so
// the CUDA deferral must not fire. See shader-slang/slang#12623.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute

// The helper is inlined into the kernel (no separate `__forceinline__ __device__` definition),
// so the constexpr offset is materialized rather than passed as a runtime argument.
// CUDA-NOT: __forceinline__

RWStructuredBuffer<int> outputBuffer;

[ForceInline]
int addConst(int x, constexpr int c)
{
    return x + c;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = addConst(int(tid.x), 7);
}
