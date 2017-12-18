//TEST(compute):HLSL_COMPUTE:-xslang -no-checking -xslang -use-ir

//TEST_INPUT:cbuffer(data=[1 2 3 4 16 32 48 64]):dxbinding(0),glbinding(0)
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out

import rewriter_use_ir_type;

RWStructuredBuffer<int> outputBuffer : register(u0);

cbuffer C : register(b0)
{
	Helper helper;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	int inVal = tid;

	int outVal = helper.a[inVal];

	outputBuffer[tid] = outVal;
}