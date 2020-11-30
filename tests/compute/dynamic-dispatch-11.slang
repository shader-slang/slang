// Test using interface typed shader parameters with dynamic dispatch.

//TEST(compute):COMPARE_COMPUTE:-dx11
//TEST(compute):COMPARE_COMPUTE:-vk
//TEST(compute):COMPARE_COMPUTE:-cpu -xslang -disable-specialization
//TEST(compute):COMPARE_COMPUTE:-cuda -xslang -disable-specialization

[anyValueSize(8)]
interface IInterface
{
    int run(int input);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutputBuffer
RWStructuredBuffer<int> gOutputBuffer;

//TEST_INPUT:cbuffer(data=[rtti(MyImpl) witness(MyImpl, IInterface) 1 0], stride=4):name=gCb
ConstantBuffer<IInterface> gCb;

[numthreads(4, 1, 1)]
void computeMain(uint3       dispatchThreadID : SV_DispatchThreadID)
{
    let tid = dispatchThreadID.x;

    let inputVal : int = tid;
    let outputVal = gCb.run(inputVal);

    gOutputBuffer[tid] = outputVal;
}

// No type input for dynamic dispatch //TEST_INPUT: globalExistentialType MyImpl
// Type must be marked `public` to ensure it is visible in the generated DLL.
public struct MyImpl : IInterface
{
    int val;
    int run(int input)
    {
        return input + val;
    }
};
