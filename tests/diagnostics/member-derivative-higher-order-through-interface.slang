//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// A member's derivative reached through an interface has no derivative of its own, so
// differentiating a function that uses it is rejected rather than treated as zero.

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
float slope<C : ICurve>(no_diff C c, float t)
{
    return __fwd_diff(c.eval)(diffPair(t, 1.0)).d;
//CHECK:                     ^ cannot propagate through non-differentiable function
//CHECK:                     ^ encountered non-differentiable function 'Cubic.eval.fwd_diff' during higher-order differentiation
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Cubic c;
    outputBuffer[0] = __fwd_diff(slope<Cubic>)(c, diffPair(3.0, 1.0)).d;
}
