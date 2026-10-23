//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)

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

    // The custom first derivative deliberately has no higher-order derivative.
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
    return no_diff(c.eval(x)) + x * x;
}

[Differentiable]
float slope<C : ICurve>(no_diff C c, float x)
{
    return __fwd_diff(evalCurve<C>)(c, diffPair(x, 1.0)).d;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Cubic c;
    // The clarified call contributes zero at second order; the separate square contributes 2.
    // Keeping that contribution also catches an incorrectly zeroed gradient for the whole function.
    // CHECK: {{^}}2.000000{{$}}
    outputBuffer[0] = __fwd_diff(slope<Cubic>)(c, diffPair(3.0, 1.0)).d;

    var x = diffPair(3.0);
    __bwd_diff(slope<Cubic>)(c, x, 1.0);
    // CHECK-NEXT: {{^}}2.000000{{$}}
    outputBuffer[1] = x.d;
}
