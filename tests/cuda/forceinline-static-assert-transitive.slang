// A transitive chain of user `[ForceInline]` helpers reached from a `static_assert` condition must
// still fold on CUDA: `static_assert(outer(K) ...)` where `outer` calls `inner` requires *both* to
// be inlined at the condition, even though only the `outer(K)` call appears in the condition itself
// (the `inner` call lives inside `outer`'s body and only exists at the condition site once `outer`
// is inlined). At the same time, ordinary runtime calls to the same helpers must stay deferred to
// NVRTC — the whole point of shader-slang/slang#12623. This exercises the call-site keying plus the
// clone-marking that carries the requirement to the freshly-inlined `inner` call.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

RWStructuredBuffer<int> outputBuffer;

[ForceInline]
int inner(int x)
{
    return x + 1;
}

[ForceInline]
int outer(int x)
{
    return inner(x) * 2;
}

int gen<let K : int>(int v)
{
    // Folds only if `outer(K)` AND the `inner(K)` call it hides both inline before the
    // static-assert check runs.
    static_assert(outer(K) == (K + 1) * 2, "chain must fold to a constant");
    return v + K;
}

// The kernel also calls `outer` at runtime, so both helpers survive as separate deferred
// `__forceinline__ __device__` functions (the runtime `outer` call is a distinct call site from the
// one the static_assert consumed, so it is not inlined).
// CHECK-DAG: __forceinline__ __device__ {{.*}}inner
// CHECK-DAG: __forceinline__ __device__ {{.*}}outer
// CHECK: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = gen<3>(int(tid.x)) + outer(int(tid.x));
}
