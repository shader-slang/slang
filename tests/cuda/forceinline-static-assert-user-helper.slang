// A user `[ForceInline]` helper called inside a `static_assert` condition must still be inlined by
// Slang on CUDA, not deferred to NVRTC: the static-assert checker needs the condition folded to a
// compile-time constant, which happens only if the helper is inlined first. A function reachable
// from a `static_assert` condition must never be deferred (shader-slang/slang#12623).

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

[ForceInline]
bool isPositive(int x)
{
    return x > 0;
}

int scaledBy<let K : int>(int v)
{
    // `isPositive(K)` must fold to a compile-time constant here; it does so only if `isPositive`
    // is inlined during the force-inline pass, before the static-assert check runs.
    static_assert(isPositive(K), "K must be positive");
    return v * K;
}

// The compile must succeed (the static_assert holds after folding). The kernel is emitted.
// CHECK: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = scaledBy<3>(int(tid.x));
}
