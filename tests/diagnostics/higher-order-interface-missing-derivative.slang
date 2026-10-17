//DIAGNOSTIC_TEST:SIMPLE(diag=BWD): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=FWD): -target spirv -DTEST_FORWARD=1

interface ICurve
{
    [Differentiable]
    float eval(float x);
}

struct Cubic : ICurve
{
    [ForwardDerivative(evalFwd)]
    [BackwardDerivative(evalBwd)]
    [NoDiffThis]
    float eval(float x) { return x * x * x; }

    // This first derivative deliberately has no higher-order conformance.
    DifferentialPair<float> evalFwd(DifferentialPair<float> x)
    {
        return diffPair(x.p * x.p * x.p, 3.0 * x.p * x.p * x.d);
    }

    void evalBwd(inout DifferentialPair<float> x, float d)
    {
        x = diffPair(x.p, 3.0 * x.p * x.p * d);
    }
}

[Differentiable]
float evalCurve<C : ICurve>(no_diff C c, float x)
{
#ifdef TEST_FORWARD
    return c.eval(x);
//FWD:           ^ derivative cannot be propagated through non-differentiable call
//FWD:           ^ derivative cannot be propagated through call to non-forward-differentiable function
#else
    return c.eval(x);
//BWD:           ^ derivative cannot be propagated through non-differentiable call
//BWD:           ^ derivative cannot be propagated through call to non-backward-differentiable function
#endif
}

[Differentiable]
float slope<C : ICurve>(no_diff C c, float x)
{
    return __fwd_diff(evalCurve<C>)(c, diffPair(x, 1.0)).d;
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Cubic c;
#ifdef TEST_FORWARD
    outputBuffer[0] = __fwd_diff(slope<Cubic>)(c, diffPair(3.0, 1.0)).d;
#else
    var x = diffPair(3.0);
    __bwd_diff(slope<Cubic>)(c, x, 1.0);
    outputBuffer[0] = x.d;
#endif
}
