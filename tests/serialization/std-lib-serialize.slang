//TEST:COMPILE: -save-core-module tests/serialization/slang-core-module.zip
//TEST:COMPILE: -load-core-module tests/serialization/slang-core-module.zip -save-glsl-module-bin-source tests/serialization/slang-glsl-module-generated.actual
//TEST:COMPILE: -save-core-module-bin-source tests/serialization/slang-core-module-generated.actual
//TEST:COMPARE_COMPUTE: -compile-arg -load-core-module -compile-arg tests/serialization/slang-core-module.zip -shaderobj

struct A
{
	float x;

	float addWith(float y)
	{
		return this.x + y;
	}
};

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;


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
