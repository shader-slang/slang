// A `static_assert` whose condition uses short-circuit `&&` / `||` must still fold on CUDA under
// user `[ForceInline]`. Short-circuit `&&`/`||` lowers to control flow — `a(K) && b(K)` becomes
// `ifElse(a(K), then, else, join)` where `then` branches `join(b(K))`, `else` branches
// `join(false)`, and the condition is `join`'s phi parameter. The force-inline pass must follow the
// phi's incoming values (to reach `b(K)`) and the controlling branch condition (to reach `a(K)`);
// otherwise those calls are deferred to NVRTC and the condition never folds, giving a spurious
// E41402. See shader-slang/slang#12623.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

RWStructuredBuffer<int> outputBuffer;

[ForceInline]
bool isPositive(int x)
{
    return x > 0;
}

[ForceInline]
bool isSmall(int x)
{
    return x < 100;
}

int genAnd<let K : int>(int v)
{
    static_assert(isPositive(K) && isSmall(K), "K must be positive and small");
    return v + K;
}

int genOr<let K : int>(int v)
{
    static_assert(isPositive(K) || isSmall(K), "K must be positive or small");
    return v + K;
}

// Both conditions fold (compile succeeds), and the helpers stay deferred as separate
// `__forceinline__ __device__` functions at their ordinary runtime call sites.
// CHECK-DAG: __forceinline__ __device__ {{.*}}isPositive
// CHECK-DAG: __forceinline__ __device__ {{.*}}isSmall
// CHECK: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int r = genAnd<3>(int(tid.x)) + genOr<3>(int(tid.x));
    r += (isPositive(int(tid.x)) ? 1 : 0) + (isSmall(int(tid.x)) ? 1 : 0);
    outputBuffer[tid.x] = r;
}
