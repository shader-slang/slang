//TEST(smoke,compute):COMPARE_COMPUTE:-xslang -use-ir
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
// Confirm that generics syntax can be used in user
// code and generates valid output.

RWStructuredBuffer<float> outputBuffer;

interface ISimple
{
    assoctype T;
    T add(T v0, T v1);
}

struct Simple : ISimple
{
    typedef float T;
    T add(T v0, float v1)
    {
        return v0 + v1;
    }
};

__generic<T:ISimple>
T.T test(T simple, T.T v0, T.T v1)
{
    return simple.add(v0, v1);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	float outVal = test<Simple>(Simple(), 2.0, 1.0); // == 3.0
	outputBuffer[tid] = outVal;
}