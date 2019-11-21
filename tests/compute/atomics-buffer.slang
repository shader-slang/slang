// atomics-buffer.slang

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute

// Note: not enabling D3D12 test yet because change
// was developed on a machine that can run D3D12
//
//TEST_DISABLED(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12

//TEST_INPUT:ubuffer(format=R_UInt32, data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]):out

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