// atomics-buffer.slang

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12

//TEST_INPUT:ubuffer(format=R_UInt32, data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]):out,name outputBuffer

RWBuffer<uint> outputBuffer;

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