// Test interface with multiple differentiable methods dispatched dynamically.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
interface IMulti : IDifferentiable
{
    [Differentiable]
    float calc1(float x);

    [Differentiable]
    float calc2(float x);
}

struct A : IMulti
{
    float a;

    __init(float a) { this.a = a; }

    [Differentiable]
    [NoDiffThis]
    float calc1(float x) { return a * x * x; }       // df/dx = 2*a*x

    [Differentiable]
    [NoDiffThis]
    float calc2(float x) { return a * x * x * x; }   // df/dx = 3*a*x^2
}

struct B : IMulti
{
    float b;

    __init(float b) { this.b = b; }

    [Differentiable]
    [NoDiffThis]
    float calc1(float x) { return b * x; }            // df/dx = b

    [Differentiable]
    [NoDiffThis]
    float calc2(float x) { return b * x * x; }        // df/dx = 2*b*x
}

[Differentiable]
float useCalc1(IMulti obj, float x)
{
    return obj.calc1(x);
}

[Differentiable]
float useCalc2(IMulti obj, float x)
{
    return obj.calc2(x);
}

[Differentiable]
float dispatchCalc1(uint id, float x, no_diff float param)
{
    IMulti obj;
    if (id == 0)
        obj = no_diff(A(param));
    else
        obj = no_diff(B(param));
    return useCalc1(obj, x);
}

[Differentiable]
float dispatchCalc2(uint id, float x, no_diff float param)
{
    IMulti obj;
    if (id == 0)
        obj = no_diff(A(param));
    else
        obj = no_diff(B(param));
    return useCalc2(obj, x);
}

//TEST_INPUT: type_conformance A:IMulti = 0
//TEST_INPUT: type_conformance B:IMulti = 1

[numthreads(1, 1, 1)]
void computeMain()
{
    // A.calc1: f(x) = 2*x^2, df/dx = 4*x. At x=3: 12.0
    {
        var p = diffPair(3.0);
        bwd_diff(dispatchCalc1)(0, p, 2.0, 1.0);
        outputBuffer[0] = p.d;
    }
    // CHECK: 12.0

    // A.calc2: f(x) = 2*x^3, df/dx = 6*x^2. At x=3: 54.0
    {
        var p = diffPair(3.0);
        bwd_diff(dispatchCalc2)(0, p, 2.0, 1.0);
        outputBuffer[1] = p.d;
    }
    // CHECK: 54.0

    // B.calc1: f(x) = 3*x, df/dx = 3. At x=2: 3.0
    {
        var p = diffPair(2.0);
        bwd_diff(dispatchCalc1)(1, p, 3.0, 1.0);
        outputBuffer[2] = p.d;
    }
    // CHECK: 3.0

    // B.calc2: f(x) = 3*x^2, df/dx = 6*x. At x=2: 12.0
    {
        var p = diffPair(2.0);
        bwd_diff(dispatchCalc2)(1, p, 3.0, 1.0);
        outputBuffer[3] = p.d;
    }
    // CHECK: 12.0
}
