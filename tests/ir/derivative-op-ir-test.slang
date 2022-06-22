//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

__differentiate_jvp float f(float x)
{
    return x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
     {
        float a = 1.0;
        float b = -2.0;
        outputBuffer[0] = __jvp(f)(a);
        outputBuffer[1] = __jvp(f)(b);
    }
}
