// Regression test for derived-interface existentials in differentiable code.
// This used to ICE while lowering DifferentialPair<IDerived>.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -Xslang -Wno-41016
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -Xslang -Wno-41016
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -Xslang -Wno-41016

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
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
    float baseCalc(float x) { return a * x * x; }

    [Differentiable]
    [NoDiffThis]
    float derivedCalc(float x) { return a * x * x * x; }
}

struct B : IDerived
{
    float b;

    __init(float b) { this.b = b; }

    [Differentiable]
    [NoDiffThis]
    float baseCalc(float x) { return b * x; }

    [Differentiable]
    [NoDiffThis]
    float derivedCalc(float x) { return b * x * x; }
}

[Differentiable]
float useDerived(IDerived obj, float x)
{
    return obj.derivedCalc(x);
}

[Differentiable]
float dispatchDerived(uint id, float x, no_diff float paramA, no_diff float paramB)
{
    // Keep the derived-interface existential as a local so lowering must form
    // DifferentialPair<IDerived> for the differentiable function body.
    IDerived obj = no_diff(A(paramA));
    if (id != 0)
        obj = no_diff(B(paramB));
    return useDerived(obj, x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var p = diffPair(3.0f);
    bwd_diff(dispatchDerived)(dispatchThreadID.x, p, 0.5f, 2.0f, 1.0f);
    outputBuffer[0] = p.d; // A.derivedCalc, expect 13.5

    p = diffPair(3.0f);
    bwd_diff(dispatchDerived)(dispatchThreadID.x + 1, p, 0.5f, 2.0f, 1.0f);
    outputBuffer[1] = p.d; // B.derivedCalc, expect 12

    // CHECK: 13.5
    // CHECK: 12.0
}
