// interface-param.slang

// Test basic use of an interface as an existential type
// for a value parameter, instead of just as a constraint
// on a generic type parameter.

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute

interface IHelper
{
	int getVal();
}

int doTheThing(IHelper helper)
{
	return helper.getVal();
}

struct HelperImpl : IHelper
{
	int storedVal;

	int getVal() { return storedVal; }
}

int test(int val)
{
    HelperImpl helperImpl = { val };
    return doTheThing(helperImpl);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
RWStructuredBuffer<int> gOutputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	int inputVal = tid;
	int outputVal = test(inputVal);
	gOutputBuffer[tid] = outputVal;
}