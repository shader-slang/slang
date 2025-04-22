//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Smoke test for function interfaces

interface IFoo
{
    static __associatedfunc functype (float) -> float foo;
}

float zap(float x)
{
    return 2 * x;
}

extension __func_as_type(zap) : IFoo
{
    static float foo(float x)
    {
        return x * x;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    outputBuffer[tig] = zap.foo(0);
}
