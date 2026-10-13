//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// Regression test for shader-slang/slang#12473, skip-a-level variant: the inner
// `[ForceUnroll]` loop's bound depends on the OUTERMOST loop's induction variable, with an
// unrelated `[ForceUnroll]` loop in between. Deferral must consider every enclosing loop, not
// just the immediate one, or the inner loop is attempted while `i` is still loop-carried and
// peels to the iteration limit.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int sum = 0;
    [ForceUnroll]
    for (int i = 0; i < 4; ++i)
    {
        [ForceUnroll]
        for (int k = 0; k < 2; ++k)
        {
            [ForceUnroll]
            for (int j = 0; j < i; ++j)
            {
                sum += i * 4 + j + k;
            }
        }
    }
    outputBuffer[0] = sum;
}

// CHECK: void computeMain
// CHECK-NOT: for(
// CHECK-NOT: for (
