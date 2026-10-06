//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

// A `[ForceUnroll]` loop bounded by a value that is not compile-time constant can never be
// unrolled, no matter what else the unroller does first. Deferring nested loops whose bound
// depends on an enclosing loop (shader-slang/slang#12473) must not swallow this case: the
// bound here comes from a uniform, not from an enclosing loop, so there is no progress that
// could ever make it a literal and it must still be reported.
//
// This test is deliberately exhaustive (no `non-exhaustive` option): an unexpected extra
// diagnostic fails by construction, which is what pins "diagnosed exactly once".

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uniform int n)
{
    int sum = 0;
    [ForceUnroll]
    for (int j = 0; j < n; ++j)
/*CHECK:
    ^^^ E40020
    ^^^ loop does not terminate within the limited number of iterations, unrolling is aborted.
*/
    {
        sum += j;
    }
    outputBuffer[0] = sum;
}
