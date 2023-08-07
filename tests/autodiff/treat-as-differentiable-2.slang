// Tests automatic synthesis of Differential type and method requirements.

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IFoo
{
    [Differentiable]
    float f(float v);
}

struct B : IFoo
{
    [TreatAsDifferentiable]
    float f(float v)
    {
        return v * v;
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
    B b;
    var p = diffPair(1.0, 1.0);
    var dpx = __fwd_diff(use)(b, p);
    outputBuffer[0] = dpx.p;
    outputBuffer[1] = dpx.d;
}
