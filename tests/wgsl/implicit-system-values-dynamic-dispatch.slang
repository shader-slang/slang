// Test calling differentiable function through dynamic dispatch.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -compute -entry computeMain -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT: type_conformance Impl1:IInterface = 0
//TEST_INPUT: type_conformance Impl2:IInterface = 1
//TEST_INPUT: type_conformance Impl3:IInterface = 2

[anyValueSize(16)]
interface IInterface
{
    uint getLaneIndex(uint base);
}

struct Impl1 : IInterface
{
    uint getLaneIndex(uint base)
    {
        return base; 
    }
}

struct Impl2 : IInterface
{
    uint getLaneIndex(uint base)
    {
        return base * WaveGetLaneIndex() * 2; 
    }
}

struct Impl3 : IInterface
{
    uint getLaneIndex(uint base)
    {
        return base + WaveGetLaneIndex();
    }
};

[numthreads(2, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const uint base = 5;

    if (dispatchThreadID.x == 0)
    {
        var obj = createDynamicObject<IInterface>(dispatchThreadID.x, 0); // Impl0
        outputBuffer[0] = obj.getLaneIndex(base);

        obj = createDynamicObject<IInterface>(dispatchThreadID.x + 1, 0); // Impl1
        outputBuffer[1] = obj.getLaneIndex(base);
    }
    else
    {
        var obj = createDynamicObject<IInterface>(dispatchThreadID.x, 0); // Impl1
        outputBuffer[2] = obj.getLaneIndex(base);

        obj = createDynamicObject<IInterface>(dispatchThreadID.x + 1, 0); // Impl2
        outputBuffer[3] = obj.getLaneIndex(base);
    }
    
    // BUF: 5
    // BUF-NEXT: 0
    // BUF-NEXT: 10
    // BUF-NEXT: 6
}
