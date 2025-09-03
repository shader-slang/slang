//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Smoke test for function interfaces

interface IFoo
{
    static float bar(float a);
}

struct Zig
{
    float zap(float x)
    {
        return 2 * x;
    }
};

extension __func_as_type(Zig::zap) : IFoo
{
    static float bar(float x)
    {
        return x * x;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    Zig zig;
    outputBuffer[tig] = zig.zap.bar(4); // Expect: 16
}
