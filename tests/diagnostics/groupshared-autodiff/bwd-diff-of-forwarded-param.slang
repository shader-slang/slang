// A non-differentiable function that receives thread-group-shared memory as a `groupshared`
// parameter and passes it to a writable reference of `bwd_diff(f)` has the derivative of `f`
// copy it and store the copy back. We report the `bwd_diff` call, even though the calling
// function is not differentiated.

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
//CHECK:        ^ passing thread-group-shared memory to a writable reference parameter of 'f' introduces a race condition when 'f' is differentiated
//CHECK:        ^ give 'f' a custom derivative, or move the access into a function that is not differentiable
    return x.d;
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    outputBuffer[i] = g(gs[i]);
}
