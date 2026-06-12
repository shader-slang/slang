//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -capability abort -skip-spirv-validation

// Document the current reverse-mode autodiff behavior for abort() in a
// differentiable function.

[Differentiable]
float f(float x)
{
    if (x > 1.0e6f)
        abort("input too large: %f", x);
//CHECK:     ^ abort is not supported in reverse-mode automatic differentiation
//CHECK:     ^ 'abort' cannot currently be used in a function being reverse-differentiated.
    return x * x;
}

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    var p = diffPair(2.0f, 0.0f);
    bwd_diff(f)(p, 1.0f);
    outBuf[0] = p.d;
}
