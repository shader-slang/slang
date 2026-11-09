//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

// The variant of shader-slang/slang#12473 that the two-pass unroll cannot make succeed: the
// inner loop's bound depends on the outer loop's variable, but the outer loop is not
// `[ForceUnroll]`, so nothing ever unrolls it and `i` never becomes a literal. Pass 1 defers the
// inner loop cheaply; pass 2 attempts it fully, its `j < i` test never folds, and it is reported.
//
// The correct outcome is the unroll-failed diagnostic (rather than success or an infinite loop).
// The diagnosing pass peels up to the iteration limit before giving up, so this is bounded, not a
// hang — but it is not instantaneous, which is expected for a genuinely non-constant bound.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int sum = 0;
    for (int i = 0; i < 4; ++i)
    {
        [ForceUnroll]
        for (int j = 0; j < i; ++j)
/*CHECK:
        ^^^ E40020
        ^^^ loop does not terminate within the limited number of iterations, unrolling is aborted.
*/
        {
            sum += i * 4 + j;
        }
    }
    outputBuffer[0] = sum;
}
