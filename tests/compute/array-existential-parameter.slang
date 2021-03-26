// Test using existential shader parameter that is an interface array.

//DISABLED_TEST(compute):COMPARE_COMPUTE:-cpu
//DISABLE_TEST(compute):COMPARE_COMPUTE:-cuda

[anyValueSize(8)]
interface IInterface
{
    int run(int input);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutputBuffer
RWStructuredBuffer<int> gOutputBuffer;

struct Params
{
    IInterface values[2];
};

//TEST_INPUT:cbuffer(data=[rtti(MyImpl) witness(MyImpl, IInterface) 1 0], stride=4):name=gCb
ConstantBuffer<Params> gCb;

//TEST_INPUT:cbuffer(data=[rtti(MyImpl) witness(MyImpl, IInterface) 1 0], stride=4):name=gCb2
ConstantBuffer<Params> gCb2;

[numthreads(4, 1, 1)]
void computeMain(uint3       dispatchThreadID : SV_DispatchThreadID)
{
    let tid = dispatchThreadID.x;

    let inputVal : int = tid;
    let outputVal = gCb.values[0].run(inputVal) + gCb2.values[0].run(inputVal);

    gOutputBuffer[tid] = outputVal;
}

//TEST_INPUT: globalExistentialType MyImpl
//TEST_INPUT: globalExistentialType __Dynamic

// Type must be marked `public` to ensure it is visible in the generated DLL.
public struct MyImpl : IInterface
{
    int val;
    int run(int input)
    {
        return input + val;
    }
};
