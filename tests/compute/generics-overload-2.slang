//TEST(smoke,compute):COMPARE_COMPUTE: -shaderobj
//TEST(smoke,compute):COMPARE_COMPUTE:-cpu -shaderobj

// Confirm that generics syntax can be used in user
// code and generates valid output.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;


__generic<T : __BuiltinFloatingPointType>
struct Foo
{
	T test(uint index, T x)
	{
		return __realCast<T, float>(index * 1.5f);
	}

	__generic<let N: int>
	T test(vector<uint, N> index, T x)
	{
		T val = __realCast<T, float>(0.f);
		for (uint i = 0; i < N; i++)
			val = val * __realCast<T, float>(float(index[i]));
		return val;
	}
};


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x + 2;
	
	Foo<float> obj;

	float outVal = obj.test(tid, 0.f);
	outputBuffer[tid] = outVal;

	float outVal2 = obj.test(uint2(tid, tid), 0.f);
	outputBuffer[tid + 1] = outVal2;
}