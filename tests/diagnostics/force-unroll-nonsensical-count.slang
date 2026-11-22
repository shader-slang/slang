//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

// A `[ForceUnroll]` with a nonsensical negative count. The frontend does not range-check the
// count, so the unroll pass must not loop forever on a negative attempt budget: normalization
// clamps the count to a one-attempt budget, so the loop is peeled once, reported as un-unrollable,
// and compilation ends quickly rather than hanging.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    [ForceUnroll(-2)]
    for (int i = 0; i < 1; ++i)
/*CHECK:
    ^^^ E40020
    ^^^ loop does not terminate within the limited number of iterations, unrolling is aborted.
*/
        outputBuffer[0] = i;
}
