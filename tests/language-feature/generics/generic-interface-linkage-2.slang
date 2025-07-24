//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

float getVal<let doStuff : bool>()
{
    if (doStuff)
    {
        return 100.0;
    }
    else
    {
        return 200.0;
    }
}

[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK: 100.0
    outputBuffer[0] = getVal<true>();
    // CHECK: 200.0
    outputBuffer[1] = getVal<false>();
}