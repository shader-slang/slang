// Test differentiable interface inheritance: IDerived extends IBase,
// dispatch through IBase reference for types that conform to IDerived.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
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

// Note: Direct dispatch through IDerived existential triggers ICE (#10775).
// See dynamic-dispatch-interface-inherit-derived.slang for the
// disabled regression test covering that path.

//TEST_INPUT: type_conformance A:IBase = 0
//TEST_INPUT: type_conformance B:IBase = 1

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

    // Forward diff: A.baseCalc at x=3, param=2: f'(3) = 4*3 = 12.0
    outputBuffer[2] = fwd_diff(dispatchBase)(0, DifferentialPair<float>(3.0, 1.0), 2.0).d;
    // CHECK: 12.0

    // Forward diff: B.baseCalc at x=2, param=3: f'(2) = 3.0
    outputBuffer[3] = fwd_diff(dispatchBase)(1, DifferentialPair<float>(2.0, 1.0), 3.0).d;
    // CHECK: 3.0
}
