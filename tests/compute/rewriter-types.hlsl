//TEST(compute):HLSL_COMPUTE:-xslang -no-checking -xslang -use-ir

//TEST_INPUT:cbuffer(data=[16]):dxbinding(0),glbinding(0)
//TEST_INPUT:cbuffer(data=[256]):dxbinding(1),glbinding(1)
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):dxbinding(1),glbinding(1)

// Test the case of user code with that uses the "rewriter" mode
// (`-no-checking` flag) and that uses a type declared in
// imported code (that will compile via IR). Also test
// the case where such a type requires legalization.

import rewriter_types;

RWStructuredBuffer<int> outputBuffer : register(u0);

cbuffer C
{
	MyHelper myHelper;
}

cbuffer D
{
	Other other;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	int inVal = tid;
	int outVal = test(inVal, myHelper, other);
	outputBuffer[tid] = outVal;
}