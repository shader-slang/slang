//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[1 2 27 15], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

struct Thing
{
    Ptr<int> somePointer;
};

ConstantBuffer<Thing> thing;

[numthreads(4, 4, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = (int)dispatchThreadID.x;

    int a = thing.somePointer[idx];

    outputBuffer[idx] = idx + a;
}