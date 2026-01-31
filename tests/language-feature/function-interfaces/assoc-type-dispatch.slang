//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Smoke test for function interfaces

interface IFoo
{
    static float bar(float a);
}

interface IZig
{
    associatedtype A : IFoo;
    static float doThing(float a);
}

struct AImpl : IFoo
{
    static float bar(float x)
    {
        return x * x;
    }
}

struct ZigImpl : IZig
{
    typealias A = AImpl;
    static float doThing(float a)
    {
        return a * 2;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    IZig izig = ZigImpl();
    outputBuffer[tig] = izig.A.bar(4); // Expect: 16
}
