// A generic user `[ForceInline]` helper specialized two different ways — `pick<int>` inside a
// `static_assert` and `pick<float>` at runtime — must keep the runtime specialization deferred.
// `specializeModule` runs before the force-inline pass, so `pick<int>` and `pick<float>` are
// already distinct `IRFunc`s by the time deferral is decided: the static_assert use of `pick<int>`
// is inlined to fold the condition, while the runtime `pick<float>` survives as a
// `__forceinline__ __device__` function. This confirms specialization separates the instances
// (shader-slang/slang#12623).

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

RWStructuredBuffer<float> outputBuffer;

[ForceInline]
T pick<T>(T a, T b)
{
    return a;
}

int checkGeneric<let K : int>(int v)
{
    static_assert(pick<int>(K, 0) == K, "pick returns its first argument");
    return v + K;
}

// The runtime `pick<float>` specialization is deferred and emitted as a separate function.
// CHECK: __forceinline__ __device__ {{.*}}pick
// CHECK: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = float(checkGeneric<4>(int(tid.x))) + pick<float>(float(tid.x), 1.0f);
}
