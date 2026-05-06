// Test first-order forward differentiation through existential dispatch.
// The higher-order case (fwd_diff of fwd_diff) is tracked in issue #10728.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
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
        // f(x) = a*x^3, f'(x) = 3*a*x^2
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
        // f(x) = b*x^4, f'(x) = 4*b*x^3
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

//TEST_INPUT: type_conformance A:IFoo = 0
//TEST_INPUT: type_conformance B:IFoo = 1

[numthreads(1, 1, 1)]
void computeMain()
{
    // A: f(x) = 2*x^3, f'(x) = 6*x^2
    // At x=2: f'(2) = 24
    outputBuffer[0] = fwd_diff(dispatch)(0, DifferentialPair<float>(2.0, 1.0)).d;
    // CHECK: 24.0

    // B: f(x) = 3*x^4, f'(x) = 12*x^3
    // At x=2: f'(2) = 96
    outputBuffer[1] = fwd_diff(dispatch)(1, DifferentialPair<float>(2.0, 1.0)).d;
    // CHECK: 96.0
}
