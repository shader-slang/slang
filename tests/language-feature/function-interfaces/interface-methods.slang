//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Smoke test for function interfaces as constraints on interface methods.

interface IFoo
{
    static float bar(float a);
}

interface IZig
{
    static float doThing(float a) : IFoo;
}

struct ZigImpl : IZig
{
    static float doThing(float a)
    {
        return a * 2;
    }
}

extension __func_as_type(ZigImpl::doThing) : IFoo
{
    static float bar(float x)
    {
        return x * x;
    }
}

float zap(float x)
{
    return 2 * x;
}

extension __func_as_type(zap) : IFoo
{
    static float bar(float x)
    {
        return x * x;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    IZig izig = ZigImpl();
    outputBuffer[tig] = izig.doThing.bar(4); // Expect: 16
}
