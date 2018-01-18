//TEST(smoke,compute):COMPARE_COMPUTE:-xslang -use-ir
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
// Confirm that generics syntax can be used in user
// code and generates valid output.

RWStructuredBuffer<float> outputBuffer;


__generic<T>
T test(T val)
{
	return val;
}

__generic<T>
T test(T val, int a)
{
	return val;
}


[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;

	float inVal = float(tid);

	float outVal = test<float>(inVal, 0);

	outputBuffer[tid] = outVal;
}