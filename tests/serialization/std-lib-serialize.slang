//TEST:COMPILE: -save-stdlib slang-std-lib.zip
//TEST:COMPARE_COMPUTE: -compile-arg -load-stdlib -compile-arg slang-std-lib.zip -shaderobj

struct A
{
	float x;

	float addWith(float y)
	{
		return this.x + y;
	}
};

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer : register(u0);


float test(float inVal)
{
	A a;
	a.x = inVal;
	return a.addWith(inVal*inVal);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	float inVal = float(tid);
	float outVal = test(inVal);
	outputBuffer[tid] = outVal;
}