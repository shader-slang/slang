//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute,vulkan):COMPARE_COMPUTE_EX:-vk -slang -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Define an attribute that will appear in reflection
[__AttributeUsage(_AttributeTargets.Var)]
struct StaticSamplerAttribute {};

//#define StaticSamplerState [StaticSampler] SamplerState

struct Thing
{
    [StaticSampler] SamplerState samplerState1;
    [StaticSampler] SamplerState samplerState2;
}

[StaticSampler] SamplerState samplerState;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;

	float inVal = float(tid);

	outputBuffer[tid] = int(inVal);
}


