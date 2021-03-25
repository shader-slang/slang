//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE:-cuda -shaderobj
//TEST(compute):COMPARE_COMPUTE:-vk -shaderobj
//TEST(compute):COMPARE_COMPUTE:-shaderobj


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=block0.buffer
//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):name=block1.buffer

// Ensure that Slang `ParameterBlock` type is lowered
// to HLSL in the fashion that we expect.

struct P
{
	RWStructuredBuffer<int> buffer;
};

ParameterBlock<P> block0;
ParameterBlock<P> block1;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;

	int inVal = block1.buffer[tid];

	int outVal = inVal;

	block0.buffer[tid] = outVal;
}