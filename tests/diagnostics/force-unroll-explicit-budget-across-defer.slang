//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

// A `[ForceUnroll(1)]` loop promises at most one iteration, but this loop needs two before its
// runtime-seeded header settles (`current` becomes `next`, then `next`'s pinned `false`). Because
// the header does not fold on the first peel, the deferral pass leaves a residual loop; the residual
// carries the loop's *remaining* peel-attempt budget (decremented for the peel already taken), so
// the diagnosing pass resumes that budget rather than restarting from a fresh `[ForceUnroll(1)]`.
// With only one iteration allowed and two required, the loop cannot be unrolled and must be
// reported — exactly as it is when nothing is deferred.
//
// (Contrast `loop-unroll-nested-convergent-runtime-state.slang`, the same loop with an unbounded
// `[ForceUnroll]`, which is allowed the two iterations it needs and compiles.)

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uniform bool seed0, uniform bool seed1)
{
    int sum = 0;
    bool current = seed0;
    bool next = seed1;
    [ForceUnroll(1)]
    while (current)
/*CHECK:
    ^^^^^ E40020
    ^^^^^ loop does not terminate within the limited number of iterations, unrolling is aborted.
*/
    {
        sum++;
        current = next;
        next = false;
    }
    outputBuffer[0] = sum;
}
