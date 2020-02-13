//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -compile-arg -O3
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float v = asfloat((int(dispatchThreadID.x) & -2147483648) | 1065353216);
	outputBuffer[dispatchThreadID.x] = v;
}
