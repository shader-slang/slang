//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int a, b;
    if (dispatchThreadID.x > 1)
    {
        a = 1;
        b = 2;
    }
    else
    {
        a = 2;
        b = 3;
    }
    outputBuffer[0] = a + b;
}
