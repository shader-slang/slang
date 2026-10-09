//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// A second derivative through a generic interface call needs the derivative of the method's
// forward derivative, which an interface does not provide. It is rejected rather than computed as
// zero (here the true value is 4 * 2 = 8).

interface IScalable
{
    [Differentiable]
    This scale2(float s);
}

struct F : IScalable, IDifferentiable
{
    float v;

    [Differentiable]
    F scale2(float s)
    {
        F r;
        r.v = v * s;
        return r;
    }
}

[Differentiable]
T applyScale<T : IScalable>(T x)
{
    return x.scale2(2.0);
//CHECK:           ^ cannot propagate through non-differentiable function
//CHECK:           ^ encountered non-differentiable function 'F.$__syn_scale2.fwd_diff' during higher-order differentiation
}

// `q(x) = 2 x^2`, so `slope(x) = 4 x`.
[Differentiable]
float q(float x)
{
    F f;
    f.v = x;
    return applyScale<F>(f).v * x;
}

[Differentiable]
float slope(float x)
{
    return __fwd_diff(q)(diffPair(x, 1.0)).d;
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = __fwd_diff(slope)(diffPair(1.5, 2.0)).d;
}
