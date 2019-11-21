// atomics-groupshared.slang

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out

RWStructuredBuffer<uint> outputBuffer;

groupshared uint shared[4];

uint test(uint val)
{
    uint originalValue;

    shared[val] = 0;

    GroupMemoryBarrierWithGroupSync();

	InterlockedAdd(shared[val], 		val, 		originalValue);
	InterlockedAdd(shared[val ^ 1], 	val*16, 	originalValue);
	InterlockedAdd(shared[val ^ 2], 	val*16*16, 	originalValue);
	
    GroupMemoryBarrierWithGroupSync();

    return shared[val];
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    uint val = test(tid);
    outputBuffer[tid] = val;
}