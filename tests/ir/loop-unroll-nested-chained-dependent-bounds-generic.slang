//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// The generic-function counterpart of `loop-unroll-nested-chained-dependent-bounds.slang`: three
// chained dependent `[ForceUnroll]` bounds (`j` depends on `k`, `k` on `i`), but nested inside a
// generic function so the unroll runs during specialization rather than on the already-monomorphic
// entry point. This exercises the same cascade — `i` unrolls first, making `k`'s bound a literal,
// then `j`'s — down the generic path, confirming the two-pass driver behaves identically there.

RWStructuredBuffer<int> outputBuffer;

int accumulate<let N : int>()
{
    int sum = 0;
    [ForceUnroll]
    for (int i = 0; i < N; ++i)
    {
        [ForceUnroll]
        for (int k = 0; k < i; ++k)
        {
            [ForceUnroll]
            for (int j = 0; j < k; ++j)
            {
                sum += i * 16 + k * 4 + j;
            }
        }
    }
    return sum;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = accumulate<4>();
}

// All three loops must be gone from the emitted HLSL.
// CHECK: void computeMain
// CHECK-NOT: for(
// CHECK-NOT: for (
