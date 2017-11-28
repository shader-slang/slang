//TEST(compute):HLSL_COMPUTE:-xslang -no-checking -xslang -use-ir

//TEST_INPUT:cbuffer(data=[16 0 0 0 32 0 0 0]):dxbinding(0),glbinding(0)
//TEST_INPUT:cbuffer(data=[256 0 0 0 512 0 0 0 768 0 0 0 1024 0 0 0]):dxbinding(1),glbinding(1)
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
//TEST_INPUT:ubuffer(data=[90 91 92 93], stride=4):dxbinding(1),glbinding(1)
//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):dxbinding(2),glbinding(2)

// Test the case of user code with that uses the "rewriter" mode
// (`-no-checking` flag) and that uses a type declared in
// imported code (that will compile via IR). Also test
// the case where such a type requires legalization.

import rewriter_types;

RWStructuredBuffer<int> outputBuffer : register(u0);

cbuffer C
{
	MyHelper myHelpers[2];
}

cbuffer D
{
	Other others[2];
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	int inVal = tid;
	int outVal = test(inVal, myHelpers[1], others[tid]);
	outputBuffer[tid] = outVal;
}