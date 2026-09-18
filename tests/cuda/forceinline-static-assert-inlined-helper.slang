// A `static_assert` that lives inside an always-inlined helper (a `constexpr`-parameter function,
// which the compiler force-inlines rather than defers) must still fold on CUDA after the helper is
// cloned into its caller. The whole-module scan marks the helper's original `predicate(k)` call, but
// inlining the helper clones that call as a fresh inst; the clone must inherit the requirement, or it
// is deferred to NVRTC and the condition fails to fold (spurious E41402). This is the ordering-
// sensitive case: an unrelated `[ForceInline]` runtime call is considered first, and its CUDA
// deferral check triggers the lazy scan, before the helper is cloned. Regression for
// shader-slang/slang#12623.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

RWStructuredBuffer<int> outputBuffer;

[ForceInline]
int runtimeHint(int x)
{
    return x + 1;
}

[ForceInline]
bool predicate(int x)
{
    return x > 0;
}

// `constexpr int k` makes this a compiler-mandated inline, so it is cloned into `caller` on CUDA
// (unlike a user `[ForceInline]`, which would be deferred and never cloned).
[ForceInline]
int outer(int v, constexpr int k)
{
    static_assert(predicate(k), "k must be positive");
    return v + k;
}

int caller<let K : int>(int v)
{
    int seeded = runtimeHint(v); // Triggers the lazy static-assert scan before `outer` is cloned.
    return outer(seeded, K);
}

// Compiles (the cloned `predicate(k)` still folds), and the unrelated runtime helper stays deferred.
// CHECK-DAG: __forceinline__ __device__ {{.*}}runtimeHint
// CHECK: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = caller<4>(int(tid.x));
}
