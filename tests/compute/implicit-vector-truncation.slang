//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute

int sum(int3 v)
{
	return v.x + v.y + v.z;
}

int test(int val)
{
	int4 a = int4(val, val*0x10, val*0x100, val*0x1000);
	return sum(a);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
RWStructuredBuffer<int> gBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	int val = int(tid);
	val = test(val);
	gBuffer[tid] = val;
}