//TEST(compute):HLSL_COMPUTE:-xslang -no-checking -xslang -use-ir
//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):dxbinding(0),glbinding(0),out

// Test that we can use Slang libraries that require IR cross-compilation
// (e.g., libraries that use generics) while writing the main code in
// vanilla HLSL/GLSL without checking enabled.

import rewriter;

RWStructuredBuffer<int> outputBuffer : register(u0);

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	int inVal = outputBuffer[tid];
	int outVal = test(inVal);
	outputBuffer[tid] = outVal;
}