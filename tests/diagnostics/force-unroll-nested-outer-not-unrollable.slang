//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

// Termination guard for the deferral: the inner `[ForceUnroll]` loop depends on the outer
// loop's index and is deferred, but the outer loop is itself `[ForceUnroll]` with a *runtime*
// bound, so it can never unroll. Compilation must terminate with exactly one diagnostic (the
// outer loop's own E40020), not loop forever waiting for an unroll that never happens and not
// report the inner loop twice. Exhaustive `diag=` mode makes "exactly once" fail by
// construction if a second diagnostic appears.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uniform int n)
{
    int sum = 0;
    [ForceUnroll]
    for (int i = 0; i < n; ++i)
//CHECK: E40020
//CHECK: loop does not terminate within the limited number of iterations, unrolling is aborted.
    {
        [ForceUnroll]
        for (int j = 0; j < i; ++j)
        {
            sum += i * 4 + j;
        }
    }
    outputBuffer[0] = sum;
}
