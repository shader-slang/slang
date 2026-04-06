// Test backward differentiation through existential dispatch using CPU target.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

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
        // f(x) = a * x^2, df/dx = 2*a*x
        return a * x * x;
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
        // f(x) = b * x^3, df/dx = 3*b*x^2
        return b * x * x * x;
    }
}

[Differentiable]
float run(IFoo obj, float x)
{
    return obj.calc(x);
}

[Differentiable]
float dispatch(uint id, float x, no_diff float param)
{
    IFoo obj;
    if (id == 0)
        obj = no_diff(A(param));
    else
        obj = no_diff(B(param));
    return run(obj, x);
}

//TEST_INPUT: type_conformance A:IFoo = 0
//TEST_INPUT: type_conformance B:IFoo = 1

[numthreads(1, 1, 1)]
void computeMain()
{
    // A: f(x) = 2.0 * x^2, df/dx = 2*2.0*x = 4*x
    // At x=3.0: df/dx = 12.0
    {
        var p = diffPair(3.0);
        bwd_diff(dispatch)(0, p, 2.0, 1.0);
        outputBuffer[0] = p.d;
    }
    // CHECK: 12.0

    // B: f(x) = 1.5 * x^3, df/dx = 3*1.5*x^2 = 4.5*x^2
    // At x=2.0: df/dx = 18.0
    {
        var p = diffPair(2.0);
        bwd_diff(dispatch)(1, p, 1.5, 1.0);
        outputBuffer[1] = p.d;
    }
    // CHECK: 18.0
}
