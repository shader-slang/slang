//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// A function that calls `bwd_diff` through an existential receiver cannot be differentiated, as
// for a concrete or generic receiver.

interface ICurve
{
    [Differentiable]
    float eval(float t);
}

struct Cubic : ICurve
{
    [Differentiable]
    [NoDiffThis]
    float eval(float t) { return t * t * t; }
}

[Differentiable]
float gradient(no_diff ICurve c, float t)
{
    var p = diffPair(t);
    __bwd_diff(c.eval)(p, 1.0);
//CHECK:              ^ cannot differentiate the result of a backward-derivative call
//CHECK:              ^ the code produced by 'bwd_diff' is not itself differentiable, so a function that calls 'bwd_diff' cannot be differentiated; for higher-order derivatives, nest 'fwd_diff' calls or apply a single 'bwd_diff' to a function that uses 'fwd_diff'.
    return p.d;
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Cubic c;
    outputBuffer[0] = __fwd_diff(gradient)(c, diffPair(2.0, 1.0)).d;
}
