// comma-operator.slang

//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj -xslang -Wno-41024
//TEST(compute):COMPARE_COMPUTE: -shaderobj -xslang -Wno-41024

// Test that the "comma operator" behaves as expected

// We will also include a cross-compilation test just to
// confirm that the generated SPIR-V (and by extension DXIL/DXBC)
// doesn't include a subroutine defintion/call for `operator,`

//TEST:CROSS_COMPILE:-target spirv-assembly -entry computeMain -stage compute -Wno-41024

int test(int inVal)
{
	int a = inVal;

	// We expect the left operand to `operator,` to be
	// evaluated before the right operand, and for the
	// result to be the value of the right operand
	//
	return a*=2, a+1;
}

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	int inVal = outputBuffer[tid];
	int outVal = test(inVal);
	outputBuffer[tid] = outVal;
}