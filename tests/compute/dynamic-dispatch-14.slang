// Test using interface typed shader parameters with associated types.

//TEST(compute):COMPARE_COMPUTE:-dx11
//TEST(compute):COMPARE_COMPUTE:-cpu
//TEST(compute):COMPARE_COMPUTE:-vk
//TEST(compute):COMPARE_COMPUTE:-cuda

[anyValueSize(8)]
interface IAssoc
{
    int eval();
}

[anyValueSize(8)]
interface IInterface
{
    associatedtype TAssoc : IAssoc;
    TAssoc run(int input);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutputBuffer
RWStructuredBuffer<int> gOutputBuffer;

//TEST_INPUT:ubuffer(data=[rtti(MyImpl) witness(MyImpl, IInterface) 1 0], stride=4):name=gCb
RWStructuredBuffer<IInterface> gCb;

//TEST_INPUT:ubuffer(data=[rtti(MyImpl) witness(MyImpl, IInterface) 1 0], stride=4):name=gCb1
RWStructuredBuffer<IInterface> gCb1;

[numthreads(4, 1, 1)]
void computeMain(uint3       dispatchThreadID : SV_DispatchThreadID)
{
    let tid = dispatchThreadID.x;

    let inputVal : int = tid;
    IInterface v0 = gCb.Load(0);
    IInterface v1 = gCb1[0];
    let outputVal = v0.run(inputVal).eval() + v1.run(inputVal).eval();

    gOutputBuffer[tid] = outputVal;
}

// Specialize gCb1, but not gCb2
//TEST_INPUT: globalExistentialType MyImpl
//TEST_INPUT: globalExistentialType __Dynamic
// Type must be marked `public` to ensure it is visible in the generated DLL.
public struct MyImpl : IInterface
{
    int val;
    public struct TAssoc : IAssoc
    {
        int val;
        int eval() { return val; }
    }
    TAssoc run(int input)
    {
        TAssoc rs;
        rs.val = input + val;
        return rs;
    }
};

public struct MyImpl2 : IInterface
{
    int val;
    public struct TAssoc : IAssoc
    {
        int val;
        int eval() { return val; }
    }
    TAssoc run(int input)
    {
        TAssoc rs;
        rs.val = input - val;
        return rs;
    }
};