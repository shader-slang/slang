// static-const-array.slang

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST_DISABLED(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
RWStructuredBuffer<int> outputBuffer;

static const int kArray[] = { 16, 1, 32, 2 };

int test(int val)
{
	return kArray[val];
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
	int inVal = tid.x;
	int outVal = test(inVal);
	outputBuffer[inVal] = outVal;
}