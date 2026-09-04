//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// The inner loop's bound is carried in through a loop-local phi (`n`) seeded from the outer
// index, rather than referencing the outer index directly in the exit test. Detecting the
// dependency requires following phi incoming values, not just direct operands; otherwise the
// inner loop is attempted at an unknown bound and peels to the iteration limit. Unrolling the
// outer loop substitutes a literal for `i` (its `i = 0; ++i` start and step are compile-time),
// hence for the `n` seed, and both loops unroll.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int sum = 0;
    [ForceUnroll]
    for (int i = 0; i < 4; ++i)
    {
        [ForceUnroll]
        for (int j = 0, n = i; j < n; ++j, --n)
        {
            sum += i * 4 + j;
        }
    }
    outputBuffer[0] = sum;
}

// CHECK: void computeMain
// CHECK-NOT: for(
// CHECK-NOT: for (
