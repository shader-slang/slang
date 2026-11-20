// Tests automatic synthesis of Differential type and method requirements.

//TEST:COMPARE_COMPUTE(targets=all):-output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IFoo
{
    [Differentiable]
    float f(float v);
}

struct B : IFoo
{
    uint factor;
    [TreatAsDifferentiable]
    float f(float v)
    {
        return factor * v;
    }
}

[Differentiable]
float use(IFoo o, float x)
{
    return o.f(x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    B b = {2};
    var p = diffPair(1.0);
    __bwd_diff(use)(b, p, 1.0);
    outputBuffer[0] = p.d; // Expect 0.0, since [TreatAsDifferentiable] create trivial derivative implementations.
}
