// Test differentiating `this` of an existential type WITHOUT [NoDiffThis].
// The interface extends IDifferentiable, implementations have differentiable fields.
// Gradient should flow to the fields of `this` through the existential.

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

    [Differentiable]
    __init(float a) { this.a = a; }

    [Differentiable]
    float calc(float x)
    {
        // f(a, x) = a * x^2
        // df/da = x^2, df/dx = 2*a*x
        return a * x * x;
    }
}

[Differentiable]
float run(IFoo obj, float x)
{
    return obj.calc(x);
}

// Construct the object inside [Differentiable] context using differentiable
// constructor, allowing gradient to flow through `this`.
[Differentiable]
float f_with_diff_this(float param, float x)
{
    IFoo obj = A(param);
    return run(obj, x);
}

//TEST_INPUT: type_conformance A:IFoo = 0

[numthreads(1, 1, 1)]
void computeMain()
{
    // Differentiate w.r.t. both `param` and `x`.
    // f(param, x) = param * x^2
    // df/dparam = x^2 = 9.0 at x=3
    // df/dx = 2*param*x = 2*2*3 = 12.0
    {
        var pParam = diffPair(2.0);
        var pX = diffPair(3.0);
        bwd_diff(f_with_diff_this)(pParam, pX, 1.0);
        outputBuffer[0] = pParam.d; // df/dparam = x^2 = 9.0
        outputBuffer[1] = pX.d;     // df/dx = 2*param*x = 12.0
    }
    // CHECK: 9.0
    // CHECK: 12.0
}
