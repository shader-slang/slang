// A declaration-scope `static_assert` whose condition does not fold to a compile-time constant is
// reported as E41402, rather than crashing or silently passing. Here the condition is a
// stage-dependent query (`__targetHasImplicitDerivatives`), which has no entry-point stage to
// resolve against at module scope, so it survives specialization unfolded; the synthesized carrier
// still gives it a block to lower into, and the post-specialization check finds it did not fold.
// `non-exhaustive` because the query also drives a stage note; the ordered check below is the point.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry computeMain -stage compute

static_assert(__targetHasImplicitDerivatives());
//CHECK: static assertion condition not compile-time constant

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
