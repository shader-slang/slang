//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// The inner loop has no header bound at all — it terminates only through `if (j >= i) break`,
// an exit test whose condition depends on the outer index and is routed to the loop's break
// block through an intermediate block. Detecting it as an exit requires reachability to the
// break block, not a direct branch to it. Deferring until the outer loop unrolls substitutes a
// literal for `i` here (its `i = 0; ++i` start and step are compile-time), and both loops
// unroll.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int sum = 0;
    [ForceUnroll]
    for (int i = 0; i < 4; ++i)
    {
        [ForceUnroll]
        for (int j = 0;; ++j)
        {
            if (j >= i)
                break;
            sum += i * 4 + j;
        }
    }
    outputBuffer[0] = sum;
}

// CHECK: void computeMain
// CHECK-NOT: for(
// CHECK-NOT: for (
