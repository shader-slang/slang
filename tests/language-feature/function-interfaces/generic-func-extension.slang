//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Smoke test for function interfaces on specialized generic functions

interface IFoo<T>
{
    static T bar(T a);
}

T zap<T : IFloat>(T x)
{
    return (T)2 * x;
}

extension<T : IFloat> __func_as_type(zap<T>) : IFoo<T>
{
    static T bar(T a)
    {
        return a * a;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    outputBuffer[tig] = zap<float3>.bar(float3(4, 4, 4)).y; // Expect: 16
}
