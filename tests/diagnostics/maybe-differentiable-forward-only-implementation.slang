//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Reverse mode through an interface cannot use an implementation that is only forward
// differentiable, as for a concrete call.

interface IMaybe
{
    [MaybeDifferentiable]
    float compute(float x);
}

struct ForwardOnly : IMaybe
{
    [ForwardDifferentiable]
    float compute(float x) { return x * x; }
}

[Differentiable]
float use<T : IMaybe>(no_diff T t, float x)
{
    return t.compute(x) + x;
//CHECK:            ^ derivative cannot be propagated through non-differentiable call
//CHECK:            ^ derivative cannot be propagated through call to non-backward-differentiable function `ForwardOnly.compute`, use 'no_diff' to clarify intention.
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    ForwardOnly f;
    var p = diffPair(3.0);
    __bwd_diff(use<ForwardOnly>)(f, p, 1.0);
    outputBuffer[0] = p.d;
}
