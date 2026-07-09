// Test no_diff on an existential-typed parameter in a differentiable function.
// The no_diff on the IFoo parameter means the interface dispatch itself is not differentiated,
// but the float parameter x should still get its derivative.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
interface IFoo
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
    float calc(float x) { return a * x * x; } // df/dx = 2*a*x
}

// The interface parameter is explicitly no_diff.
[Differentiable]
float runWithNoDiffObj(no_diff IFoo obj, float x)
{
    return obj.calc(x);
}

[Differentiable]
float dispatch(float x, no_diff float param)
{
    IFoo obj = no_diff(A(param));
    return runWithNoDiffObj(obj, x);
}

//TEST_INPUT: type_conformance A:IFoo = 0

[numthreads(1, 1, 1)]
void computeMain()
{
    // f(x) = 2*x^2, df/dx = 4*x. At x=3 with seed=1: 12.0
    {
        var p = diffPair(3.0);
        bwd_diff(dispatch)(p, 2.0, 1.0);
        outputBuffer[0] = p.d;
    }
    // CHECK: 12.0

    // Forward diff test too
    outputBuffer[1] = fwd_diff(dispatch)(DifferentialPair<float>(3.0, 1.0), 2.0).d;
    // CHECK: 12.0
}
