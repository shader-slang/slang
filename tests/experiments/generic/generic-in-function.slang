//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj

RWStructuredBuffer<float> outputBuffer;

int doThing()
{    
	int getValue<let N : int>() { return N; }
    
    return getValue<10>();
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;

	float inVal = float(tid);

	outputBuffer[tid] = doThing();
}
