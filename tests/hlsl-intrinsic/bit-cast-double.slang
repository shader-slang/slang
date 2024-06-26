//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-slang -vk -compute -shaderobj -render-feature double
//TEST(compute):COMPARE_COMPUTE_EX:-slang -cuda -compute -shaderobj
//DISABLE_TEST(compute):COMPARE_COMPUTE:-slang -shaderobj -mtl

#define THREAD_SIZE 3

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer1
RWStructuredBuffer<int> outputBuffer1;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer2
RWStructuredBuffer<int> outputBuffer2;

[numthreads(THREAD_SIZE, THREAD_SIZE, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int tid = dispatchThreadID.x + THREAD_SIZE * dispatchThreadID.y;

    // asdouble
    double d = double(dispatchThreadID.x);

    uint hi, low;
    asuint(d, low, hi);

    double r = asdouble(low, hi);

    outputBuffer1[tid] = int(r);

    // asdouble2
    double2 d2 = double2(dispatchThreadID.x, dispatchThreadID.y);

    uint2 hi2, low2;
    asuint(d2.x, low2.x, hi2.x);
    asuint(d2.y, low2.y, hi2.y);

    double2 r2 = asdouble2(low2, hi2);
    
    outputBuffer2[tid] = int(r2.x + r2.y);
}
