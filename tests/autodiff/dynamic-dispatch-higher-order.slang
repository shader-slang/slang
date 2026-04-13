// Test higher-order differentiation through existential dispatch:
// fwd_diff(fwd_diff(f)) where f dispatches through an interface.
//
// BUG: Higher-order differentiation through dynamic dispatch causes an
// internal compiler error (E99999 at core:619:11). The first-order
// forward diff works, but computing the second derivative via
// fwd_diff(firstDeriv) triggers the ICE.

// BUG: https://github.com/shader-slang/slang/issues/10728
//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
interface IFoo : IDifferentiable
{
    [Differentiable]
    float calc(float x);
}

struct A : IFoo
{
    float a;

    __init(float a) { this.a = a; }

    [Differentiable]
    [NoDiffThis]
    float calc(float x)
    {
        // f(x) = a*x^3, f'(x) = 3*a*x^2, f''(x) = 6*a*x
        return a * x * x * x;
    }
}

struct B : IFoo
{
    float b;

    __init(float b) { this.b = b; }

    [Differentiable]
    [NoDiffThis]
    float calc(float x)
    {
        // f(x) = b*x^4, f'(x) = 4*b*x^3, f''(x) = 12*b*x^2
        return b * x * x * x * x;
    }
}

[Differentiable]
float run(IFoo obj, float x)
{
    return obj.calc(x);
}

[Differentiable]
float dispatch(uint id, float x)
{
    IFoo obj;
    if (id == 0)
        obj = no_diff(A(2.0));
    else
        obj = no_diff(B(3.0));
    return run(obj, x);
}

// First derivative wrapper
[Differentiable]
float firstDeriv(uint id, float x)
{
    return fwd_diff(dispatch)(id, DifferentialPair<float>(x, 1.0)).d;
}

//TEST_INPUT: type_conformance A:IFoo = 0
//TEST_INPUT: type_conformance B:IFoo = 1

[numthreads(1, 1, 1)]
void computeMain()
{
    // A: f(x) = 2*x^3, f'(x) = 6*x^2
    // At x=2: f'(2) = 24
    outputBuffer[0] = fwd_diff(dispatch)(0, DifferentialPair<float>(2.0, 1.0)).d;
    // CHECK: 24.0

    // Second derivative of A at x=2: f''(x) = 12*x, f''(2) = 24
    outputBuffer[1] = fwd_diff(firstDeriv)(0, DifferentialPair<float>(2.0, 1.0)).d;
    // CHECK: 24.0

    // B: f(x) = 3*x^4, f'(x) = 12*x^3
    // At x=2: f'(2) = 96
    outputBuffer[2] = fwd_diff(dispatch)(1, DifferentialPair<float>(2.0, 1.0)).d;
    // CHECK: 96.0

    // Second derivative of B at x=2: f''(x) = 36*x^2, f''(2) = 144
    outputBuffer[3] = fwd_diff(firstDeriv)(1, DifferentialPair<float>(2.0, 1.0)).d;
    // CHECK: 144.0
}
