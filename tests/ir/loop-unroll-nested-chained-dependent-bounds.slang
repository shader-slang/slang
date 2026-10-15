//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// Three `[ForceUnroll]` loops with chained dependent bounds: the innermost `j` depends on the
// middle `k`, which depends on the outermost `i`. Unrolling has to cascade — `i` unrolls first,
// making `k`'s bound a literal so `k` unrolls, making `j`'s bound a literal so `j` unrolls.
// The diagnosing pass re-collects after each unroll, so the `k` and `j` clones that only come
// into existence once an enclosing loop is unrolled are still found and unrolled; all three
// loops disappear from the emit.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int sum = 0;
    [ForceUnroll]
    for (int i = 0; i < 4; ++i)
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
    outputBuffer[0] = sum;
}

// CHECK: void computeMain
// CHECK-NOT: for(
// CHECK-NOT: for (
