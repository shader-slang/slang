//TEST(smoke,compute):COMPARE_COMPUTE:
//TEST_INPUT:ubuffer(data=[0], stride=4):dxbinding(0),glbinding(0),out
//TEST_INPUT:type Wrapper<Impl>

RWStructuredBuffer<float> outputBuffer;

interface IBase
{
    float compute();
}

struct Wrapper<T : IBase> : IBase
{
    T obj;
    float compute()
    {
        return obj.compute();        
    }
};

struct Impl : IBase
{
    float compute()
    {
        return 1.0;
    }
};

__generic_param TImpl : IBase;

TImpl impl;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	float outVal = impl.compute();
	outputBuffer[tid] = outVal;
}