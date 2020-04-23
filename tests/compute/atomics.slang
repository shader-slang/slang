// atomics.slang

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -vk
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -cuda

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out, name outputBuffer

RWStructuredBuffer<uint> outputBuffer;

void test(uint val)
{
    uint originalValue;

	InterlockedAdd(outputBuffer[val], 		val, 		originalValue);
	InterlockedAdd(outputBuffer[val ^ 1], 	val*16, 	originalValue);
	InterlockedAdd(outputBuffer[val ^ 2], 	val*16*16, 	originalValue);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    test(tid);
}