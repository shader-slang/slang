// atomics-groupshared.slang

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out

RWStructuredBuffer<uint> outputBuffer;

groupshared uint shared[4];

uint test(uint val)
{
    uint originalValue;

    outputBuffer[val] = 0;

    GroupMemoryBarrierWithGroupSync();

	InterlockedAdd(outputBuffer[val], 		val, 		originalValue);
	InterlockedAdd(outputBuffer[val ^ 1], 	val*16, 	originalValue);
	InterlockedAdd(outputBuffer[val ^ 2], 	val*16*16, 	originalValue);

    GroupMemoryBarrierWithGroupSync();

    return outputBuffer[val];
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    uint val = test(tid);
    outputBuffer[tid] = val;
}