// A non-differentiable function that receives thread-group-shared memory as a `groupshared`
// parameter and passes it to a writable reference of `bwd_diff(f)` lets the differentiated `f`
// write it, which we cannot check from here. We warn at the `bwd_diff` call, even though the
// calling function is not differentiated.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

RWStructuredBuffer<float> outputBuffer;
groupshared float gs[4];

[Differentiable]
float f(no_diff __ref float a, float x)
{
    a = a + 1;
    return a * x;
}

float g(groupshared float a)
{
    var x = diffPair(2.0);
    bwd_diff(f)(a, x, 1.0);
//CHECK:        ^ passing thread-group-shared memory by reference introduces a race condition when this call is differentiated
//CHECK:        ^ access the groupshared variable directly as a global in the callee instead of passing it as an argument
    return x.d;
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    outputBuffer[i] = g(gs[i]);
}
