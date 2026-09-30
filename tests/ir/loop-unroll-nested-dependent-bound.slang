//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// Regression test for shader-slang/slang#12473: an inner `[ForceUnroll]` loop whose bound
// depends on the enclosing `[ForceUnroll]` loop's induction variable.
//
// Loops are collected inner-first, so the `j` loop is reached while `i` is still a
// loop-carried phi param and its trip count is unknowable. It must be deferred until the
// `i` loop is unrolled, which substitutes a literal for `i` in each cloned body (its
// `i = 0; ++i` start and step are compile-time); only then can the `j` loop unroll. Both
// loops must therefore disappear from the emit.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int sum = 0;
    [ForceUnroll]
    for (int i = 0; i < 4; ++i)
    {
        [ForceUnroll]
        for (int j = 0; j < i; ++j)
        {
            sum += i * 4 + j;
        }
    }
    outputBuffer[0] = sum;
}

// Both loops must be gone from the emitted HLSL: the outer one because it is
// `[ForceUnroll]`, the inner one because deferring it let it unroll after the outer did.
// CHECK: void computeMain
// CHECK-NOT: for(
// CHECK-NOT: for (
