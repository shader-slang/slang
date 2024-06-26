//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-slang -vk -compute -shaderobj -render-feature double
//TEST(compute):COMPARE_COMPUTE_EX:-slang -cuda -compute -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE:-slang -shaderobj -mtl

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // asdouble
    double d = 1.0;

    uint hi, low;
    asuint(d, low, hi);

    double r = asdouble(low, hi);

    outputBuffer[0] = int(r);

    // asdouble2
    double2 d2 = double2(2.0, 3.0);

    uint2 hi2, low2;
    asuint(d2.x, low2.x, hi2.x);
    asuint(d2.y, low2.y, hi2.y);

    double2 r2 = asdouble(low2, hi2);
    
    outputBuffer[1] = int(r2.x);
    outputBuffer[2] = int(r2.y);
}
