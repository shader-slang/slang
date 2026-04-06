// Test differentiable interface inheritance: IDerived extends IBase,
// dispatch through both IBase and IDerived references.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
interface IBase : IDifferentiable
{
    [Differentiable]
    float baseCalc(float x);
}

[anyValueSize(16)]
interface IDerived : IBase
{
    [Differentiable]
    float derivedCalc(float x);
}

struct A : IDerived
{
    float a;

    __init(float a) { this.a = a; }

    [Differentiable]
    [NoDiffThis]
    float baseCalc(float x) { return a * x * x; }       // df/dx = 2*a*x

    [Differentiable]
    [NoDiffThis]
    float derivedCalc(float x) { return a * x * x * x; } // df/dx = 3*a*x^2
}

struct B : IDerived
{
    float b;

    __init(float b) { this.b = b; }

    [Differentiable]
    [NoDiffThis]
    float baseCalc(float x) { return b * x; }             // df/dx = b

    [Differentiable]
    [NoDiffThis]
    float derivedCalc(float x) { return b * x * x; }      // df/dx = 2*b*x
}

[Differentiable]
float useBase(IBase obj, float x)
{
    return obj.baseCalc(x);
}

[Differentiable]
float dispatchBase(uint id, float x, no_diff float param)
{
    IBase obj;
    if (id == 0)
        obj = no_diff(A(param));
    else
        obj = no_diff(B(param));
    return useBase(obj, x);
}

[Differentiable]
float useDerived(IDerived obj, float x)
{
    return obj.derivedCalc(x);
}

[Differentiable]
float dispatchDerived(uint id, float x, no_diff float param)
{
    IDerived obj;
    if (id == 0)
        obj = no_diff(A(param));
    else
        obj = no_diff(B(param));
    return useDerived(obj, x);
}

//TEST_INPUT: type_conformance A:IBase = 0
//TEST_INPUT: type_conformance B:IBase = 1
//TEST_INPUT: type_conformance A:IDerived = 2
//TEST_INPUT: type_conformance B:IDerived = 3

[numthreads(1, 1, 1)]
void computeMain()
{
    // A.baseCalc: f(x) = 2*x^2, df/dx = 4*x. At x=3: 12.0
    {
        var p = diffPair(3.0);
        bwd_diff(dispatchBase)(0, p, 2.0, 1.0);
        outputBuffer[0] = p.d;
    }
    // CHECK: 12.0

    // B.baseCalc: f(x) = 3*x, df/dx = 3. At x=2: 3.0
    {
        var p = diffPair(2.0);
        bwd_diff(dispatchBase)(1, p, 3.0, 1.0);
        outputBuffer[1] = p.d;
    }
    // CHECK: 3.0

    // A.derivedCalc: f(x) = 2*x^3, df/dx = 6*x^2. At x=3: 54.0
    {
        var p = diffPair(3.0);
        bwd_diff(dispatchDerived)(0, p, 2.0, 1.0);
        outputBuffer[2] = p.d;
    }
    // CHECK: 54.0

    // B.derivedCalc: f(x) = 3*x^2, df/dx = 6*x. At x=2: 12.0
    {
        var p = diffPair(2.0);
        bwd_diff(dispatchDerived)(1, p, 3.0, 1.0);
        outputBuffer[3] = p.d;
    }
    // CHECK: 12.0
}
