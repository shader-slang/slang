//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Smoke test for function interfaces on specialized generic functions

interface IFoo
{
    static float bar(float a);
}

T zap<T : IFloat>(T x)
{
    return (T)2 * x;
}

extension __func_as_type(zap<float3>) : IFoo
{
    static float bar(float x)
    {
        return x * x;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    // This should just work:
    outputBuffer[tig] = zap<float3>.bar(4); // Expect: 16
}
